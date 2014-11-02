#ifndef PRODUCER_H
#define PRODUCER_H

#include <3ds.h>

#define PRODUCER_STACKSIZE (0x1000)

typedef struct job_s
{
	struct job_s* next;
}job_s;

typedef struct
{
	Handle thread;
	Handle requestMutex, responseMutex;
	bool exit;
	job_s* privateList;
	job_s* requestList;
	job_s* responseList;
	u64 stack[PRODUCER_STACKSIZE/8];
}producer_s;

void initProducer(producer_s* p);
void exitProducer(producer_s* p);

#endif
