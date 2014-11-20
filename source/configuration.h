#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <3ds.h>

#define CONFIG_3D_SLIDERSTATE (*(volatile float*)0x1FF81080)

typedef struct
{
	char* path;
	FS_archive sdmc;
	char currentFile[32];
}configuration_s;

extern configuration_s configuration;

void initConfiguration(configuration_s* c, int argc, char** argv);

#endif
