#include "port/atomic.h"
#include "delbuffer.h"
#include "rmqueue.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "util/rtm.h"


DELBuffer::DELBuffer()
{
	//default size 64;
	qsize = 64;
	queue = new DELElement[qsize];
	elems = 0;
	delayNum = 0;
}
	
DELBuffer::~DELBuffer()
{
	delete[] queue;
}

void DELBuffer::Reset()
{
	elems = 0;
	delayNum = 0;
}

inline void DELBuffer::Add(int tableid, uint64_t key, Memstore::MemNode* n, bool delay)
{
	queue[elems].tableid = tableid;
	queue[elems].key = key;
	queue[elems].node = n;
	queue[elems].delay = delay;

	if(delay)
		delayNum++;

	elems++;

	if(elems == qsize) {
		printf("Error Del Buffer Overflow!!!\n");
		exit(1);
	}
}

inline uint64_t** DELBuffer::getGCNodes()
{
	int len = elems - delayNum;
	if (0 == len)
		return NULL;
	
	uint64_t **res = new uint64_t*[len];

	int count = 0;
	
	for(int i = 0; i < elems; i++) {

		if(queue[i].delay)
			continue;
		
		res[count] = (uint64_t *)queue[elems].node;
		count++;
	}

	assert(count == len);
	
	return res;
}
	
inline uint64_t** DELBuffer::getDelayNodes()
{
	if (0 == delayNum)
		return NULL;
	
	uint64_t **res = new uint64_t*[delayNum];

	int count = 0;
	
	for(int i = 0; i < elems; i++) {

		if(queue[i].delay) {
			res[count] = (uint64_t *)new leveldb::RMQueue::RMElement(
				queue[i].tableid, queue[i].key, queue[i].node);
			count++;
		}
	}

	assert(count == delayNum);
	
	return res;
}
