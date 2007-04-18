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
static void (*FreeSurface)(SDL_Surface *);
static SDL_Surface * (*CreateRGBSurface)(Uint32, int, int, int, Uint32, Uint32, Uint32, Uint32);

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

	WRAP(FreeSurface);
	WRAP(CreateRGBSurface);

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
 * Actual wrappers for SDL functions
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

/* vim: set ts=4 sw=4: */
