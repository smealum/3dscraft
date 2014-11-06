#include <3ds.h>

#include "text.h"
#include "producer.h"

void producerMain(u32 arg)
{
	producer_s* p=(producer_s*)arg;
	while(!p->exit)
	{
		svcWaitSynchronization(p->requestMutex, U64_MAX);
		appendJobQueue(&p->privateList, &p->requestList);
		svcReleaseMutex(p->requestMutex);

		// bool debug=false;
		// u64 val=svcGetSystemTick();
		
		job_s* j=NULL;
		while((j=unqueueJob(&p->privateList)) && !p->exit)
		{
			handleJob(p, j);

			svcWaitSynchronization(p->responseMutex, U64_MAX);
			queueJob(&p->responseList, j);
			svcReleaseMutex(p->responseMutex);
			// debug=true;
			svcSleepThread(1000);
		}

		// if(debug)print("%d ticks\n",(int)(svcGetSystemTick()-val));

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
	Result val = svcCreateThread(&p->thread, producerMain, (u32)p, (u32*)&p->stack[PRODUCER_STACKSIZE/8], 0x18, 1);
	print("%08X (%08X)\n",(unsigned int)val,(unsigned int)p->thread);
	if(val)
	{
		//thread creation failed ! what do we do ?!
	}
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
