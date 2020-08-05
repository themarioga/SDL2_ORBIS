#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "graphics.h"

// Linearly interpolate x with y over s
inline float lerp(float x, float y, float s){
    return x * (1.0f - s) + y * s;
}

Scene2D* Init(int w, int h, int pixelDepth, size_t memSize, int numFrameBuffers){
	Scene2D *ret = (Scene2D*)malloc(sizeof(Scene2D));

	ret->width = w;
	ret->height = h;
	ret->depth = pixelDepth;
	
	ret->frameBufferSize = ret->width * ret->height * ret->depth;

	ret->video = sceVideoOutOpen(ORBIS_VIDEO_USER_MAIN, ORBIS_VIDEO_OUT_BUS_MAIN, 0, 0);
	ret->videoMem = NULL;

	if (ret->video < 0) {
		printf("Failed to open a video out handle: %s", strerror(errno));
		return NULL;
	}
	
	if(!initFlipQueue(ret)) {
		printf("Failed to initialize flip queue: %s", strerror(errno));
		return NULL;
	}
	
	if(!allocateVideoMem(ret, memSize, 0x200000)) {
		printf("Failed to allocate video memory: %s", strerror(errno));
		return NULL;
	}
	
	if(!allocateFrameBuffers(ret, numFrameBuffers)) {
		printf("Failed to allocate frame buffers: %s", strerror(errno));
		return NULL;
	}
	
	sceVideoOutSetFlipRate(ret->video, 0);

	return ret;
}

bool initFlipQueue(Scene2D *scene2D){
	int rc = sceKernelCreateEqueue(&scene2D->flipQueue, "homebrew flip queue");
	
	if(rc < 0) return false;
		
	sceVideoOutAddFlipEvent(scene2D->flipQueue, scene2D->video, 0);
	return true;
}

bool allocateFrameBuffers(Scene2D *scene2D, int num){
	// Allocate frame buffers array
	scene2D->frameBuffers = (char**) malloc(num * sizeof(char *));
	
	// Set the display buffers
	for(int i = 0; i < num; i++)
		scene2D->frameBuffers[i] = allocateDisplayMem(scene2D, scene2D->frameBufferSize);

	// Set SRGB pixel format
	sceVideoOutSetBufferAttribute(&scene2D->attr, 0x80000000, 1, 0, scene2D->width, scene2D->height, scene2D->width);
	
	// Register the buffers to the video handle
	return (sceVideoOutRegisterBuffers(scene2D->video, 0, (void **)scene2D->frameBuffers, num, &scene2D->attr) == 0);
}

char *allocateDisplayMem(Scene2D *scene2D, size_t size){
	// Essentially just bump allocation
	char *allocatedPtr = (char *)scene2D->videoMemSP;
	scene2D->videoMemSP += size;

	return allocatedPtr;
}

bool allocateVideoMem(Scene2D *scene2D, size_t size, int alignment){
	int rc;
	
	// Align the allocation size
	scene2D->directMemAllocationSize = (size + alignment - 1) / alignment * alignment;
	
	// Allocate memory for display buffer
	rc = sceKernelAllocateDirectMemory(0, sceKernelGetDirectMemorySize(), scene2D->directMemAllocationSize, alignment, 3, &scene2D->directMemOff);
	
	if(rc < 0) {
		scene2D->directMemAllocationSize = 0;
		return false;
	}
	
	// Map the direct memory
	rc = sceKernelMapDirectMemory(&scene2D->videoMem, scene2D->directMemAllocationSize, 0x33, 0, scene2D->directMemOff, alignment);
	
	if(rc < 0) {
		sceKernelReleaseDirectMemory(scene2D->directMemOff, scene2D->directMemAllocationSize);
		
		scene2D->directMemOff = 0;
		scene2D->directMemAllocationSize = 0;
		
		return false;
	}
	
	// Set the stack pointer to the beginning of the buffer
	scene2D->videoMemSP = (uintptr_t)scene2D->videoMem;
	return true;
}

