#ifndef DISPATCHER
#define DISPATCHER

#include <3ds.h>
#include "producer.h"

#define NUM_PRODUCERS (1)

typedef struct
{
	producer_s producers[NUM_PRODUCERS];
	jobQueue_s requestList;
	int pendingJobs;
}dispatcher_s;

void initDispatcher(dispatcher_s* d);
void updateDispatcher(dispatcher_s* d);
void dispatchJob(dispatcher_s* d, job_s* j);
void exitDispatcher(dispatcher_s* d);

#endif
