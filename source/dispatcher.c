#include <3ds.h>

#include "dispatcher.h"
#include "text.h"

dispatcher_s dispatcher;

void initDispatcher(dispatcher_s* d)
{
	if(!d)d=&dispatcher;

	initJobPool();

	int i; for(i=0;i<NUM_PRODUCERS;i++)initProducer(&d->producers[i]);

	initJobQueue(&d->requestList);

	d->pendingJobs=0;
}

void updateDispatcher(dispatcher_s* d)
{
	if(!d)d=&dispatcher;

	// if(d->requestList.length)print("%d job requests\n",d->requestList.length);

	//ALL THE FOLLOWING TEMPORARY
	svcWaitSynchronization(d->producers[0].requestMutex, U64_MAX);
	appendJobQueue(&d->producers[0].requestList, &d->requestList);
	svcReleaseMutex(d->producers[0].requestMutex);

	jobQueue_s tmpQueue;
	initJobQueue(&tmpQueue);
	svcWaitSynchronization(d->producers[0].responseMutex, U64_MAX);
	appendJobQueue(&tmpQueue, &d->producers[0].responseList);
	svcReleaseMutex(d->producers[0].responseMutex);

	// if(tmpQueue.length)print("%d job responses\n",tmpQueue.length);

	job_s* j=NULL; while((j=unqueueJob(&tmpQueue))){finalizeJob(j);freeJob(j);d->pendingJobs--;}
	// print("pending jobs %d\n",d->pendingJobs);
}

void dispatchJob(dispatcher_s* d, job_s* j)
{
	if(!j)return;
	if(!d)d=&dispatcher;

	queueJob(&d->requestList, j);

	d->pendingJobs++;
}

void exitDispatcher(dispatcher_s* d)
{
	if(!d)d=&dispatcher;

	while(d->pendingJobs){updateDispatcher(d); gspWaitForEvent(GSPEVENT_VBlank0, true);}

	int i; for(i=0;i<NUM_PRODUCERS;i++)exitProducer(&d->producers[i]);
}
