#include <3ds.h>

#include "world.h"
#include "job.h"
#include "producer.h"
#include "dispatcher.h"

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

//JOB_GENERATE_CHUNK_DATA
typedef struct
{
	worldChunk_s* target;
	world_s* world;
}jobGenerateChunkData_s;

job_s* createJobGenerateChunkData(worldChunk_s* wch)
{
	if(!wch || !wch->world)return NULL;
	if(isChunkBusy(wch))return NULL;
	// if(!(wcl->status&WCL_DATA_UNAVAILABLE))return NULL;
	job_s* j=createNewJob(JOB_GENERATE_CHUNK_DATA);
	if(!j)return j;
	jobGenerateChunkData_s* d=(jobGenerateChunkData_s*)j->data;

	d->target=wch;
	d->world=wch->world;
	int i; for(i=0; i<CHUNK_HEIGHT; i++)d->target->data[i].status|=WCL_DATA_UNAVAILABLE|WCL_BUSY;

	return j;
}

void jobGenerateChunkDataHandler(struct producer_s* p, job_s* j)
{
	if(!p || !j)return;
	jobGenerateChunkData_s* d=(jobGenerateChunkData_s*)j->data;

	if(d->target->next)return; //if chunk is in a list, it means it's being discarded in tmpChunkPool => do nothing

	if(loadChunk(&d->world->stream, d->target->position.x, d->target->position.z, (u8*)p->tmpBuffer))
	{
		const u32 size=CLUSTER_SIZE*CLUSTER_SIZE*CLUSTER_SIZE;
		int i; for(i=0;i<CHUNK_HEIGHT;i++)memcpy(d->target->data[i].data, &(((u8*)p->tmpBuffer)[i*size]), size);
	}else generateWorldChunkData(d->target);
}

