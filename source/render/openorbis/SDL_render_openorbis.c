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

#if SDL_VIDEO_RENDER_OPENORBIS

#include "SDL_render_openorbis.h"
#include <orbis/libkernel.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>

static int
GetScaleQuality(void){
	const char *hint = SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY);

	if (!hint || *hint == '0' || SDL_strcasecmp(hint, "nearest") == 0) {
		return ORBIS2D_MODE_TILE; // good for tile-map
	} else {
		return ORBIS2D_MODE_LINEAR; // good for scaling
	}
}

void
StartDrawing(SDL_Renderer *renderer){
	ORBIS_RenderData *data = (OPENORBIS_RenderData *) renderer->driverdata;
	if(data->displayListAvail)
		return;

	orbis2dStartDrawing();

	data->displayListAvail = SDL_TRUE;
}

SDL_Renderer *
OPENORBIS_CreateRenderer(SDL_Window *window, Uint32 flags){

	SDL_Renderer *renderer;
	ORBIS_RenderData *data;

	renderer = (SDL_Renderer *) SDL_calloc(1, sizeof(*renderer));
	if (!renderer) {
		SDL_OutOfMemory();
		return NULL;
	}

	data = (OPENORBIS_RenderData *) SDL_calloc(1, sizeof(*data));
	if (!data) {
		ORBIS_DestroyRenderer(renderer);
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
	if(orbis2dGetStatus()==1) {
			
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
	ORBIS_TextureData* openorbis_texture = (OPENORBIS_TextureData *) SDL_calloc(1, sizeof(*orbis_texture));

	if(!orbis_texture)
		return -1;

	orbis_texture->tex = orbis2dCreateEmptyTexture(texture->w, texture->h);

	if(!orbis_texture->tex)
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
	//int scaleMode = GetScaleQuality();
	orbis_texture->w = openorbis_texture->tex->width;
	orbis_texture->h = openorbis_texture->tex->height;
	orbis_texture->pitch = openorbis_texture->w *SDL_BYTESPERPIXEL(texture->format);

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

	ORBIS_LockTexture(renderer, texture, rect, (void **)&dst, &dpitch);
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
	ORBIS_TextureData *orbis_texture = (OPENORBIS_TextureData *) texture->driverdata;

	*pixels =
		(void *) ((Uint8 *) orbis2dTextureGetDataPointer(openorbis_texture->tex)
			+ (rect->y * openorbis_texture->w + rect->x) * SDL_BYTESPERPIXEL(texture->format));
	*pitch = openorbis_texture->pitch;
	return 0;
}

static void
OPENORBIS_UnlockTexture(SDL_Renderer *renderer, SDL_Texture *texture){
	// no needs to update texture data on ps vita. VITA_LockTexture
	// already return a pointer to the vita2d texture pixels buffer.
	// This really improve framerate when using lock/unlock.
	
	/*
	VITA_TextureData *vita_texture = (VITA_TextureData *) texture->driverdata;
	SDL_Rect rect;

	// We do whole texture updates, at least for now
	rect.x = 0;
	rect.y = 0;
	rect.w = texture->w;
	rect.h = texture->h;
	VITA_UpdateTexture(renderer, texture, &rect, vita_texture->data, vita_texture->pitch);
	*/
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
	/*VITA_RenderData *data = (VITA_RenderData *) renderer->driverdata;
	if (blendMode != data-> currentBlendMode) {
		switch (blendMode) {
		case SDL_BLENDMODE_NONE:
				sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
				sceGuDisable(GU_BLEND);
			break;
		case SDL_BLENDMODE_BLEND:
				sceGuTexFunc(GU_TFX_MODULATE , GU_TCC_RGBA);
				sceGuEnable(GU_BLEND);
				sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0 );
			break;
		case SDL_BLENDMODE_ADD:
				sceGuTexFunc(GU_TFX_MODULATE , GU_TCC_RGBA);
				sceGuEnable(GU_BLEND);
				sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_FIX, 0, 0x00FFFFFF );
			break;
		case SDL_BLENDMODE_MOD:
				sceGuTexFunc(GU_TFX_MODULATE , GU_TCC_RGBA);
				sceGuEnable(GU_BLEND);
				sceGuBlendFunc( GU_ADD, GU_FIX, GU_SRC_COLOR, 0, 0);
			break;
		}
		data->currentBlendMode = blendMode;
	}*/
}



static int
OPENORBIS_RenderClear(SDL_Renderer *renderer){
	/* start list */
	StartDrawing(renderer);
	uint32_t color = renderer->a << 24 | renderer->r << 16 | renderer->g << 8 | renderer->b;
	if(orbis2dGetStatus()) {
		orbis2dSetBgColor(color);
	}
	orbis2dClearBuffer(1);
	
	return 0;
}

static int
OPENORBIS_RenderDrawPoints(SDL_Renderer *renderer, const SDL_FPoint *points,
					  int count){
	uint32_t color = renderer->a << 24 | renderer->r << 16 | renderer->g << 8 | renderer->b;
	int i;
	StartDrawing(renderer);

	for (i = 0; i < count; ++i) {
		orbis2dDrawPixelColor(points[i].x, points[i].y, color);
	}

	return 0;
}

static int
OPENORBIS_RenderDrawLines(SDL_Renderer *renderer, const SDL_FPoint *points,
					 int count){
	uint32_t color = renderer->a << 24 | renderer->r << 16 | renderer->g << 8 | renderer->b;
	int i;
	StartDrawing(renderer);

	for (i = 0; i < count; ++i) {
	if (i < count -1) {
		orbis2dDrawLineColor(points[i].x, points[i].y, points[i+1].x, points[i+1].y, color);
	}
	}

	return 0;
}

static int
OPENORBIS_RenderFillRects(SDL_Renderer *renderer, const SDL_FRect *rects,
					 int count){
	uint32_t color = renderer->a << 24 | renderer->r << 16 | renderer->g << 8 | renderer->b;
	int i;
	StartDrawing(renderer);

	for (i = 0; i < count; ++i) {
		const SDL_FRect *rect = &rects[i];
		orbis2dDrawRectColor(rect->x, rect->w, rect->y, rect->h, color);
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
	ORBIS_TextureData *orbis_texture = (OPENORBIS_TextureData *) texture->driverdata;
	//float scaleX = dstrect->w > srcrect->w ? (float)(dstrect->w/srcrect->w) : 1;
	//float scaleY = dstrect->h > srcrect->h ? (float)(dstrect->h/srcrect->h) : 1;
	float scaleX = dstrect->w == srcrect->w ? 1 : (float)(dstrect->w/srcrect->w);
	float scaleY = dstrect->h == srcrect->h ? 1 : (float)(dstrect->h/srcrect->h);

	StartDrawing(renderer);

	ORBIS_SetBlendMode(renderer, renderer->blendMode);

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
	/*float x, y, width, height;
	float u0, v0, u1, v1;
	unsigned char alpha;
	float centerx, centery;

	x = dstrect->x;
	y = dstrect->y;
	width = dstrect->w;
	height = dstrect->h;

	u0 = srcrect->x;
	v0 = srcrect->y;
	u1 = srcrect->x + srcrect->w;
	v1 = srcrect->y + srcrect->h;

	centerx = center->x;
	centery = center->y;

	alpha = texture->a;

	StartDrawing(renderer);
	TextureActivate(texture);
	VITA_SetBlendMode(renderer, renderer->blendMode);

	if(alpha != 255)
	{
		sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
		sceGuColor(GU_RGBA(255, 255, 255, alpha));
	}else{
		sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
		sceGuColor(0xFFFFFFFF);
	}

	x += centerx;
	y += centery;

	float c, s;

	MathSincos(degToRad(angle), &s, &c);

	width  -= centerx;
	height -= centery;


	float cw = c*width;
	float sw = s*width;
	float ch = c*height;
	float sh = s*height;

	VertTV* vertices = (VertTV*)sceGuGetMemory(sizeof(VertTV)<<2);

	vertices[0].u = u0;
	vertices[0].v = v0;
	vertices[0].x = x - cw + sh;
	vertices[0].y = y - sw - ch;
	vertices[0].z = 0;

	vertices[1].u = u0;
	vertices[1].v = v1;
	vertices[1].x = x - cw - sh;
	vertices[1].y = y - sw + ch;
	vertices[1].z = 0;

	vertices[2].u = u1;
	vertices[2].v = v1;
	vertices[2].x = x + cw - sh;
	vertices[2].y = y + sw + ch;
	vertices[2].z = 0;

	vertices[3].u = u1;
	vertices[3].v = v0;
	vertices[3].x = x + cw + sh;
	vertices[3].y = y + sw - ch;
	vertices[3].z = 0;

	if (flip & SDL_FLIP_HORIZONTAL) {
				Swap(&vertices[0].v, &vertices[2].v);
				Swap(&vertices[1].v, &vertices[3].v);
	}
	if (flip & SDL_FLIP_VERTICAL) {
				Swap(&vertices[0].u, &vertices[2].u);
				Swap(&vertices[1].u, &vertices[3].u);
	}

	sceGuDrawArray(GU_TRIANGLE_FAN, GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 4, 0, vertices);

	if(alpha != 255)
		sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	return 0;*/
	return 1;
}

static void
OPENORBIS_RenderPresent(SDL_Renderer *renderer){
	ORBIS_RenderData *data = (OPENORBIS_RenderData *) renderer->driverdata;
	if(!data->displayListAvail)
		return;

	orbis2dFinishDrawing(flipArg);
	orbis2dSwapBuffers();
	flipArg++;

	data->displayListAvail = SDL_FALSE;
}

static void
OPENORBIS_DestroyTexture(SDL_Renderer *renderer, SDL_Texture *texture){
	ORBIS_RenderData *renderdata = (OPENORBIS_RenderData *) renderer->driverdata;
	ORBIS_TextureData *orbis_texture = (OPENORBIS_TextureData *) texture->driverdata;

	if (renderdata == 0)
		return;

	if(openorbis_texture == 0)
		return;

	orbis2dDestroyTexture(openorbis_texture->tex);
	SDL_free(openorbis_texture);
	texture->driverdata = NULL;
}

static void
OPENORBIS_DestroyRenderer(SDL_Renderer *renderer){
	ORBIS_RenderData *data = (OPENORBIS_RenderData *) renderer->driverdata;
	if (data) {
		if (!data->initialized)
			return;

	//	orbis2dFinish();

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

