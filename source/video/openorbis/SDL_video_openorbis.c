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

#if SDL_VIDEO_DRIVER_OPENORBIS

/* SDL internals */
#include "../SDL_sysvideo.h"
#include "SDL_version.h"
#include "SDL_syswm.h"
#include "SDL_loadso.h"
#include "SDL_events.h"
#include "../../events/SDL_mouse_c.h"
#include "../../events/SDL_keyboard_c.h"

/* ORBIS declarations */
#include "SDL_video_openorbis.h"

SDL_bool OPENORBIS_initialized = SDL_FALSE;

#define FRAME_DEPTH 4
#define MEM_SIZE 0xC000000
#define NUMBER_OF_BUFFERS 2

static int
OPENORBIS_Available(void)
{
    if(OPENORBIS_initialized == SDL_FALSE) {
        return 1;
    }
    else {
        return 0;
    }
}

static void
OPENORBIS_Destroy(SDL_VideoDevice * device)
{
    if (device->driverdata != NULL) {
        device->driverdata = NULL;
    }
}

static SDL_VideoDevice *
OPENORBIS_Create()
{
    SDL_VideoDevice *device;
    SDL_VideoData *phdata;
    int status;

    /* Check if ORBIS could be initialized */
    status = OPENORBIS_Available();
    if (status == 0) {
        /* VITA could not be used */
        return NULL;
    }

    /* Initialize SDL_VideoDevice structure */
    device = (SDL_VideoDevice *) SDL_calloc(1, sizeof(SDL_VideoDevice));
    if (device == NULL) {
        SDL_OutOfMemory();
        return NULL;
    }

    /* Initialize internal ORBIS specific data */
    phdata = (SDL_VideoData *) SDL_calloc(1, sizeof(SDL_VideoData));
    if (phdata == NULL) {
        SDL_OutOfMemory();
        SDL_free(device);
        return NULL;
    }

    device->driverdata = phdata;

    /* Setup amount of available displays and current display */
    device->num_displays = 0;

    /* Set device free function */
    device->free = OPENORBIS_Destroy;

    /* Setup all functions which we can handle */
    device->VideoInit = OPENORBIS_VideoInit;
    device->VideoQuit = OPENORBIS_VideoQuit;
    device->GetDisplayModes = OPENORBIS_GetDisplayModes;
    device->SetDisplayMode = OPENORBIS_SetDisplayMode;
    device->CreateSDLWindow = OPENORBIS_CreateWindow;
    device->CreateSDLWindowFrom = OPENORBIS_CreateWindowFrom;
    device->SetWindowTitle = OPENORBIS_SetWindowTitle;
    device->SetWindowIcon = OPENORBIS_SetWindowIcon;
    device->SetWindowPosition = OPENORBIS_SetWindowPosition;
    device->SetWindowSize = OPENORBIS_SetWindowSize;
    device->ShowWindow = OPENORBIS_ShowWindow;
    device->HideWindow = OPENORBIS_HideWindow;
    device->RaiseWindow = OPENORBIS_RaiseWindow;
    device->MaximizeWindow = OPENORBIS_MaximizeWindow;
    device->MinimizeWindow = OPENORBIS_MinimizeWindow;
    device->RestoreWindow = OPENORBIS_RestoreWindow;
    device->SetWindowGrab = OPENORBIS_SetWindowGrab;
    device->DestroyWindow = OPENORBIS_DestroyWindow;
    device->GetWindowWMInfo = OPENORBIS_GetWindowWMInfo;
    device->HasScreenKeyboardSupport = OPENORBIS_HasScreenKeyboardSupport;
    device->ShowScreenKeyboard = OPENORBIS_ShowScreenKeyboard;
    device->HideScreenKeyboard = OPENORBIS_HideScreenKeyboard;
    device->IsScreenKeyboardShown = OPENORBIS_IsScreenKeyboardShown;

	device->PumpEvents = OPENORBIS_PumpEvents;

    return device;
}

VideoBootStrap OPENORBIS_bootstrap = {
    "OPENORBIS",
    "OPENORBIS Video Driver",
    OPENORBIS_Available,
    OPENORBIS_Create
};

