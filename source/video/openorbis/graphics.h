#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdbool.h>
#include <stdint.h>
#include <orbis/libkernel.h>
#include <orbis/VideoOut.h>
#include <orbis/Sysmodule.h>

// Color is used to pack together RGB information, and is used for every function that draws colored pixels.
typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Color;

typedef struct
{
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
void FrameBufferFill(Scene2D *scene2D, Color color);

void DrawPixel(Scene2D *scene2D, int x, int y, Color color);
void DrawRectangle(Scene2D *scene2D, int x, int y, int w, int h, Color color);

bool initFlipQueue(Scene2D *scene2D);
bool allocateFrameBuffers(Scene2D *scene2D, int num);
char *allocateDisplayMem(Scene2D *scene2D, size_t size);
bool allocateVideoMem(Scene2D *scene2D, size_t size, int alignment);
void deallocateVideoMem(Scene2D *scene2D);

#endif
