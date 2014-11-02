#include <3ds.h>

#include "dispatcher.h"

void initDispatcher(dispatcher_s* d)
{
	if(!d)return;

	initJobPool();

	int i; for(i=0;i<NUM_PRODUCERS;i++)initProducer(&d->producers[i]);

	initJobQueue(&d->requestList);
}

void updateDispatcher(dispatcher_s* d)
{
	if(!d)return;

	//ALL THE FOLLOWING TEMPORARY
	svcWaitSynchronization(d->producers[0].requestMutex, U64_MAX);
	appendJobQueue(&d->producers[0].requestList, &d->requestList);
	svcReleaseMutex(d->producers[0].requestMutex);

	jobQueue_s tmpQueue;
	initJobQueue(&tmpQueue);
	svcWaitSynchronization(d->producers[0].responseMutex, U64_MAX);
	appendJobQueue(&tmpQueue, &d->producers[0].responseList);
	svcReleaseMutex(d->producers[0].responseMutex);

	job_s* j=NULL; while((j=unqueueJob(&tmpQueue)))freeJob(j);
}

void dispatchJob(dispatcher_s* d, job_s* j)
{
	if(!d || !j)return;

	queueJob(&d->requestList, j);
}

void exitDispatcher(dispatcher_s* d)
{
	if(!d)return;

	int i; for(i=0;i<NUM_PRODUCERS;i++)exitProducer(&d->producers[i]);
}
