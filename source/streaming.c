#include <3ds.h>

#include "text.h"
#include "world.h"
#include "streaming.h"
#include "configuration.h"

#define CHUNK_DATASIZE (CLUSTER_SIZE*CLUSTER_SIZE*CLUSTER_SIZE*CHUNK_HEIGHT)

void initChunkRepository(chunkRepository_s* cr)
{
	if(!cr)return;

	cr->first=NULL;
	cr->length=0;
}

void addChunkToRepository(chunkRepository_s* cr, s32 x, s32 z, u32 offset)
{
	if(!cr || !offset)return;

	chunkInfo_s* ci=malloc(sizeof(chunkInfo_s));
	if(!ci)return;

	ci->position=vect3Di(x,0,z);
	ci->offset=offset;
	ci->next=cr->first;
	cr->first=ci;
	cr->length++;
}

chunkInfo_s* getChunkFromRepository(chunkRepository_s* cr, s32 x, s32 z)
{
	if(!cr)return NULL;

	chunkInfo_s* ci=cr->first;
	while(ci)
	{
		if(ci->position.x==x && ci->position.z==z)return ci;
		ci=ci->next;
	}
	return NULL;
}

int readWorldStreamHeader(worldStream_s* ws)
{
	if(!ws)return -1;

	u32 bytesRead=0;
	Result ret=FSFILE_Read(ws->file, &bytesRead, 0, &ws->header, sizeof(fileWorldHeader_s));
	if(ret || bytesRead!=sizeof(fileWorldHeader_s))return -1;
	ws->size+=bytesRead;

	return 0;
}

int readWorldStream(worldStream_s* ws)
{
	if(!ws)return -1;

	fileListHeader_s* buffer=&ws->current;
	u32 offset=ws->header.first;
	u32 bytesRead=0;
	do{
		Result ret=FSFILE_Read(ws->file, &bytesRead, offset, buffer, sizeof(fileListHeader_s));
		if(ret || bytesRead!=sizeof(fileListHeader_s))return -1;
		ws->size+=bytesRead;
		print("test %d\n",buffer->length);
		int i; for(i=0;i<FILELISTBLOCK_LENGTH;i++)addChunkToRepository(&ws->chunks, buffer->list[i].x, buffer->list[i].z, buffer->list[i].offset);
		offset=buffer->next;
	}while(offset);

	return 0;
}

void createFileList(fileListHeader_s* flh)
{
	if(!flh)return;

	flh->length=0;
	flh->next=0;
	memset(flh->list, 0x00, sizeof(flh->next));
}

int addChunkToList(fileListHeader_s* flh, s32 x, s32 z, u32 offset)
{
	if(!flh)return -1;
	if(flh->length>=FILELISTBLOCK_LENGTH)return -2; //no room left !

	flh->list[flh->length].x=x;
	flh->list[flh->length].z=z;
	flh->list[flh->length].offset=offset;

	flh->length++;

	return 0;
}

void flushWorldStream(worldStream_s* ws)
{
	if(!ws || ws->flushed)return;
	u32 bytesWritten=0;
	Result ret;

	ret=FSFILE_Write(ws->file, &bytesWritten, 0, &ws->header, sizeof(fileWorldHeader_s), FS_WRITE_FLUSH);
	if(ret || bytesWritten!=sizeof(fileWorldHeader_s))return;

	ret=FSFILE_Write(ws->file, &bytesWritten, ws->header.last, &ws->current, sizeof(fileListHeader_s), FS_WRITE_FLUSH);
	if(ret || bytesWritten!=sizeof(fileListHeader_s))return;

	ws->flushed=true;
}

u32 addChunkToWorldStream(worldStream_s* ws, s32 x, s32 z, u8* data)
{
	if(!ws || !data)return 0;
	u32 bytesWritten=0;
	u32 offset=ws->size;

	Result ret=FSFILE_Write(ws->file, &bytesWritten, offset, data, CHUNK_DATASIZE, FS_WRITE_FLUSH);
	if(ret || bytesWritten!=CHUNK_DATASIZE)return 0;
	ws->size+=bytesWritten;

	ret=addChunkToList(&ws->current, x, z, offset);
	if(ret && ret!=-2)return 0;
	if(ret==-2)
	{
		ws->current.next=ws->size;
		ws->flushed=false;
		flushWorldStream(ws);
		createFileList(&ws->current);
		addChunkToList(&ws->current, x, z, offset);
		ws->header.last=ws->size;
		ws->size+=sizeof(fileListHeader_s);
	}

	ws->flushed=false;
	return offset;
}

