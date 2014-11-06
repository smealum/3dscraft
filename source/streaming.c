#include <3ds.h>

#include "world.h"
#include "streaming.h"
#include "configuration.h"

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
}

bool isChunkInRepository(chunkRepository_s* cr, s32 x, s32 z)
{
	if(!cr)return false;

	chunkInfo_s* ci=cr->first;
	while(ci)
	{
		if(ci->position.x==x && ci->position.z==z)return true;
		ci=ci->next;
	}
	return false;
}

u32 getChunkOffset(chunkRepository_s* cr, s32 x, s32 z)
{
	if(!cr)return 0;

	chunkInfo_s* ci=cr->first;
	while(ci)
	{
		if(ci->position.x==x && ci->position.z==z)return ci->offset;
		ci=ci->next;
	}
	return 0;
}

int readWorldStreamHeader(worldStream_s* ws)
{
	if(!ws)return -1;

	u32 bytesRead=0;
	Result ret=FSFILE_Read(ws->file, &bytesRead, 0, &ws->header, sizeof(fileWorldHeader_s));
	if(ret || bytesRead!=sizeof(fileWorldHeader_s))return -1;

	return 0;
}

int readWorldStream(worldStream_s* ws)
{
	if(!ws)return -1;

	static fileListHeader_s buffer;
	u32 offset=sizeof(fileWorldHeader_s);
	u32 bytesRead=0;
	do{
		Result ret=FSFILE_Read(ws->file, &bytesRead, offset, &buffer, sizeof(fileListHeader_s));
		if(ret || bytesRead!=sizeof(fileListHeader_s))return -1;
		int i; for(i=0;i<FILELISTBLOCK_LENGTH;i++)addChunkToRepository(&ws->chunks, buffer.list[i].x, buffer.list[i].z, buffer.list[i].offset);
		offset=buffer.next;
	}while(offset);

	return 0;
}

void initWorldStream(worldStream_s* ws, char* path)
{
	if(!ws || !path)return;

	initChunkRepository(&ws->chunks);
	strncpy(ws->path, path, STREAMPATH_LENGTH);

	Result ret=FSUSER_OpenFile(NULL, &ws->file, configuration.sdmc, FS_makePath(PATH_CHAR, ws->path), FS_OPEN_READ, FS_ATTRIBUTE_NONE);
	if(ret){svcCloseHandle(ws->file); return;}

	readWorldStreamHeader(ws);
	readWorldStream(ws);
}

int getChunkFromStream(worldStream_s* ws, s32 x, s32 z, u8* dst)
{
	if(!ws || !dst)return -1;
	
	u32 offset=getChunkOffset(&ws->chunks, x, z);
	if(offset)
	{
		u32 bytesRead=0;
		u32 size=CLUSTER_SIZE*CLUSTER_SIZE*CLUSTER_SIZE*CHUNK_HEIGHT;
		Result ret=FSFILE_Read(ws->file, &bytesRead, offset, dst, size);
		if(ret || bytesRead!=size)return -2;
		return 0;
	}

	return -3;
}

bool isChunkInStream(worldStream_s* ws, s32 x, s32 z)
{
	if(!ws)return false;

	return isChunkInRepository(&ws->chunks, x, z);
}

void exitWorldStream(worldStream_s* ws)
{
	if(!ws)return;

	FSFILE_Close(ws->file);
	svcCloseHandle(ws->file);
}
