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
	void setPixel( int x, int y, int r, int g, int b );
	void setPixel( int x, int y, int pixel );
	int convertColour(int r, int g, int b);
	void update( int x, int y, int w, int h );
	void update( SDL_Rect* area );

	SDL_Surface* getSurface( void );

	int drawText(int x, int y, int pixel, int bg, const char* text,int size=1);

	bool isFullScreen() const;

	struct error {};

     private:

	void drawChar(int x, int y, int pixel, int bg, char c, int size=1);

	SDL_Surface* frameBuffer;
	SDL_Surface* backBuffer;
	char text_[256*8];
};

void allow_resizing(bool);

struct resize_lock {
	resize_lock();
	~resize_lock();
};

void pump_events();

#endif
