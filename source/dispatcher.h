#ifndef DISPATCHER
#define DISPATCHER

#include <3ds.h>
#include "producer.h"

#define NUM_PRODUCERS (1)

typedef struct
{
	producer_s producers[NUM_PRODUCERS];
	jobQueue_s requestList;
}dispatcher_s;

void initDispatcher(dispatcher_s* d);
void exitDispatcher(dispatcher_s* d);

#endif
