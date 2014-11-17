#ifndef GENERATION_H
#define GENERATION_H

#include <3ds.h>
#include "world.h"

void initGeneration();
void exitGeneration();

void generateWorldChunkInfo(worldChunk_s* wch, worldChunkInfo_s* wci);
void generateWorldCluster(worldChunkInfo_s* wci, worldCluster_s* wcl);

#endif
