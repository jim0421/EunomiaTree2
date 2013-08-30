#include "port/atomic.h"
#include "gcqueue.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

GCQueue::GCQueue()
{
	//the default size of the gc queue is 64
	qsize = 1204;
	head = tail = 0;
	queue = new GCElement*[qsize];
	need_del = 0;
	actual_del = 0;
	
}
	
GCQueue::~GCQueue()
{
	while(head != tail) {
		delete queue[head];
		head = (head + 1) % qsize;
	}

	delete[] queue;

}

void GCQueue::AddGCElement(Epoch* e, uint64_t** arr, int len)
{
	//the queue is empty
	queue[tail] = new GCElement(e, arr, len);
	tail = (tail + 1) % qsize;
	if(tail == head) {
		printf("ERROR: GCQUEUE Over Flow \n");
		exit(1);
	}
	
	assert(tail != head);
	
#if GCTEST
	need_del++;
#endif	
}

void GCQueue::GC(Epoch* current)
{
	while(head != tail && queue[head]->epoch->Compare(current) < 0) {
		delete queue[head];
		head = (head + 1) % qsize;

#if GCTEST
		actual_del++;
#endif
	}
}
