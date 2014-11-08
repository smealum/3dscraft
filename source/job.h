#ifndef JOB_H
#define JOB_H

#include "world.h"

struct producer_s;

typedef enum
{
	JOB_GENERATE_CHUNK_DATA,
	JOB_GENERATE_CLUSTER_GEOM,
	JOB_GENERATE_ADDITIONAL_CLUSTER_GEOM,
	JOB_DISCARD_CHUNK,
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

void handleJob(struct producer_s* p, job_s* j);
void finalizeJob(job_s* j);

//job type
typedef void (*jobHandler_func)(struct producer_s* p, job_s* j);
typedef void (*jobFinalizer_func)(job_s* j);

typedef struct
{
	jobHandler_func handler; //executed by one of the producer threads
	jobFinalizer_func finalizer; //executed by main thread
	u32 dataSize;
}jobType_s;

extern jobType_s jobTypes[NUM_JOB_TYPES];

job_s* createJobGenerateChunkData(worldChunk_s* wch); //JOB_GENERATE_CHUNK_DATA
job_s* createJobGenerateClusterGeometry(worldCluster_s* wcl, worldChunk_s* wch, world_s* w); //JOB_GENERATE_CLUSTER_GEOM
job_s* createJobGenerateAdditionalClusterGeometry(worldCluster_s* wcl, worldChunk_s* wch, world_s* w, u8 direction); //JOB_GENERATE_ADDITIONAL_CLUSTER_GEOM
job_s* createJobDiscardChunk(worldChunk_s* wch); //JOB_DISCARD_CHUNK

void createJobsGenerateAdditionalClusterGeometry(worldChunk_s* wch, world_s* w, u8 direction); //helper

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
