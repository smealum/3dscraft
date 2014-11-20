#include <3ds.h>

#include "text.h"
#include "configuration.h"

configuration_s configuration;

static const char* defaultPath="/";
static const FS_archive sdmcArchive={0x00000009, (FS_path){PATH_EMPTY, 1, (u8*)""}};

void initConfiguration(configuration_s* c, int argc, char** argv)
{
	if(!c)c=&configuration;
	if(argc && argv && !memcmp("sdmc:", argv[0], 5))
	{
		//grab path from arguments
		int l;
		for(l=strlen(argv[0]); l>=0 && argv[0][l]!='/'; l--);
		l-=5;
		c->path=malloc(l+1);
		memcpy(c->path, &argv[0][5], l);
		c->path[l+1]=0x00;
	}else{
		c->path=(char*)defaultPath;
	}
	print("%s\n%s\n",argv[0],c->path);
	c->sdmc=sdmcArchive;
	Result ret=FSUSER_OpenArchive(NULL, &c->sdmc);
	if(ret)print("couldn't open SDMC : %08X\n",(unsigned int)ret);
	strncpy(c->currentFile, "test.world", 32);
}
