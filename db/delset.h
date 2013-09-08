#ifndef DELSet_H
#define DELSet_H

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "memstore/memstore.h"

class DELSet {


struct DELElement {
	int tableid;
	uint64_t key;
	Memstore::MemNode* node;
	bool delay;
};



private:
	int qsize;
	DELElement* queue;
	
public:
	int elems;
	int delayNum;
	
	DELSet();
	
	~DELSet();

	void Reset();
	
	void Add(int tableid, uint64_t key, Memstore::MemNode* n, bool delay);

	uint64_t** getGCNodes();
	
	uint64_t** getDelayNodes();

	
};

#endif