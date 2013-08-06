#ifndef MEMSTORE_CUCKOOHASH_H_
#define MEMSTORE_CUCKOOHASH_H_

#include "util/hash.h"
#include "memstore.h"
#include "util/txprofile.h"
#include "util/spinlock.h"


class MemstoreCuckooHashTable: public Memstore {

#define ASSOCIATIVITY 4
#define MAX_CUCKOO_COUNT 128
#define DEFAULT_SIZE 100*1024*1024 // 100 Rows

 private:

  struct Element
  {
    Element()
	{
		key = -1;
		hash1 = 0;
		hash2 = 0;
		value = NULL;
	}
	uint64_t key;
	uint64_t hash1;
	uint64_t hash2;
	MemNode* value;
  };
  
  struct Entry
  {
  	Element elems[ASSOCIATIVITY];
  };

  char padding[64];
  Entry* table_;
  uint64_t size_;
  char padding2[64];
  RTMProfile prof;
  char padding3[64];
  SpinLock glock;
  char padding4[64];
  SpinLock rtmlock;
  char padding5[64];

  static __thread bool localinit_;
  static __thread MemNode *dummyval_;
  static __thread uint32_t *undo_index;
  static __thread uint32_t *undo_slot;

 public:

  MemstoreCuckooHashTable();
  
  ~MemstoreCuckooHashTable();

  //Only for initialization

  Memstore::Iterator* GetIterator();

  void Put(uint64_t k, uint64_t* val);

  MemNode* Get(uint64_t key);
  
  MemNode* GetWithInsert(uint64_t key);
  
  void PrintStore();
  
  void ThreadLocalInit();

private:

  bool Insert(uint64_t k, uint64_t* val);

  inline int GetFreeSlot(Entry& e)
  {
	  int i = 0;
	  
	  for(i = 0; i < ASSOCIATIVITY; i++) {
		 if(e.elems[i].key == -1 && e.elems[i].hash1 == 0)
			return i;
      }

	  return i;
  }

  inline void WriteAtSlot(Entry& e, int slot, uint64_t key, 
  									uint64_t h1, uint64_t h2, MemNode* val)
  {
	  e.elems[slot].key = key;
	  e.elems[slot].hash1 = h1;
	  e.elems[slot].hash2 = h2;
	  e.elems[slot].value = val;
  }

  inline int GetSlot(Entry& e, uint64_t key)
  {
	  int i = 0;
	  
	  for(i = 0; i < ASSOCIATIVITY; i++) {
		 if(e.elems[i].key == key)
			return i;
      }

	  return i;
  }

  inline uint64_t MurmurHash64A (uint64_t key, unsigned int seed )	{

		const uint64_t m = 0xc6a4a7935bd1e995;
		const int r = 47;
		uint64_t h = seed ^ (8 * m);
		const uint64_t * data = &key;
		const uint64_t * end = data + 1;

		while(data != end)	{
			uint64_t k = *data++;
			k *= m; 
			k ^= k >> r; 
			k *= m; 	
			h ^= k;
			h *= m; 
		}

		const unsigned char * data2 = (const unsigned char*)data;

		switch(8 & 7)	{
  		  case 7: h ^= uint64_t(data2[6]) << 48;
		  case 6: h ^= uint64_t(data2[5]) << 40;
		  case 5: h ^= uint64_t(data2[4]) << 32;
		  case 4: h ^= uint64_t(data2[3]) << 24;
		  case 3: h ^= uint64_t(data2[2]) << 16;
		  case 2: h ^= uint64_t(data2[1]) << 8;
		  case 1: h ^= uint64_t(data2[0]);
		  		  h *= m;
		};

		h ^= h >> r;
		h *= m;
		h ^= h >> r;	

		return h;
	}

  inline void ComputeHash(uint64_t key, uint64_t* h1, uint64_t* h2)
  {
//  	*h1 = key;
//	*h2 = key;
	 *h1 = MurmurHash64A(key, 0xdeadbeef);
	 *h2 = MurmurHash64A(key, 0xbeefdead);
  }
  
};


#endif
