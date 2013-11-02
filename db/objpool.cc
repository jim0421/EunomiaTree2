#include "port/atomic.h"
#include "objpool.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


OBJPool::OBJPool()
{
	gclists_ = NULL;
	curlist_ = NULL;

	freenum_ = 0;	
	freelist_ = NULL;

	debug = false;
}
	
OBJPool::~OBJPool()
{
	while(gclists_ != NULL) {
		Header* cur = gclists_;
		gclists_ = gclists_->next;
		FreeList(cur);
		delete cur;
	}
}
	
void OBJPool::AddGCObj(uint64_t* gobj, uint64_t sn)
{

	Obj* o = new Obj();
	o->value = gobj;
	
	if(curlist_ == NULL) {

		assert(gclists_ == NULL);
		gclists_ = curlist_ = new Header();
		curlist_->sn = sn;
		
	} else if(curlist_->sn != sn) {
	
		assert(curlist_->sn < sn);

		curlist_->next = new Header();
		curlist_ = curlist_->next;
		curlist_->sn = sn;
	}

	
	if(NULL == curlist_->tail) {
		curlist_->tail = o;
		assert(NULL == curlist_->head);
	}
		
	o->next = curlist_->head;
	curlist_->head = o;
		
	curlist_->gcnum++;

}

uint64_t* OBJPool::GetFreeObj()
{
	if(0 == freenum_)
		return NULL;

	assert(freenum_ > 0);

	
	Obj* r = freelist_;

	freelist_ = freelist_->next;
	freenum_--;

	uint64_t *val = r->value;
	delete r;
	
	return val;
}

void OBJPool::GC(uint64_t safesn)
{
	if(gclists_ == NULL) {
		assert(curlist_ == NULL);
		return;
	}

	while(gclists_ != NULL && gclists_->sn <= safesn) {
		Header* cur = gclists_;
		gclists_ = gclists_->next;
		if(cur == curlist_) {
			curlist_ = NULL;
			assert(gclists_ == NULL);
		}
		FreeList(cur);
		delete cur;
	}
		
}

void OBJPool::GCList(Header* list)
{
	list->tail = freelist_;
	freelist_ = list->head;
	freenum_ += list->gcnum;
}

void OBJPool::FreeList(Header* list)
{
	assert(list != NULL);
	
	while (NULL != list->head) {
		Obj* o = list->head;
		list->head = list->head->next;
		delete o->value;
		delete o;
	}
}


void OBJPool::Print()
{
	
	printf("==================GC List=======================\n");
	
	int i = 0;
	Header* cur = gclists_;
	while(cur  != NULL) {
		cur = cur->next;
		i++;
		printf("Header [%d] cur %lx elems %d sn %ld\n", 
			i, cur, cur->gcnum, cur->sn);
	}
/*
	printf("==================Free List=======================\n");
	cur = freelist_;
	while(cur != NULL) {
		printf("Cur %lx Next %lx\n", cur, cur->next);
		cur = cur->next;
	}*/
	
}

