#ifndef JOB_H
#define JOB_H

#include "world.h"

typedef enum
{
	JOB_GENERATE_CLUSTER,
	NUM_JOB_TYPES
}jobTypes_t;

//job
typedef struct job_s
{
	jobTypes_t type;
	void* data;
	struct job_s* next;
}job_s;

void initJobPool(void);
void allocatePoolJobs(void);
job_s* createNewJob(jobTypes_t t);
job_s* getNewJob(void);
void freeJob(job_s* j);

void handleJob(job_s* j);

//job type
typedef void (*jobHandler_func)(job_s* j);

typedef struct
{
	jobHandler_func handler;
	u32 dataSize;
}jobType_s;

extern jobType_s jobTypes[NUM_JOB_TYPES];

job_s* createJobGenerateCluster(worldCluster_s* wcl); //JOB_GENERATE_CLUSTER

//job queue (FIFO)
typedef struct
{
	job_s* first;
	job_s* last;
	int length;
}jobQueue_s;

void initJobQueue(jobQueue_s* jq);
void queueJob(jobQueue_s* jq, job_s* j);
job_s* unqueueJob(jobQueue_s* jq);
void appendJobQueue(jobQueue_s* jq1, jobQueue_s* jq2);

#endif