void saveChunk(worldStream_s* ws, s32 x, s32 z, u8* data)
{
	if(!ws || !data)return;

	chunkInfo_s* ci=getChunkFromRepository(&ws->chunks, x, z);
	if(ci)
	{
		//overwrite chunk that already exists
		u32 bytesWritten=0;
		Result ret=FSFILE_Write(ws->file, &bytesWritten, ci->offset, data, CHUNK_DATASIZE, FS_WRITE_FLUSH);
		if(ret || bytesWritten!=CHUNK_DATASIZE)return;
	}else{
		//add new chunk to file...
		u32 offset=addChunkToWorldStream(ws, x, z, data);
		//...and repository
		addChunkToRepository(&ws->chunks, x, z, offset);
	}
}

bool loadChunk(worldStream_s* ws, s32 x, s32 z, u8* dst)
{
	if(!ws || !dst)return false;

	chunkInfo_s* ci=getChunkFromRepository(&ws->chunks, x, z);
	if(!ci)return false;

	print("found %d %d\n",x,z);

	u32 bytesRead=0;
	Result ret=FSFILE_Read(ws->file, &bytesRead, ci->offset, dst, CHUNK_DATASIZE);
	if(ret || bytesRead!=CHUNK_DATASIZE)return false;

	return true;
}

int createWorldStream(worldStream_s* ws)
{
	if(!ws)return -1;
	Result ret;

	ret=FSUSER_OpenFile(NULL, &ws->file, configuration.sdmc, FS_makePath(PATH_CHAR, ws->path), FS_OPEN_READ|FS_OPEN_WRITE|FS_OPEN_CREATE, FS_ATTRIBUTE_NONE);
	if(ret){svcCloseHandle(ws->file); return -2;}

	u32 bytesWritten=0;

	ws->header.magic=WORLDFILE_MAGIC;
	ws->header.first=ws->header.last=sizeof(fileWorldHeader_s);

	ws->size=0;
	ret=FSFILE_Write(ws->file, &bytesWritten, ws->size, &ws->header, sizeof(fileWorldHeader_s), FS_WRITE_FLUSH);
	if(ret || bytesWritten!=sizeof(fileWorldHeader_s))return -1;
	ws->size+=bytesWritten;

	createFileList(&ws->current);
	ret=FSFILE_Write(ws->file, &bytesWritten, ws->size, &ws->current, sizeof(fileListHeader_s), FS_WRITE_FLUSH);
	if(ret || bytesWritten!=sizeof(fileListHeader_s))return -1;
	ws->size+=bytesWritten;

	return 0;
}

void initWorldStream(worldStream_s* ws, char* path)
{
	if(!ws || !path)return;

	ws->size=0;
	initChunkRepository(&ws->chunks);
	strncpy(ws->path, path, STREAMPATH_LENGTH);

	Result ret=FSUSER_OpenFile(NULL, &ws->file, configuration.sdmc, FS_makePath(PATH_CHAR, ws->path), FS_OPEN_READ|FS_OPEN_WRITE, FS_ATTRIBUTE_NONE);
	if(ret)
	{
		svcCloseHandle(ws->file);
		if(createWorldStream(ws)){svcCloseHandle(ws->file); return;}
	}else{
		readWorldStreamHeader(ws);
		print("STREAM HEADER %08X %08X %08X\n",ws->header.magic,ws->header.first,ws->header.last);
		readWorldStream(ws);
		print("STREAM %d chunks\n",ws->chunks.length);
	}
	ret=FSFILE_GetSize(ws->file, &ws->size);
	ws->flushed=true;
}

int getChunkFromStream(worldStream_s* ws, s32 x, s32 z, u8* dst)
{
	if(!ws || !dst)return -1;
	
	chunkInfo_s* ci=getChunkFromRepository(&ws->chunks, x, z);
	if(ci)
	{
		u32 bytesRead=0;
		u32 size=CHUNK_DATASIZE;
		Result ret=FSFILE_Read(ws->file, &bytesRead, ci->offset, dst, size);
		if(ret || bytesRead!=size)return -2;
		return 0;
	}

	return -3;
}

bool isChunkInStream(worldStream_s* ws, s32 x, s32 z)
{
	if(!ws)return false;

	return getChunkFromRepository(&ws->chunks, x, z)!=NULL;
}

void exitWorldStream(worldStream_s* ws)
{
	if(!ws)return;

	flushWorldStream(ws);
	FSFILE_Close(ws->file);
	svcCloseHandle(ws->file);
}