void deallocateVideoMem(Scene2D *scene2D){
	// Free the direct memory
	sceKernelReleaseDirectMemory(scene2D->directMemOff, scene2D->directMemAllocationSize);
	
	// Zero out meta data
	scene2D->videoMem = 0;
	scene2D->videoMemSP = 0;
	scene2D->directMemOff = 0;
	scene2D->directMemAllocationSize = 0;
	
	// Free the frame buffer array
	free(scene2D->frameBuffers);
	scene2D->frameBuffers = 0;
}

void SetActiveFrameBuffer(Scene2D *scene2D, int index){
	scene2D->activeFrameBufferIdx = index;
}

void SubmitFlip(Scene2D *scene2D, int frameID){
	sceVideoOutSubmitFlip(scene2D->video, scene2D->activeFrameBufferIdx, ORBIS_VIDEO_OUT_FLIP_VSYNC, frameID);
}

void FrameWait(Scene2D *scene2D, int frameID){
	OrbisKernelEvent evt;
	int count;
	
	// If the video handle is not initialized, bail out. This is mostly a failsafe, this should never happen.
	if(scene2D->video == 0) return;
		
	for(;;) {
		OrbisVideoOutFlipStatus flipStatus;
		
		// Get the flip status and check the arg for the given frame ID
		sceVideoOutGetFlipStatus(scene2D->video, &flipStatus);
		
		if(flipStatus.flipArg == frameID) break;
			
		// Wait on next flip event
		if(sceKernelWaitEqueue(scene2D->flipQueue, &evt, 1, &count, 0) != 0) break;
	}
}

void FrameBufferSwap(Scene2D *scene2D){
	// Swap the frame buffer for some perf
	scene2D->activeFrameBufferIdx = (scene2D->activeFrameBufferIdx + 1) % 2;
}

void FrameBufferClear(Scene2D *scene2D){
	// Clear the screen with a white frame buffer
	Scene2DColor blank = { 255, 255, 255 };
	FrameBufferFill(scene2D, blank);
}

void FrameBufferFill(Scene2D *scene2D, Scene2DColor color){
	DrawRectangle(scene2D, 0, 0, scene2D->width, scene2D->height, color);
}

void DrawRectangle(Scene2D *scene2D, int x, int y, int w, int h, Scene2DColor color){
	int xPos, yPos;
	
	// Draw row-by-row, column-by-column
	for(yPos = y; yPos < y + h; yPos++)
	{
		for(xPos = x; xPos < x + w; xPos++)
		{
			DrawPixel(scene2D, xPos, yPos, color);
		}
	}
}

void DrawLine(Scene2D *scene2D, int x1, int y1, int x2, int y2, Scene2DColor color){
	int xPos, yPos;

	int dx = x2 - x1;
	int dy = y2 - y1;

	for (int xPos = x1; xPos < x2; xPos++) {
		yPos = y1 + dy * (xPos - x1);
		if (dx != 0) yPos = yPos / dx;
		DrawPixel(scene2D, xPos, yPos, color);
	}
}

void DrawPixel(Scene2D *scene2D, int x, int y, Scene2DColor color){
	// Get pixel location based on pitch
	int pixel = (y * scene2D->width) + x;
	
	// Encode to 24-bit color
	uint32_t encodedColor = 0x80000000 + (color.r << 16) + (color.g << 8) + color.b;
	
	// Draw to the frame buffer
	((uint32_t *)scene2D->frameBuffers[scene2D->activeFrameBufferIdx])[pixel] = encodedColor;
}

Scene2DTexture* CreateEmptyTexture(unsigned int w, unsigned int h) {
	Scene2DTexture *img = NULL;
	img = malloc(sizeof(Scene2DTexture));

	if (img!=NULL) {
		img->datap=malloc(w*h*4);
		if(img->datap==NULL) {
			free(img);
			return NULL;
		}
		img->width=w;
		img->height=h;
		img->depth=32;
	}
	return img;
}

void DestroyTexture(Scene2DTexture *texture) {
	if(texture != NULL) {
		if(texture->datap != NULL) {
			free(texture->datap);
		}
	}
	free(texture);
}