#ifndef PRODUCER_H
#define PRODUCER_H

#include <3ds.h>
#include "job.h"

#define PRODUCER_STACKSIZE (0x1000)
#define PRODUCER_TMPBUFSIZE (4096*20)

typedef struct producer_s
{
	Handle thread;
	Handle requestMutex, responseMutex;
	bool exit;
	jobQueue_s privateList; //only accessible from producer
	jobQueue_s requestList; //accessible by anyone, given they've locked requestMutex
	jobQueue_s responseList; //accessible by anyone, given they've locked responseMutex
	u64 stack[PRODUCER_STACKSIZE/8];
	u64 tmpBuffer[PRODUCER_TMPBUFSIZE/8];
}producer_s;

void initProducer(producer_s* p);
void exitProducer(producer_s* p);

#endif
