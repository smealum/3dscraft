#include <3ds.h>

#include "job.h"

//JOB_GENERATE_CLUSTER
typedef struct
{
	//stuff
}jobGenerateClusterData_s;

void jobGenerateClusterHandler(job_s* j)
{
	if(!j)return;
	jobGenerateClusterData_s* d=(jobGenerateClusterData_s*)j->data;
}

jobType_s jobTypes[NUM_JOB_TYPES]= {
	(jobType_s){jobGenerateClusterHandler, sizeof(jobGenerateClusterData_s)}, // JOB_GENERATE_CLUSTER
};

//job
void handleJob(job_s* j)
{
	if(!j || j->type>=NUM_JOB_TYPES)return;

	jobTypes[j->type].handler(j);
}

//job queue
void initJobQueue(jobQueue_s* jq)
{
	if(!jq)return;

	jq->first=jq->last=NULL;
	jq->length=0;
}

void queueJob(jobQueue_s* jq, job_s* j)
{
	if(!jq || !j)return;

	j->next=NULL;
	if(!jq->length)jq->first=jq->last=j;
	else jq->last=jq->last->next=j;
	jq->length++;
}

job_s* unqueueJob(jobQueue_s* jq)
{
	if(!jq || !jq->length)return NULL;

	job_s* j=jq->first;
	if(jq->length==1)jq->first=jq->last=NULL;
	else jq->first=j->next;
	j->next=NULL;

	return j;
}
