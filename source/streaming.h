#ifndef STREAMING_H
#define STREAMING_H

#include <3ds.h>

#include "math.h"
#include "world.h"

#define STREAMPATH_LENGTH (256)
#define FILELISTBLOCK_LENGTH (64)

typedef struct
{
	u32 reserved[16];
}fileWorldHeader_s;

typedef struct
{
	struct {
		s32 x, z;
		u32 offset;
	} list[FILELISTBLOCK_LENGTH];
	u32 next; //file offset
	u32 reserved[8];
}fileListHeader_s;

typedef struct chunkInfo_s
{
	vect3Di_s position;
	u32 offset;
	struct chunkInfo_s* next;
}chunkInfo_s;

typedef struct
{
	//for now this is just a simple list, we do naive linear search...
	chunkInfo_s* first;
	int length;
}chunkRepository_s;

typedef struct
{
	fileWorldHeader_s header;
	chunkRepository_s chunks;
	char path[STREAMPATH_LENGTH];
	Handle file;
}worldStream_s;

void initWorldStream(worldStream_s* ws, char* path);
bool isChunkInStream(worldStream_s* ws, s32 x, s32 z);
int getChunkFromStream(worldStream_s* ws, s32 x, s32 z, u8* dst);
void exitWorldStream(worldStream_s* ws);

#endif
