#include <3ds.h>

#include "text.h"
#include "configuration.h"

configuration_s configuration;

static const char* defaultPath="/";
static const FS_archive sdmcArchive={0x00000009, (FS_path){PATH_EMPTY, 1, (u8*)""}};

void initConfiguration(configuration_s* c, int argc, char** argv)
{
	if(!c)c=&configuration;
	if(argc && argv)
	{
		//grab path from arguments
	}else{
		c->path=(char*)defaultPath;
	}

	c->sdmc=sdmcArchive;
	Result ret=FSUSER_OpenArchive(NULL, &c->sdmc);
	if(ret)print("couldn't open SDMC : %08X\n",(unsigned int)ret);
	strncpy(c->currentFile, "test.world", 32);
}