void jobGenerateChunkDataFinalizer(job_s* j)
{
	if(!j)return;
	jobGenerateChunkData_s* d=(jobGenerateChunkData_s*)j->data;

	worldChunk_s* wch=NULL;

	wch=getWorldChunk(d->world, vaddi(d->target->position, vect3Di(-1,0,0)));
	if(wch)createJobsGenerateAdditionalClusterGeometry(wch, d->world, WCL_PX);
	wch=getWorldChunk(d->world, vaddi(d->target->position, vect3Di(+1,0,0)));
	if(wch)createJobsGenerateAdditionalClusterGeometry(wch, d->world, WCL_MX);
	wch=getWorldChunk(d->world, vaddi(d->target->position, vect3Di(0,0,-1)));
	if(wch)createJobsGenerateAdditionalClusterGeometry(wch, d->world, WCL_PZ);
	wch=getWorldChunk(d->world, vaddi(d->target->position, vect3Di(0,0,+1)));
	if(wch)createJobsGenerateAdditionalClusterGeometry(wch, d->world, WCL_MZ);

	int i; for(i=0; i<CHUNK_HEIGHT; i++)d->target->data[i].status&=~(WCL_DATA_UNAVAILABLE|WCL_BUSY);
	fixChunk(d->target);
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

//JOB_GENERATE_ADDITIONAL_CLUSTER_GEOM
typedef struct
{
	worldCluster_s* target;
	worldChunk_s* chunk; //read-only
	world_s* world; //read-only
	gsVbo_s vbo;
	u8 direction;
	int value;
}jobGenerateAdditionalClusterGeometryData_s;

#include "text.h"

job_s* createJobGenerateAdditionalClusterGeometry(worldCluster_s* wcl, worldChunk_s* wch, world_s* w, u8 direction)
{
	if(!wcl || !wch || !w || !direction)return NULL;
	if(wcl->status&WCL_BUSY)return NULL;
	if(wcl->status&WCL_DATA_UNAVAILABLE)return NULL;
	if(wcl->status&WCL_GEOM_UNAVAILABLE)return NULL;
	job_s* j=createNewJob(JOB_GENERATE_ADDITIONAL_CLUSTER_GEOM);
	if(!j)return j;
	jobGenerateAdditionalClusterGeometryData_s* d=(jobGenerateAdditionalClusterGeometryData_s*)j->data;

	d->target=wcl;
	d->target->status|=WCL_BUSY;
	d->chunk=wch;
	d->world=w;
	d->direction=direction;
	d->value=0;
	gsVboInit(&d->vbo);

	return j;
}

void createJobsGenerateAdditionalClusterGeometry(worldChunk_s* wch, world_s* w, u8 direction)
{
	if(!wch || !w || !direction)return;
	int i;

	for(i=0; i<CHUNK_HEIGHT; i++)
	{
		worldCluster_s* wcl=&wch->data[i];
		if(!(wcl->status&WCL_BUSY) && !(wcl->status&WCL_GEOM_UNAVAILABLE) && !(wcl->directions&direction))dispatchJob(NULL, createJobGenerateAdditionalClusterGeometry(wcl, wch, w, direction));
		// if(!(wcl->status&WCL_BUSY) && !(wcl->status&WCL_GEOM_UNAVAILABLE)){print("%08X vs %08X\n",(unsigned int)wcl->directions,(unsigned int)direction);}
	}
}

void jobGenerateAdditionalClusterGeometryHandler(struct producer_s* p, job_s* j)
{
	if(!p || !j)return;
	jobGenerateAdditionalClusterGeometryData_s* d=(jobGenerateAdditionalClusterGeometryData_s*)j->data;

	if(d->chunk->next)return; //if chunk is in a list, it means it's being discarded in tmpChunkPool => do nothing

	d->value=generateWorldAdditionalClusterGeometry(d->target, d->world, d->direction, (blockFace_s*)p->tmpBuffer, PRODUCER_TMPBUFSIZE, &d->vbo);
}

void jobGenerateAdditionalClusterGeometryFinalizer(job_s* j)
{
	if(!j)return;
	jobGenerateAdditionalClusterGeometryData_s* d=(jobGenerateAdditionalClusterGeometryData_s*)j->data;

	if(d->value==1)
	{
		gsVboDestroy(&d->target->vbo);
		d->target->vbo=d->vbo;
	}
	d->target->status&=~(WCL_BUSY);
	fixChunk(d->chunk);
}

//JOB_DISCARD_CHUNK
typedef struct
{
	worldChunk_s* target;
	world_s* world;
}jobDiscardChunk_s;

job_s* createJobDiscardChunk(worldChunk_s* wch)
{
	if(!wch || !wch->next || !wch->world || !wch->modified)return NULL;
	if(isChunkBusy(wch))return NULL;
	job_s* j=createNewJob(JOB_DISCARD_CHUNK);
	if(!j)return j;
	jobDiscardChunk_s* d=(jobDiscardChunk_s*)j->data;

	d->target=wch;
	d->world=wch->world;
	int i; for(i=0; i<CHUNK_HEIGHT; i++)d->target->data[i].status|=WCL_DATA_UNAVAILABLE|WCL_BUSY;

	return j;
}

void jobDiscardChunkHandler(struct producer_s* p, job_s* j)
{
	if(!p || !j)return;
	jobDiscardChunk_s* d=(jobDiscardChunk_s*)j->data;

	if(!d->target->next)return; //if chunk is *not* in a list, we shouldn't be discarding it

	const u32 size=CLUSTER_SIZE*CLUSTER_SIZE*CLUSTER_SIZE;
	int i; for(i=0;i<CHUNK_HEIGHT;i++)memcpy(&(((u8*)p->tmpBuffer)[i*size]), d->target->data[i].data, size);
	saveChunk(&d->world->stream, d->target->position.x, d->target->position.z, (u8*)p->tmpBuffer);
}

void jobDiscardChunkFinalizer(job_s* j)
{
	if(!j)return;
	jobDiscardChunk_s* d=(jobDiscardChunk_s*)j->data;

	d->target->modified=false;
	int i; for(i=0; i<CHUNK_HEIGHT; i++)d->target->data[i].status&=~(WCL_DATA_UNAVAILABLE|WCL_BUSY);
	fixChunk(d->target);
}

jobType_s jobTypes[NUM_JOB_TYPES]= {
	(jobType_s){jobGenerateChunkDataHandler, jobGenerateChunkDataFinalizer, sizeof(jobGenerateChunkData_s)}, // JOB_GENERATE_CHUNK_DATA
	(jobType_s){jobGenerateClusterGeometryHandler, jobGenerateClusterGeometryFinalizer, sizeof(jobGenerateClusterGeometryData_s)}, // JOB_GENERATE_CLUSTER_GEOM
	(jobType_s){jobGenerateAdditionalClusterGeometryHandler, jobGenerateAdditionalClusterGeometryFinalizer, sizeof(jobGenerateAdditionalClusterGeometryData_s)}, // JOB_GENERATE_ADDITIONAL_CLUSTER_GEOM
	(jobType_s){jobDiscardChunkHandler, jobDiscardChunkFinalizer, sizeof(jobDiscardChunk_s)}, // JOB_DISCARD_CHUNK
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
