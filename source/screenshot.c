#include <stdio.h>

#include "bmp.h"
#include "configuration.h"
#include "string.h"

#define MAX_SCREENSHOTS (1024)

//i took this code from DScraft and i'm pretty sure I didn't write it myself back then
//if you're the person who wrote this code and would like credit, please let me know; I'd be happy to oblige

void write16(u16* address, u16 value) {

	u8* first=(u8*)address;
	u8* second=first+1;

	*first=value&0xff;
	*second=value>>8;
}

void write32(u32* address, u32 value) {

	u8* first=(u8*)address;
	u8* second=first+1;
	u8* third=first+2;
	u8* fourth=first+3;

	*first=value&0xff;
	*second=(value>>8)&0xff;
	*third=(value>>16)&0xff;
	*fourth=(value>>24)&0xff;
}

int saveBitmap(const char* filename, u8* buffer, u32 w, u32 h)
{
	u8* temp=(u8*)malloc(w*h*3+sizeof(INFOHEADER)+sizeof(HEADER));

	HEADER* header=(HEADER*)temp;
	INFOHEADER* infoheader=(INFOHEADER*)(temp+sizeof(HEADER));

	write16(&header->type, 0x4D42);
	write32(&header->size, w*h*3+sizeof(INFOHEADER)+sizeof(HEADER));
	write32(&header->offset, sizeof(INFOHEADER)+sizeof(HEADER));
	write16(&header->reserved1, 0);
	write16(&header->reserved2, 0);

	write16(&infoheader->bits, 24);
	write32(&infoheader->size, sizeof(INFOHEADER));
	write32(&infoheader->compression, 0);
	write32(&infoheader->width, w);
	write32(&infoheader->height, h);
	write16(&infoheader->planes, 1);
	write32(&infoheader->imagesize, w*h*3);
	write32(&infoheader->xresolution, 0);
	write32(&infoheader->yresolution, 0);
	write32(&infoheader->importantcolours, 0);
	write32(&infoheader->ncolours, 0);
	int y,x;
	for(y=0;y<h;y++)
	{
		for(x=0;x<w;x++)
		{
			temp[((y*w)+x)*3+sizeof(INFOHEADER)+sizeof(HEADER)]=buffer[(x*h+y)*3+0];
			temp[((y*w)+x)*3+1+sizeof(INFOHEADER)+sizeof(HEADER)]=buffer[(x*h+y)*3+1];
			temp[((y*w)+x)*3+2+sizeof(INFOHEADER)+sizeof(HEADER)]=buffer[(x*h+y)*3+2];
		}
	}

	Handle file;
	Result ret=FSUSER_OpenFile(NULL, &file, configuration.sdmc, FS_makePath(PATH_CHAR, filename), FS_OPEN_WRITE|FS_OPEN_CREATE, FS_ATTRIBUTE_NONE);
	if(ret){svcCloseHandle(file); return -2;}

	u32 size=w*h*3+sizeof(INFOHEADER)+sizeof(HEADER);
	u32 bytesWritten=0;

	ret=FSFILE_Write(file, &bytesWritten, 0, temp, size, FS_WRITE_FLUSH);
	if(ret || bytesWritten!=size)return -2;

	FSFILE_Close(file);
	svcCloseHandle(file);
	free(temp);

	return 0;
}

int screenshotCnt;

void initScreenshot()
{
	screenshotCnt=0;
}

void getNextScreenshotCnt()
{
	//open files while incrementing screenshotCnt until file doesn't exist
	static char path[256];
	Result ret=0;
	while(!ret && screenshotCnt<MAX_SCREENSHOTS)
	{
		screenshotCnt++;
		snprintf(path, 256, "%s/scr_%d_left.bmp", configuration.path, screenshotCnt);
		Handle file;
		ret=FSUSER_OpenFile(NULL, &file, configuration.sdmc, FS_makePath(PATH_CHAR, path), FS_OPEN_READ, FS_ATTRIBUTE_NONE);
		FSFILE_Close(file);
		svcCloseHandle(file);
	}
}

void saveScreenshot()
{
	static char path[256];
	getNextScreenshotCnt();
	if(screenshotCnt<MAX_SCREENSHOTS)
	{
		snprintf(path, 256, "%s/scr_%d_left.bmp", configuration.path, screenshotCnt);
		saveBitmap(path, gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), 400, 240);
		if(CONFIG_3D_SLIDERSTATE>0.0f)
		{
			snprintf(path, 256, "%s/scr_%d_right.bmp", configuration.path, screenshotCnt);
			saveBitmap(path, gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL), 400, 240);
		}
	}
}

void exitScreenshot()
{

}
