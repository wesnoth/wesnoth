/* $Id */
/*
   Copyright (C) 2003 by Karol Nowak <grzywacz@sul.uni.lodz.pl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include <SDL/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

/*
 * This file contains wrapper functions for SDL surface handling routines
 * which make it possible to easily monitor memory used by graphics. Note
 * that this is posix and gcc specific (I belive).
 *
 * Meant to be used as a LD_PRELOADed library.
 * Compile with:
 * 	gcc sdl_wrappers.c -shared -lSDL -fPIC -o wrappers.so
 *
 * Then:
 *  export LD_LIBRARY_PATH=. (or the directory where wrappers.so is located)
 *  export LD_PRELOAD=wrappers.so
 *
 *  and launch wesnoth. Observe memory used by surfaces on stderr.
 */

/*
 * Number of bytes used by SDL_Surfaces
 */
static unsigned int allocated;

/*
 * Handlers for dlopen()
 */
static void *handler;
static void *imghandler;

/*
 * Handles for SDL routines we're wrapping
 */
static SDL_Surface * (*Load)(const char *);
static void (*FreeSurface)(SDL_Surface *);
static SDL_Surface * (*ConvertSurface)(SDL_Surface *, SDL_PixelFormat *, Uint32);
static SDL_Surface * (*CreateRGBSurface)(Uint32, int, int, int, Uint32, Uint32, Uint32, Uint32);
static SDL_Surface * (*CreateRGBSurfaceFrom)(void *, int, int, int, int, Uint32, Uint32, Uint32, Uint32);
static SDL_Surface * (*DisplayFormat)(SDL_Surface *);
static SDL_Surface * (*DisplayFormatAlpha)(SDL_Surface *);

/*
 * Initialization and cleanup functions
 */
void init() __attribute__((constructor));
void fini() __attribute__((destructor));

#define STR(X) # X
#define WRAP(X) ((X) = dlsym(handler, STR(SDL_ ## X)))

void init() 
{
	fprintf(stderr, "Loading SDL wrappers...\n");

	handler = dlopen("libSDL.so", RTLD_NOW);
	imghandler = dlopen("libSDL_image.so", RTLD_NOW);

	if(!handler || !imghandler) {
		fprintf(stderr, "Wrapper error: %s", dlerror());
		exit(1);
	}

	Load = dlsym(imghandler, "IMG_Load");

	WRAP(FreeSurface);
	WRAP(ConvertSurface);
	WRAP(CreateRGBSurface);
	WRAP(CreateRGBSurfaceFrom);
	WRAP(DisplayFormat);
	WRAP(DisplayFormatAlpha);

	fprintf(stderr, "SDL wrappers loaded.\n");
}

void fini()
{
	if(handler) {
		dlclose(handler);
		handler = 0;
	}

	if(imghandler) {
		dlclose(imghandler);
		imghandler = 0;
	}
}

void print_allocated()
{
	fprintf(stderr, "A: %dkB\n", allocated / 1024);
}

static unsigned int surface_size(const SDL_Surface *s)
{
	print_allocated();
	if(s)
		return s->h * s->pitch;// FIXME
	else
		return 0;
}

/*
 * Actual wrappers for SDL surface manipulating functions start here
 */
void SDL_FreeSurface(SDL_Surface *s)
{
	if(s && s->refcount == 1)
		allocated -= surface_size(s);

	FreeSurface(s);
}

SDL_Surface *SDL_CreateRGBSurface(Uint32 flags, int width, int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
	SDL_Surface *tmp = CreateRGBSurface(flags, width, height, depth, Rmask, Gmask, Bmask, Amask);
	allocated += surface_size(tmp);
	return tmp;
}

/*
 * These, as it turns out, use SDL_CreateRGBSurface internally, so counting
 * memory allocated by "them" would cause it being counted twice...
 *
 * Commented out, at least for now.
 *
SDL_Surface *SDL_ConvertSurface(SDL_Surface *src, SDL_PixelFormat *fmt, Uint32 flags)
{
	SDL_Surface *tmp = ConvertSurface(src, fmt, flags);
	//allocated += surface_size(tmp);
	return tmp;
}

SDL_Surface *IMG_Load(const char *s)
{
	SDL_Surface *tmp = (SDL_Surface *) Load(s);

//	if(s)
//		allocated += surface_size(tmp);

	return tmp;
}

SDL_Surface *SDL_CreateRGBSurfaceFrom(void  *pixels,  int width, int height, int depth, int pitch, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
	SDL_Surface *tmp = CreateRGBSurfaceFrom(pixels, width, height, depth, pitch, Rmask, Gmask, Bmask, Amask);
	// allocated += surface_size(tmp);
	return tmp;
}

SDL_Surface *SDL_DisplayFormat(SDL_Surface *surface)
{
	SDL_Surface *tmp = DisplayFormat(surface);
	//allocated += surface_size(tmp);
	return tmp;
}

SDL_Surface *SDL_DisplayFormatAlpha(SDL_Surface *surface)
{
	SDL_Surface *tmp = DisplayFormatAlpha(surface);
//	allocated += surface_size(tmp);
	return tmp;
}
*/

/* vim: set ts=4 sw=4: */
