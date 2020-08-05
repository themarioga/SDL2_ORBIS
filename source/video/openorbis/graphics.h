/** Mix from OpenOrbis Graphic Sample and OrbisDev liborbis2d
Thanks to SpecterDev, masterzorag and their teams **/

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdbool.h>
#include <stdint.h>
#include <orbis/libkernel.h>
#include <orbis/VideoOut.h>
#include <orbis/Sysmodule.h>

// Color is used to pack together RGB information, and is used for every function that draws colored pixels.
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Scene2DColor;

typedef struct {
	uint32_t *datap;
	unsigned short width;
	unsigned short height;
	unsigned short depth;
} Scene2DTexture __attribute__ ((aligned (16)));

typedef struct {
	int width;
	int height;
	int depth;
	int video;
	
	off_t directMemOff;
	size_t directMemAllocationSize;
	
	uintptr_t videoMemSP;
	void *videoMem;
	
	char **frameBuffers;
	OrbisKernelEqueue flipQueue;
	OrbisVideoOutBufferAttribute attr;
	
	int frameBufferSize;
	int frameBufferCount;
	
	int activeFrameBufferIdx;
} Scene2D;

Scene2D *Init(int w, int h, int pixelDepth, size_t memSize, int numFrameBuffers);

void SetActiveFrameBuffer(Scene2D *scene2D, int index);
void SubmitFlip(Scene2D *scene2D, int frameID);

void FrameWait(Scene2D *scene2D, int frameID);
void FrameBufferSwap(Scene2D *scene2D);
void FrameBufferClear(Scene2D *scene2D);
void FrameBufferFill(Scene2D *scene2D, Scene2DColor color);

void DrawPixel(Scene2D *scene2D, int x, int y, Scene2DColor color);
void DrawLine(Scene2D *scene2D, int x1, int y1, int x2, int y2, Scene2DColor color);
void DrawRectangle(Scene2D *scene2D, int x, int y, int w, int h, Scene2DColor color);

Scene2DTexture* CreateEmptyTexture(unsigned int w, unsigned int h);
void DestroyTexture(Scene2DTexture *texture);

//Internal
bool initFlipQueue(Scene2D *scene2D);
bool allocateFrameBuffers(Scene2D *scene2D, int num);
char *allocateDisplayMem(Scene2D *scene2D, size_t size);
bool allocateVideoMem(Scene2D *scene2D, size_t size, int alignment);
void deallocateVideoMem(Scene2D *scene2D);

#endif
