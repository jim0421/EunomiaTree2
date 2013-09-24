// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_UTIL_RTM_H_
#define STORAGE_LEVELDB_UTIL_RTM_H_
#include <immintrin.h>
#include <sys/time.h>
#include "util/spinlock.h"
#include "txprofile.h"

#define MAXNEST 0
#define MAXZERO 3
#define MAXCAPACITY 16
#define MAXCONFLICT 128
#define RTMPROFILE 0

#define MAXWRITE 64
#define MAXREAD 128

class RTMScope {

	
 RTMProfile localprofile;
 RTMProfile* globalprof;
 int retry;
 int conflict;
 int capacity;
 int nested;
 int zero;
 uint64_t befcommit;
 uint64_t aftcommit;
 SpinLock* slock;
 
 public:

 static SpinLock fblock;


 inline RTMScope(RTMProfile* prof, int read = 1, int write = 1, SpinLock* sl = NULL) {

  	globalprof = prof;
	retry = 0;
	conflict = 0;
	capacity = 0;
    zero = 0;
	nested = 0;

	if(sl == NULL) {
		//If the user doesn't provide a lock, we give him a default locking
		slock = &fblock;
	} else {
		slock = sl;
	}
	
	if(read > MAXREAD || write > MAXWRITE) {
		slock->Lock();
		return;
	}
	
	while(true) {
	    unsigned stat;
	 	stat = _xbegin();
		if(stat == _XBEGIN_STARTED) {

		  //Put the global lock into read set
		  if(slock->IsLocked())
		    _xabort(0xff);
		  
		  return;
			
		} else {
		
		  retry++;
		  if(stat == 0)
			zero++;
		  else if((stat & _XABORT_CONFLICT) != 0) 
		  	conflict++;
		  else if((stat & _XABORT_CAPACITY) != 0)
			capacity++;
		  else if((stat & _XABORT_NESTED) != 0)
			nested++;
		  
		  if((stat & _XABORT_EXPLICIT) && _XABORT_CODE(stat) == 0xff) {
			 while(slock->IsLocked())
			 	_mm_pause();
		  }


		  int step = (read + write) / 64;

		  if(step == 0)
		  	step = 1;
		  
		  if(nested > MAXNEST) {
			break;
		  }
		  
		  if(zero > MAXZERO) {
			break;
		  }
		  
		  if(capacity > MAXCAPACITY / step) {
		  	break;
		  }
		  
		  if (conflict > MAXCONFLICT/step) {  	
		  	break;
		  }
		
		}
	}
	//printf("hold lock\n");
	slock->Lock();
	
  }

  void Abort() {
  	
  	  _xabort(0x1);
	
  }
inline  ~RTMScope() {  
  	if(slock->IsLocked())
	  slock->Unlock();
	else
	  _xend ();
	//access the global profile info outside the transaction scope
#if RTMPROFILE
	if(globalprof != NULL) {
		globalprof->succCounts++;
		globalprof->abortCounts += retry;
		globalprof->capacityCounts += capacity;
		globalprof->conflictCounts += conflict;
	}
#endif
//		globalprof->MergeLocalStatus(localprofile);

  }

 private:
  RTMScope(const RTMScope&);
  void operator=(const RTMScope&);
};



#endif  // STORAGE_LEVELDB_UTIL_RTM_H_
