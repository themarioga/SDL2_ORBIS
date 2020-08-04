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

#ifndef _SDL_video_openorbis_h
#define _SDL_video_openorbis_h

#include "../../SDL_internal.h"
#include "../SDL_sysvideo.h"

#include "graphics.h"

typedef struct SDL_VideoData
{
   
} SDL_VideoData;


typedef struct SDL_DisplayData
{

} SDL_DisplayData;


typedef struct SDL_WindowData
{
   Scene2D *scene;
   int64_t frame;
} SDL_WindowData;

extern SDL_bool OPENORBIS_initialized;

/****************************************************************************/
/* SDL_VideoDevice functions declaration                                    */
/****************************************************************************/

/* Display and window functions */
int OPENORBIS_VideoInit(_THIS);
void OPENORBIS_VideoQuit(_THIS);
void OPENORBIS_GetDisplayModes(_THIS, SDL_VideoDisplay * display);
int OPENORBIS_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode);
int OPENORBIS_CreateWindow(_THIS, SDL_Window * window);
int OPENORBIS_CreateWindowFrom(_THIS, SDL_Window * window, const void *data);
void OPENORBIS_SetWindowTitle(_THIS, SDL_Window * window);
void OPENORBIS_SetWindowIcon(_THIS, SDL_Window * window, SDL_Surface * icon);
void OPENORBIS_SetWindowPosition(_THIS, SDL_Window * window);
void OPENORBIS_SetWindowSize(_THIS, SDL_Window * window);
void OPENORBIS_ShowWindow(_THIS, SDL_Window * window);
void OPENORBIS_HideWindow(_THIS, SDL_Window * window);
void OPENORBIS_RaiseWindow(_THIS, SDL_Window * window);
void OPENORBIS_MaximizeWindow(_THIS, SDL_Window * window);
void OPENORBIS_MinimizeWindow(_THIS, SDL_Window * window);
void OPENORBIS_RestoreWindow(_THIS, SDL_Window * window);
void OPENORBIS_SetWindowGrab(_THIS, SDL_Window * window, SDL_bool grabbed);
void OPENORBIS_DestroyWindow(_THIS, SDL_Window * window);

/* Window manager function */
SDL_bool OPENORBIS_GetWindowWMInfo(_THIS, SDL_Window * window,
                             struct SDL_SysWMinfo *info);


/* ORBIS on screen keyboard */
SDL_bool OPENORBIS_HasScreenKeyboardSupport(_THIS);
void OPENORBIS_ShowScreenKeyboard(_THIS, SDL_Window *window);
void OPENORBIS_HideScreenKeyboard(_THIS, SDL_Window *window);
SDL_bool OPENORBIS_IsScreenKeyboardShown(_THIS, SDL_Window *window);

void OPENORBIS_PumpEvents(_THIS);

#endif /* _SDL_video_openorbis_h */

/* vi: set ts=4 sw=4 expandtab: */