/*****************************************************************************/
/* SDL Video and Display initialization/handling functions                   */
/*****************************************************************************/
int
OPENORBIS_VideoInit(_THIS)
{
    SDL_VideoDisplay display;
    SDL_DisplayMode current_mode;

    SDL_zero(current_mode);

    current_mode.w = 1280;
    current_mode.h = 720;

    current_mode.refresh_rate = 60;
    /* 32 bpp for default */
    current_mode.format = SDL_PIXELFORMAT_ARGB8888;

    current_mode.driverdata = NULL;

    SDL_zero(display);
    display.desktop_mode = current_mode;
    display.current_mode = current_mode;
    display.driverdata = NULL;

    SDL_AddVideoDisplay(&display);

    return 1;
}

void
OPENORBIS_VideoQuit(_THIS)
{
    
}

void
OPENORBIS_GetDisplayModes(_THIS, SDL_VideoDisplay * display)
{

}

int
OPENORBIS_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode)
{
    return 0;
}

int
OPENORBIS_CreateWindow(_THIS, SDL_Window * window)
{
    SDL_WindowData *wdata;

    /* Allocate window internal data */
    wdata = (SDL_WindowData *) SDL_calloc(1, sizeof(SDL_WindowData));
    if (wdata == NULL) {
        return SDL_OutOfMemory();
    }

    wdata->scene = Init(window->w, window->h, FRAME_DEPTH, MEM_SIZE, NUMBER_OF_BUFFERS);
    wdata->frame = 0;

    FrameBufferClear(wdata->scene);

    /* Setup driver data for this window */
    window->driverdata = wdata;

    // ORBIS can only have one window
    if (OPENORBIS_initialized == SDL_TRUE)
    {
        // Replace this with something else
        return SDL_OutOfMemory();
    }

    // fix input, we need to find a better way
    SDL_SetKeyboardFocus(window);

    // Set as initialized
    OPENORBIS_initialized = SDL_TRUE;

    /* Window has been successfully created */
    return 0;
}

int
OPENORBIS_CreateWindowFrom(_THIS, SDL_Window * window, const void *data)
{
    return -1;
}

void
OPENORBIS_SetWindowTitle(_THIS, SDL_Window * window)
{
}
void
OPENORBIS_SetWindowIcon(_THIS, SDL_Window * window, SDL_Surface * icon)
{
}
void
OPENORBIS_SetWindowPosition(_THIS, SDL_Window * window)
{
}
void
OPENORBIS_SetWindowSize(_THIS, SDL_Window * window)
{
}
void
OPENORBIS_ShowWindow(_THIS, SDL_Window * window)
{
}
void
OPENORBIS_HideWindow(_THIS, SDL_Window * window)
{
}
void
OPENORBIS_RaiseWindow(_THIS, SDL_Window * window)
{
}
void
OPENORBIS_MaximizeWindow(_THIS, SDL_Window * window)
{
}
void
OPENORBIS_MinimizeWindow(_THIS, SDL_Window * window)
{
}
void
OPENORBIS_RestoreWindow(_THIS, SDL_Window * window)
{
}
void
OPENORBIS_SetWindowGrab(_THIS, SDL_Window * window, SDL_bool grabbed)
{
}
void
OPENORBIS_DestroyWindow(_THIS, SDL_Window * window)
{
}

/*****************************************************************************/
/* SDL Window Manager function                                               */
/*****************************************************************************/
SDL_bool
OPENORBIS_GetWindowWMInfo(_THIS, SDL_Window * window, struct SDL_SysWMinfo *info)
{
    if (info->version.major <= SDL_MAJOR_VERSION) {
        return SDL_TRUE;
    } else {
        SDL_SetError("application not compiled with SDL %d.%d\n",
                     SDL_MAJOR_VERSION, SDL_MINOR_VERSION);
        return SDL_FALSE;
    }

    /* Failed to get window manager information */
    return SDL_FALSE;
}


/* TO Write Me */
SDL_bool OPENORBIS_HasScreenKeyboardSupport(_THIS)
{
    return SDL_FALSE;
}
void OPENORBIS_ShowScreenKeyboard(_THIS, SDL_Window *window)
{
}
void OPENORBIS_HideScreenKeyboard(_THIS, SDL_Window *window)
{
}
SDL_bool OPENORBIS_IsScreenKeyboardShown(_THIS, SDL_Window *window)
{
    return SDL_FALSE;
}

void OPENORBIS_PumpEvents(_THIS)
{
    
}

#endif /* SDL_VIDEO_DRIVER_OPENORBIS */

/* vi: set ts=4 sw=4 expandtab: */
