#include <3ds.h>

#include "world.h"
#include "job.h"
#include "producer.h"

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

//JOB_GENERATE_CLUSTER_DATA
typedef struct
{
	worldCluster_s* target;
	worldChunk_s* chunk; //read-only
}jobGenerateClusterData_s;

job_s* createJobGenerateCluster(worldCluster_s* wcl, worldChunk_s* wch)
{
	if(!wcl || !wch)return NULL;
	if(wcl->status&WCL_BUSY)return NULL;
	if(!(wcl->status&WCL_DATA_UNAVAILABLE))return NULL;
	job_s* j=createNewJob(JOB_GENERATE_CLUSTER_DATA);
	if(!j)return j;
	jobGenerateClusterData_s* d=(jobGenerateClusterData_s*)j->data;

	d->target=wcl;
	d->target->status|=WCL_DATA_UNAVAILABLE|WCL_BUSY;
	d->chunk=wch;

	return j;
}

void jobGenerateClusterHandler(struct producer_s* p, job_s* j)
{
	if(!p || !j)return;
	jobGenerateClusterData_s* d=(jobGenerateClusterData_s*)j->data;

	if(d->chunk->next)return; //if chunk is in a list, it means it's being discarded in tmpChunkPool => do nothing

	generateWorldClusterData(d->target);
}

void jobGenerateClusterFinalizer(job_s* j)
{
	if(!j)return;
	jobGenerateClusterData_s* d=(jobGenerateClusterData_s*)j->data;

	d->target->status&=~(WCL_DATA_UNAVAILABLE|WCL_BUSY);
	fixChunk(d->chunk);
}

//JOB_GENERATE_CLUSTER_GEOM
typedef struct
{
	worldCluster_s* target;
	worldChunk_s* chunk; //read-only
	world_s* world; //read-only
}jobGenerateClusterGeometryData_s;

job_s* createJobGenerateClusterGeometry(worldCluster_s* wcl, worldChunk_s* wch, world_s* w)
{
	if(!wcl || !wch || !w)return NULL;
	if(wcl->status&WCL_BUSY)return NULL;
	if(wcl->status&WCL_DATA_UNAVAILABLE)return NULL;
	if(!(wcl->status&WCL_GEOM_UNAVAILABLE))return NULL;
	job_s* j=createNewJob(JOB_GENERATE_CLUSTER_GEOM);
	if(!j)return j;
	jobGenerateClusterGeometryData_s* d=(jobGenerateClusterGeometryData_s*)j->data;

	d->target=wcl;
	d->target->status|=WCL_GEOM_UNAVAILABLE|WCL_BUSY;
	d->chunk=wch;
	d->world=w;

	return j;
}

void jobGenerateClusterGeometryHandler(struct producer_s* p, job_s* j)
{
	if(!p || !j)return;
	jobGenerateClusterGeometryData_s* d=(jobGenerateClusterGeometryData_s*)j->data;

	if(d->chunk->next)return; //if chunk is in a list, it means it's being discarded in tmpChunkPool => do nothing

	generateWorldClusterGeometry(d->target, d->world, (blockFace_s*)p->tmpBuffer, PRODUCER_TMPBUFSIZE);
}

void jobGenerateClusterGeometryFinalizer(job_s* j)
{
	if(!j)return;
	jobGenerateClusterGeometryData_s* d=(jobGenerateClusterGeometryData_s*)j->data;

	d->target->status&=~(WCL_GEOM_UNAVAILABLE|WCL_BUSY);
	fixChunk(d->chunk);
}

jobType_s jobTypes[NUM_JOB_TYPES]= {
	(jobType_s){jobGenerateClusterHandler, jobGenerateClusterFinalizer, sizeof(jobGenerateClusterData_s)}, // JOB_GENERATE_CLUSTER_DATA
	(jobType_s){jobGenerateClusterGeometryHandler, jobGenerateClusterGeometryFinalizer, sizeof(jobGenerateClusterGeometryData_s)}, // JOB_GENERATE_CLUSTER_GEOM
};

//job
void handleJob(producer_s* p, job_s* j)
{
	if(!p || !j || j->type>=NUM_JOB_TYPES)return;

	jobTypes[j->type].handler(p,j);
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
