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

#ifndef _SDL_render_orbis_h
#define _SDL_render_orbis_h

#include "../../SDL_internal.h"

#include "SDL_hints.h"
#include "../SDL_sysrender.h"

typedef struct{
	void		*frontbuffer;
	void		*backbuffer;
	SDL_bool	initialized;
	SDL_bool	displayListAvail;
	unsigned int	psm;
	unsigned int	bpp;
	SDL_bool	vsync;
	unsigned int	currentColor;
	int		 currentBlendMode;
} OPENORBIS_RenderData;


typedef struct{
	Orbis2dTexture	*tex;
	unsigned int	pitch;
	unsigned int	w;
	unsigned int	h;
} OPENORBIS_TextureData;

extern int64_t flipArg;

/* Orbis renderer implementation, based on the orbis2d lib  */
extern int SDL_RecreateWindow(SDL_Window *window, Uint32 flags);

static SDL_Renderer *ORBIS_CreateRenderer(SDL_Window *window, Uint32 flags);
static void OPENORBIS_WindowEvent(SDL_Renderer *renderer, const SDL_WindowEvent *event);
static int OPENORBIS_CreateTexture(SDL_Renderer *renderer, SDL_Texture *texture);
static int OPENORBIS_UpdateTexture(SDL_Renderer *renderer, SDL_Texture *texture,
	const SDL_Rect *rect, const void *pixels, int pitch);
static int OPENORBIS_LockTexture(SDL_Renderer *renderer, SDL_Texture *texture,
	const SDL_Rect *rect, void **pixels, int *pitch);
static void OPENORBIS_UnlockTexture(SDL_Renderer *renderer,
	 SDL_Texture *texture);
static int OPENORBIS_SetRenderTarget(SDL_Renderer *renderer,
		 SDL_Texture *texture);
static int OPENORBIS_UpdateViewport(SDL_Renderer *renderer);
static int OPENORBIS_RenderClear(SDL_Renderer *renderer);
static int OPENORBIS_RenderDrawPoints(SDL_Renderer *renderer,
		const SDL_FPoint *points, int count);
static int OPENORBIS_RenderDrawLines(SDL_Renderer *renderer,
		const SDL_FPoint *points, int count);
static int OPENORBIS_RenderFillRects(SDL_Renderer *renderer,
		const SDL_FRect *rects, int count);
static int OPENORBIS_RenderCopy(SDL_Renderer *renderer, SDL_Texture *texture,
	const SDL_Rect *srcrect, const SDL_FRect *dstrect);
static int OPENORBIS_RenderReadPixels(SDL_Renderer *renderer, const SDL_Rect *rect,
	Uint32 pixel_format, void *pixels, int pitch);
static int OPENORBIS_RenderCopyEx(SDL_Renderer *renderer, SDL_Texture *texture,
	const SDL_Rect *srcrect, const SDL_FRect *dstrect,
	const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip);
static void OPENORBIS_RenderPresent(SDL_Renderer *renderer);
static void OPENORBIS_DestroyTexture(SDL_Renderer *renderer, SDL_Texture *texture);
static void OPENORBIS_DestroyRenderer(SDL_Renderer *renderer);

#endif