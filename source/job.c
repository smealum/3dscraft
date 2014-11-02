#include <3ds.h>

#include "world.h"
#include "job.h"

#define JOBPOOL_ALLOCSIZE (128)

job_s* jobPool;

void initJobPool(void)
{
	jobPool=NULL;
}

void allocatePoolJobs(void)
{
	job_s* newJobs=malloc(sizeof(job_s)*JOBPOOL_ALLOCSIZE);
	int i; for(i=0;i<JOBPOOL_ALLOCSIZE-1;i++){newJobs[i].data=NULL;newJobs[i].next=&newJobs[i+1];}
	newJobs[JOBPOOL_ALLOCSIZE-1].data=NULL; newJobs[JOBPOOL_ALLOCSIZE-1].next=jobPool;
	jobPool=newJobs;
}

job_s* getNewJob(void)
{
	if(!jobPool)allocatePoolJobs();
	job_s* j=jobPool;
	if(!j)return j;
	jobPool=j->next;
	j->next=NULL;
	return j;
}

job_s* createNewJob(jobTypes_t t)
{
	if(t<0 || t>=NUM_JOB_TYPES)return NULL;
	job_s* j=getNewJob();
	if(!j)return j;
	j->type=t;
	j->data=malloc(jobTypes[t].dataSize);
	return j;
}

void freeJob(job_s* j)
{
	if(!j)return;
	if(j->data){free(j->data);j->data=NULL;}
	j->next=jobPool;
	jobPool=j;
}

//JOB_GENERATE_CLUSTER
typedef struct
{
	worldCluster_s* target;
}jobGenerateClusterData_s;

job_s* createJobGenerateCluster(worldCluster_s* wcl)
{
	job_s* j=createNewJob(JOB_GENERATE_CLUSTER);
	if(!j)return j;
	jobGenerateClusterData_s* d=(jobGenerateClusterData_s*)j->data;

	d->target=wcl;
	d->target->busy=true;

	return j;
}

void jobGenerateClusterHandler(job_s* j)
{
	if(!j)return;
	jobGenerateClusterData_s* d=(jobGenerateClusterData_s*)j->data;

	generateWorldClusterData(d->target);
}

void jobGenerateClusterFinalizer(job_s* j)
{
	if(!j)return;
	jobGenerateClusterData_s* d=(jobGenerateClusterData_s*)j->data;

	d->target->busy=false;
}

jobType_s jobTypes[NUM_JOB_TYPES]= {
	(jobType_s){jobGenerateClusterHandler, jobGenerateClusterFinalizer, sizeof(jobGenerateClusterData_s)}, // JOB_GENERATE_CLUSTER
};

//job
void handleJob(job_s* j)
{
	if(!j || j->type>=NUM_JOB_TYPES)return;

	jobTypes[j->type].handler(j);
}
void finalizeJob(job_s* j)
{
	if(!j || j->type>=NUM_JOB_TYPES)return;

	jobTypes[j->type].finalizer(j);
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

void appendJobQueue(jobQueue_s* jq1, jobQueue_s* jq2)
{
	if(!jq1 || !jq2 || !jq2->length)return;

	if(!jq1->length)*jq1=*jq2;
	else{
		jq1->last->next=jq2->first;
		jq1->last=jq2->last;
		jq1->length+=jq2->length;
	}
	jq2->length=0;jq2->first=jq2->last=NULL;
}

job_s* unqueueJob(jobQueue_s* jq)
{
	if(!jq || !jq->length)return NULL;

	job_s* j=jq->first;
	if(jq->length==1)jq->first=jq->last=NULL;
	else jq->first=j->next;
	j->next=NULL;
	jq->length--;

	return j;
}
