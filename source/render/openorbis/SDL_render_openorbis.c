/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2015 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "../../SDL_internal.h"

#if SDL_VIDEO_RENDER_OPENORBIS

#include <orbis/libkernel.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>

#include "SDL_render_openorbis.h"

void
StartDrawing(SDL_Renderer *renderer){
	OPENORBIS_RenderData *data = (OPENORBIS_RenderData *) renderer->driverdata;
	if(data->displayListAvail)
		return;

	SDL_WindowData *windowData = (SDL_WindowData *)renderer->window->driverdata;

	FrameWait(windowData->scene, windowData->frame);

	data->displayListAvail = SDL_TRUE;
}

SDL_Renderer *
OPENORBIS_CreateRenderer(SDL_Window *window, Uint32 flags){
	SDL_Renderer *renderer;
	OPENORBIS_RenderData *data;

	renderer = (SDL_Renderer *) SDL_calloc(1, sizeof(*renderer));
	if (!renderer) {
		SDL_OutOfMemory();
		return NULL;
	}

	data = (OPENORBIS_RenderData *) SDL_calloc(1, sizeof(*data));
	if (!data) {
		OPENORBIS_DestroyRenderer(renderer);
		SDL_OutOfMemory();
		return NULL;
	}

	renderer->WindowEvent = OPENORBIS_WindowEvent;
	renderer->CreateTexture = OPENORBIS_CreateTexture;
	renderer->UpdateTexture = OPENORBIS_UpdateTexture;
	renderer->LockTexture = OPENORBIS_LockTexture;
	renderer->UnlockTexture = OPENORBIS_UnlockTexture;
	renderer->SetRenderTarget = OPENORBIS_SetRenderTarget;
	renderer->UpdateViewport = OPENORBIS_UpdateViewport;
	renderer->RenderClear = OPENORBIS_RenderClear;
	renderer->RenderDrawPoints = OPENORBIS_RenderDrawPoints;
	renderer->RenderDrawLines = OPENORBIS_RenderDrawLines;
	renderer->RenderFillRects = OPENORBIS_RenderFillRects;
	renderer->RenderCopy = OPENORBIS_RenderCopy;
	renderer->RenderReadPixels = OPENORBIS_RenderReadPixels;
	renderer->RenderCopyEx = OPENORBIS_RenderCopyEx;
	renderer->RenderPresent = OPENORBIS_RenderPresent;
	renderer->DestroyTexture = OPENORBIS_DestroyTexture;
	renderer->DestroyRenderer = OPENORBIS_DestroyRenderer;
	renderer->info = OPENORBIS_RenderDriver.info;
	renderer->info.flags = (SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
	renderer->driverdata = data;
	renderer->window = window;

	if (data->initialized != SDL_FALSE)
		return 0;

	if(OPENORBIS_initialized==1) {
		data->initialized = SDL_TRUE;

		if (flags & SDL_RENDERER_PRESENTVSYNC) {
			data->vsync = SDL_TRUE;
		} else {
			data->vsync = SDL_FALSE;
		}
		return renderer;
	}

	return NULL;
}

static void
OPENORBIS_WindowEvent(SDL_Renderer *renderer, const SDL_WindowEvent *event){

}

static int
OPENORBIS_CreateTexture(SDL_Renderer *renderer, SDL_Texture *texture){
	OPENORBIS_TextureData* openorbis_texture = (OPENORBIS_TextureData *) SDL_calloc(1, sizeof(*openorbis_texture));

	if(!openorbis_texture)
		return -1;

	openorbis_texture->texture = CreateEmptyTexture(texture->w, texture->h);

	if(!openorbis_texture->texture->datap)
	{
		SDL_free(openorbis_texture);
		return SDL_OutOfMemory();
	}

	/* 
	set texture filtering according to SDL_HINT_RENDER_SCALE_QUALITY
	suported hint values are nearest (0, default) or linear (1)
	scaleMode is either SCE_GXM_TEXTURE_FILTER_POINT (good for tile-map)
	or SCE_GXM_TEXTURE_FILTER_LINEAR (good for scaling)
	*/
	openorbis_texture->w = openorbis_texture->texture->width;
	openorbis_texture->h = openorbis_texture->texture->height;
	openorbis_texture->pitch = openorbis_texture->w *SDL_BYTESPERPIXEL(texture->format);

	texture->driverdata = openorbis_texture;

	return 0;
}

static int
OPENORBIS_UpdateTexture(SDL_Renderer *renderer, SDL_Texture *texture,
				   const SDL_Rect *rect, const void *pixels, int pitch){
	const Uint8 *src;
	Uint8 *dst;
	int row, length,dpitch;
	src = pixels;

	OPENORBIS_LockTexture(renderer, texture, rect, (void **)&dst, &dpitch);
	length = rect->w * SDL_BYTESPERPIXEL(texture->format);
	if (length == pitch && length == dpitch) {
		SDL_memcpy(dst, src, length*rect->h);
	} else {
		for (row = 0; row < rect->h; ++row) {
			SDL_memcpy(dst, src, length);
			src += pitch;
			dst += dpitch;
		}
	}

	//sceKernelDcacheWritebackAll();
	return 0;
}

static int
OPENORBIS_LockTexture(SDL_Renderer *renderer, SDL_Texture *texture,
				 const SDL_Rect *rect, void **pixels, int *pitch){
	OPENORBIS_TextureData *openorbis_texture = (OPENORBIS_TextureData *) texture->driverdata;

	*pixels =
		(void *) ((Uint8 *) openorbis_texture->texture->datap
			+ (rect->y * openorbis_texture->w + rect->x) * SDL_BYTESPERPIXEL(texture->format));
	*pitch = openorbis_texture->pitch;
	return 0;
}

static void
OPENORBIS_UnlockTexture(SDL_Renderer *renderer, SDL_Texture *texture){

}

static int
OPENORBIS_SetRenderTarget(SDL_Renderer *renderer, SDL_Texture *texture){
	return 0;
}

static int
OPENORBIS_UpdateViewport(SDL_Renderer *renderer){
	return 0;
}


static void
OPENORBIS_SetBlendMode(SDL_Renderer *renderer, int blendMode){
	
}

static int
OPENORBIS_RenderClear(SDL_Renderer *renderer){
	SDL_WindowData *windowData = (SDL_WindowData *)renderer->window->driverdata;
	/* start list */
	StartDrawing(renderer);
	if(OPENORBIS_initialized) {
		Scene2DColor gc;
		gc.r = renderer->r;
		gc.g = renderer->g;
		gc.b = renderer->b;
		FrameBufferFill(windowData->scene, gc);
	}
	
	return 0;
}

static int
OPENORBIS_RenderDrawPoints(SDL_Renderer *renderer, const SDL_FPoint *points, int count){
	SDL_WindowData *windowData = (SDL_WindowData *)renderer->window->driverdata;
	StartDrawing(renderer);
		Scene2DColor gc;
		gc.r = renderer->r;
		gc.g = renderer->g;
		gc.b = renderer->b;

	for (int i = 0; i < count; ++i) {
		DrawPixel(windowData->scene, points[i].x, points[i].y, gc);
	}

	return 0;
}

static int
OPENORBIS_RenderDrawLines(SDL_Renderer *renderer, const SDL_FPoint *points, int count){
	SDL_WindowData *windowData = (SDL_WindowData *)renderer->window->driverdata;
	StartDrawing(renderer);
	Scene2DColor gc;
	gc.r = renderer->r;
	gc.g = renderer->g;
	gc.b = renderer->b;

	for (int i = 0; i < count; ++i) {
		if (i < count -1) {
			DrawLine(windowData->scene, points[i].x, points[i].y, points[i+1].x, points[i+1].y, gc);
		}
	}

	return 0;
}

static int
OPENORBIS_RenderFillRects(SDL_Renderer *renderer, const SDL_FRect *rects, int count){
	SDL_WindowData *windowData = (SDL_WindowData *)renderer->window->driverdata;
	StartDrawing(renderer);
	Scene2DColor gc;
	gc.r = renderer->r;
	gc.g = renderer->g;
	gc.b = renderer->b;

	for (int i = 0; i < count; ++i) {
		const SDL_FRect *rect = &rects[i];
		DrawRectangle(windowData->scene, rect->x, rect->w, rect->y, rect->h, gc);
	}

	return 0;
}


#define PI   3.14159265358979f

#define radToDeg(x) ((x)*180.f/PI)
#define degToRad(x) ((x)*PI/180.f)

float MathAbs(float x){
	return (x < 0) ? -x : x;
}

void MathSincos(float r, float *s, float *c){
	*s = sinf(r);
	*c = cosf(r);
}

void Swap(float *a, float *b){
	float n=*a;
	*a = *b;
	*b = n;
}

static int
OPENORBIS_RenderCopy(SDL_Renderer *renderer, SDL_Texture *texture,
				const SDL_Rect *srcrect, const SDL_FRect *dstrect){
	OPENORBIS_TextureData *openorbis_texture = (OPENORBIS_TextureData *) texture->driverdata;
	float scaleX = dstrect->w == srcrect->w ? 1 : (float)(dstrect->w/srcrect->w);
	float scaleY = dstrect->h == srcrect->h ? 1 : (float)(dstrect->h/srcrect->h);

	StartDrawing(renderer);

	OPENORBIS_SetBlendMode(renderer, renderer->blendMode);

	//float scaleX = dstrect->w > srcrect->w ? (float)(dstrect->w/srcrect->w) : 1;
	//float scaleY = dstrect->h > srcrect->h ? (float)(dstrect->h/srcrect->h) : 1;
	//orbis2dDrawTexturePartScale(openorbis_texture->tex, dstrect->x, dstrect->y,srcrect->x, srcrect->y, srcrect->w, srcrect->h, scaleX, scaleY);

	return 0;
}

static int
OPENORBIS_RenderReadPixels(SDL_Renderer *renderer, const SDL_Rect *rect,
					Uint32 pixel_format, void *pixels, int pitch)
{
		return 0;
}


static int
OPENORBIS_RenderCopyEx(SDL_Renderer *renderer, SDL_Texture *texture,
				const SDL_Rect *srcrect, const SDL_FRect *dstrect,
				const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip){
	return 1;
}

static void
OPENORBIS_RenderPresent(SDL_Renderer *renderer){
	SDL_WindowData *windowData = (SDL_WindowData *)renderer->window->driverdata;
	OPENORBIS_RenderData *data = (OPENORBIS_RenderData *) renderer->driverdata;
	if(!data->displayListAvail)
		return;

 	// Submit the frame buffer
	SubmitFlip(windowData->scene, windowData->frame);
	FrameWait(windowData->scene, windowData->frame);

    // Swap to the next buffer
    FrameBufferSwap(windowData->scene);
    windowData->frame++;

	data->displayListAvail = SDL_FALSE;
}

static void
OPENORBIS_DestroyTexture(SDL_Renderer *renderer, SDL_Texture *texture){
	OPENORBIS_RenderData *renderdata = (OPENORBIS_RenderData *) renderer->driverdata;
	OPENORBIS_TextureData *openorbis_texture = (OPENORBIS_TextureData *) texture->driverdata;

	if (renderdata == 0)
		return;

	if(openorbis_texture == 0)
		return;

	DestroyTexture(openorbis_texture->texture);
	SDL_free(openorbis_texture);
	texture->driverdata = NULL;
}

static void
OPENORBIS_DestroyRenderer(SDL_Renderer *renderer){
	OPENORBIS_RenderData *data = (OPENORBIS_RenderData *) renderer->driverdata;
	if (data) {
		if (!data->initialized)
			return;

		// orbis2dFinish();

		data->initialized = SDL_FALSE;
		data->displayListAvail = SDL_FALSE;
		SDL_free(data);
	}
	SDL_free(renderer);
}

SDL_RenderDriver OPENORBIS_RenderDriver = {
	.CreateRenderer = OPENORBIS_CreateRenderer,
	.info = {
		.name = "ORBIS",
		.flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC,
		.num_texture_formats = 1,
		.texture_formats = {
		[0] = SDL_PIXELFORMAT_ARGB8888,
		},
		.max_texture_width = 1280,
		.max_texture_height = 720,
	 }
};
#endif /* SDL_VIDEO_RENDER_OPENORBIS */

/* vi: set ts=4 sw=4 expandtab: */