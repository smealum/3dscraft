#include <3ds.h>

#include "dispatcher.h"

void initDispatcher(dispatcher_s* d)
{
	if(!d)return;

	int i; for(i=0;i<NUM_PRODUCERS;i++)initProducer(&d->producers[i]);

	initJobQueue(&d->requestList);
}

void exitDispatcher(dispatcher_s* d)
{
	if(!d)return;

	int i; for(i=0;i<NUM_PRODUCERS;i++)exitProducer(&d->producers[i]);
}
