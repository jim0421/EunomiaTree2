// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "db/hashtable_template.h"
#include <set>
#include "leveldb/env.h"
#include "leveldb/comparator.h"
#include "util/arena.h"
#include "util/hash.h"
#include "util/random.h"
#include "util/testharness.h"

namespace leveldb {

typedef uint64_t Key;


class KeyComparator : public leveldb::Comparator {
    public:
	int operator()(const Key& a, const Key& b) const {
		if (a < b) {
	      return -1;
	    } else if (a > b) {
	      return +1;
	    } else {
	      return 0;
	    }
	}

	virtual int Compare(const Slice& a, const Slice& b) const {
		assert(0);
		return 0;
	}

	virtual const char* Name()  const {
		assert(0);
		return 0;
	};

   virtual void FindShortestSeparator(
      std::string* start,
      const Slice& limit)  const {
		assert(0);

	}
  
   virtual void FindShortSuccessor(std::string* key)  const {
		assert(0);

	}
  
  };

class KeyHash : public leveldb::HashFunction
{
public:
	
	virtual uint64_t hash(const Key& k)
	{
		return k;
	}

};

}

int main(int argc, char** argv) {
	
  leveldb::KeyHash kh;
  leveldb::KeyComparator cmp;
  leveldb::HashTable<leveldb::Key, leveldb::KeyHash, leveldb::KeyComparator> ht(kh, cmp);

  
  return 1;
}
