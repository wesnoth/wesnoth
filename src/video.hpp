/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef VIDEO_HPP_INCLUDED
#define VIDEO_HPP_INCLUDED

#include "SDL.h"

//possible flags when setting video modes
#define FULL_SCREEN SDL_FULLSCREEN
#define VIDEO_MEMORY SDL_HWSURFACE
#define SYSTEM_MEMORY SDL_SWSURFACE

class CVideo {
     public:
	CVideo(const char* text);
	CVideo( int x, int y, int bits_per_pixel, int flags, const char* text );
	~CVideo();

	int modePossible( int x, int y, int bits_per_pixel, int flags );
	int setMode( int x, int y, int bits_per_pixel, int flags );

	//functions to get the dimensions of the current video-mode
	int getx() const;
	int gety() const;
	int getBitsPerPixel();
	int getBytesPerPixel();
	int getRedMask();
	int getGreenMask();
	int getBlueMask();

	//functions to access the screen
	void* getAddress();
	void lock();
	void unlock();
	int mustLock();

	void flip();

	SDL_Surface* getSurface( void );

	bool isFullScreen() const;

	struct error {};

     private:

	SDL_Surface* frameBuffer;
	char text_[256*8];
};

void allow_resizing(bool);

struct resize_lock {
	resize_lock();
	~resize_lock();
};

void pump_events();

#endif
