#ifndef _bmp_h_
#define _bmp_h_

#include <3ds.h>

typedef struct {
	u16 type;						/* Magic identifier            */
	u32 size;                       /* File size in bytes          */
	u16 reserved1, reserved2;
	u32 offset;                     /* Offset to image data, bytes */
} __attribute__ ((__packed__)) HEADER;

typedef struct {
	u32 size;						/* Header size in bytes      */
	u32 width,height;				/* Width and height of image */
	u16 planes;						/* Number of colour planes   */
	u16 bits;						/* Bits per pixel            */
	u32 compression;				/* Compression type          */
	u32 imagesize;					/* Image size in bytes       */
	u32 xresolution,yresolution;	/* Pixels per meter          */
	u32 ncolours;					/* Number of colours         */
	u32 importantcolours;			/* Important colours         */
} __attribute__ ((__packed__)) INFOHEADER;

int saveBitmap(const char* filename, u8* buffer, u32 w, u32 h);

void initScreenshot();
void saveScreenshot();
void exitScreenshot();

#endif //_bmp_h_
