#include <3ds.h>

#include "producer.h"

void producerMain(u32 arg)
{
	producer_s* p=(producer_s*)arg;
	while(!p->exit)
	{
		svcSleepThread(1000000);
	}
	svcExitThread();
}

void initProducer(producer_s* p)
{
	if(!p)return;

	initJobQueue(&p->privateList);
	initJobQueue(&p->requestList);
	initJobQueue(&p->responseList);

	p->exit=false;
	svcCreateMutex(&p->requestMutex, false);
	svcCreateMutex(&p->responseMutex, false);
	svcCreateThread(&p->thread, producerMain, (u32)p, (u32*)&p->stack[PRODUCER_STACKSIZE/8], 0x18, 0);
}

void exitProducer(producer_s* p)
{
	if(!p)return;

	p->exit=true;
	svcWaitSynchronization(p->thread, U64_MAX);
	svcCloseHandle(p->requestMutex);
	svcCloseHandle(p->responseMutex);
	svcCloseHandle(p->thread);
}
