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
#include <stdio.h>
#include <iostream>

#include "mouse.hpp"
#include "preferences.hpp"
#include "video.hpp"

#define TEST_VIDEO_ON 0

#if (TEST_VIDEO_ON==1)

#include <stdlib.h>


//test program takes three args - x-res y-res colour-depth
int main( int argc, char** argv )
{
	if( argc != 4 ) {
		printf( "usage: %s x-res y-res bitperpixel\n", argv[0] );
		return 1;
	}
	SDL_Init(SDL_INIT_VIDEO);
	CVideo video;

	printf( "args: %s, %s, %s\n", argv[1], argv[2], argv[3] );

	printf( "(%d,%d,%d)\n", strtoul(argv[1],0,10), strtoul(argv[2],0,10),
	                        strtoul(argv[3],0,10) );

	if( video.setMode( strtoul(argv[1],0,10), strtoul(argv[2],0,10),
	                        strtoul(argv[3],0,10), FULL_SCREEN ) ) {
		printf( "video mode possible\n" );
	} else  printf( "video mode NOT possible\n" );
	printf( "%d, %d, %d\n", video.getx(), video.gety(),
	                        video.getBitsPerPixel() );

	for( int s = 0; s < 50; s++ ) {
		video.lock();
		for( int i = 0; i < video.getx(); i++ )
				video.setPixel( i, 90, 255, 0, 0 );
		if( s%10==0)
			printf( "frame %d\n", s );
		video.unlock();
		video.update( 0, 90, video.getx(), 1 );
	}
	return 0;
}

#endif

namespace {
	bool fullScreen = false;

	unsigned int get_flags(unsigned int flags)
	{
		//SDL under Windows doesn't seem to like hardware surfaces for
		//some reason.
#ifndef _WIN32
		flags |= SDL_HWSURFACE | SDL_DOUBLEBUF;
#endif
		if((flags&SDL_FULLSCREEN) == 0)
			flags |= SDL_RESIZABLE;

		return flags;
	}
}

CVideo::CVideo() : frameBuffer(NULL)
{
	const int res =
	       SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ); // | SDL_INIT_NOPARACHUTE);

	if(res < 0) {
		std::cerr << "Could not initialize SDL: " << SDL_GetError() << "\n";
		throw CVideo::error();
	}
}

CVideo::CVideo( int x, int y, int bits_per_pixel, int flags)
		 : frameBuffer(NULL)
{
	const int res = SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE);
	if(res < 0) {
		throw error();
	}

	setMode( x, y, bits_per_pixel, flags );
}

CVideo::~CVideo()
{
	SDL_Quit();
}

int CVideo::modePossible( int x, int y, int bits_per_pixel, int flags )
{
	return SDL_VideoModeOK( x, y, bits_per_pixel, get_flags(flags) );
}

int CVideo::setMode( int x, int y, int bits_per_pixel, int flags )
{
	flags = get_flags(flags);
	const int res = SDL_VideoModeOK( x, y, bits_per_pixel, flags );

	if( res == 0 )
		return 0;

	fullScreen = (flags & FULL_SCREEN) != 0;
	frameBuffer = SDL_SetVideoMode( x, y, bits_per_pixel, flags );

	if( frameBuffer != NULL ) {
		return bits_per_pixel;
	} else	return 0;
}

int CVideo::getx() const
{
	return frameBuffer->w;
}

int CVideo::gety() const
{
	return frameBuffer->h;
}

int CVideo::getBitsPerPixel()
{
	return frameBuffer->format->BitsPerPixel;
}

int CVideo::getBytesPerPixel()
{
	return frameBuffer->format->BytesPerPixel;
}

int CVideo::getRedMask()
{
	return frameBuffer->format->Rmask;
}

int CVideo::getGreenMask()
{
	return frameBuffer->format->Gmask;
}

int CVideo::getBlueMask()
{
	return frameBuffer->format->Bmask;
}

void CVideo::flip()
{
	::SDL_Flip(frameBuffer);
}

void CVideo::lock()
{
	if( SDL_MUSTLOCK(frameBuffer) )
		SDL_LockSurface( frameBuffer );
}

void CVideo::unlock()
{
	if( SDL_MUSTLOCK(frameBuffer) )
		SDL_UnlockSurface( frameBuffer );
}

int CVideo::mustLock()
{
	return SDL_MUSTLOCK(frameBuffer);
}

SDL_Surface* CVideo::getSurface( void )
{
	return frameBuffer;
}

bool CVideo::isFullScreen() const { return fullScreen; }
