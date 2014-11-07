#ifndef STREAMING_H
#define STREAMING_H

#include <3ds.h>

#include "math.h"

#define WORLDFILE_MAGIC (0x43534433)
#define STREAMPATH_LENGTH (256)
#define FILELISTBLOCK_LENGTH (64)

typedef struct
{
	u32 magic; //MAGIC
	u32 first; //offset to first list
	u32 last; //offset to last list
	u32 reserved[16];
}fileWorldHeader_s;

typedef struct
{
	struct {
		s32 x, z;
		u32 offset;
	} list[FILELISTBLOCK_LENGTH];
	u16 length; //list length
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
	fileListHeader_s current;
	char path[STREAMPATH_LENGTH];
	Handle file;
	u32 size;
	bool flushed;
}worldStream_s;

void initWorldStream(worldStream_s* ws, char* path);
bool isChunkInStream(worldStream_s* ws, s32 x, s32 z);
int getChunkFromStream(worldStream_s* ws, s32 x, s32 z, u8* dst);
void saveChunk(worldStream_s* ws, s32 x, s32 z, u8* data);
void exitWorldStream(worldStream_s* ws);

#endif
