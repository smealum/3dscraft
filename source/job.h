#ifndef JOB_H
#define JOB_H

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

void handleJob(job_s* j);

//job type
typedef void (*jobHandler_func)(job_s* j);

typedef struct
{
	jobHandler_func handler;
	u32 dataSize;
}jobType_s;

extern jobType_s jobTypes[NUM_JOB_TYPES];

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

#endif
