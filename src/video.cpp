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
}

CVideo::CVideo(const char* text) : frameBuffer(NULL), backBuffer(NULL)
{
	const int res =
	       SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE);

	if(res < 0) {
		std::cerr << "Could not initialize SDL: " << SDL_GetError() << "\n";
		throw CVideo::error();
	}

	for(int i = 0; i != sizeof(text_); ++i) {
		text_[i] = text[i];
	}
}

CVideo::CVideo( int x, int y, int bits_per_pixel, int flags, const char* text )
		 : frameBuffer(NULL), backBuffer(NULL)
{
	const int res = SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE);
	if(res < 0) {
		throw error();
	}

	setMode( x, y, bits_per_pixel, flags );

	for(int i = 0; i != sizeof(text_); ++i) {
		text_[i] = text[i];
	}
}

CVideo::~CVideo()
{
	SDL_FreeSurface( backBuffer );
	SDL_Quit();
}

int CVideo::modePossible( int x, int y, int bits_per_pixel, int flags )
{
	return SDL_VideoModeOK( x, y, bits_per_pixel, flags );
}

int CVideo::setMode( int x, int y, int bits_per_pixel, int flags )
{
	const int res = SDL_VideoModeOK( x, y, bits_per_pixel, flags );

	if( res == 0 )
		return 0;

	fullScreen = (flags & FULL_SCREEN) != 0;
	frameBuffer = SDL_SetVideoMode( x, y, bits_per_pixel, flags );

	if( frameBuffer != NULL ) {
		if(backBuffer != NULL)
			SDL_FreeSurface(backBuffer);

		backBuffer = SDL_ConvertSurface( frameBuffer,
		                                 frameBuffer->format, 0 );
		if( backBuffer == NULL )
			fprintf( stderr, "out of memory\n" );
		return bits_per_pixel;
	} else	return 0;
}

int CVideo::getx() const
{
	return backBuffer->w;
}

int CVideo::gety() const
{
	return backBuffer->h;
}

int CVideo::getBitsPerPixel()
{
	return backBuffer->format->BitsPerPixel;
}

int CVideo::getBytesPerPixel()
{
	return backBuffer->format->BytesPerPixel;
}

int CVideo::getRedMask()
{
	return backBuffer->format->Rmask;
}

int CVideo::getGreenMask()
{
	return backBuffer->format->Gmask;
}

int CVideo::getBlueMask()
{
	return backBuffer->format->Bmask;
}

void* CVideo::getAddress()
{
	return backBuffer->pixels;
}

void CVideo::lock()
{
	if( SDL_MUSTLOCK(backBuffer) )
		SDL_LockSurface( backBuffer );
}

void CVideo::unlock()
{
	if( SDL_MUSTLOCK(backBuffer) )
		SDL_UnlockSurface( backBuffer );
}

int CVideo::mustLock()
{
	return SDL_MUSTLOCK(backBuffer);
}

void CVideo::setPixel( int x, int y, int r, int g, int b )
{
	int pixel = ((r<<(backBuffer->format->Rshift-backBuffer->format->Rloss))
	              & backBuffer->format->Rmask)+
	            ((g<<(backBuffer->format->Gshift-backBuffer->format->Gloss))
		      & backBuffer->format->Gmask)+
		    ((b>>(backBuffer->format->Bloss-backBuffer->format->Bshift))
		      & backBuffer->format->Bmask);

	setPixel( x, y, pixel );
}

int CVideo::convertColour(int r, int g, int b)
{
	return ((r<<(backBuffer->format->Rshift-backBuffer->format->Rloss))
	              & backBuffer->format->Rmask)+
	            ((g<<(backBuffer->format->Gshift-backBuffer->format->Gloss))
		      & backBuffer->format->Gmask)+
		    ((b>>(backBuffer->format->Bloss-backBuffer->format->Bshift))
		      & backBuffer->format->Bmask);
}

void CVideo::setPixel( int x, int y, int p )
{
	static int pixel;
	static char* p1 = ((char*)&pixel);
	static char* p2 = ((char*)&pixel)+1;
	static char* p3 = ((char*)&pixel)+2;
	static short* sp = ((short*)&pixel);
	pixel = p;

	if( x < 0 || x >= backBuffer->w || y < 0 || y >= backBuffer->h )
		return;

	switch( backBuffer->format->BytesPerPixel ) {
		case 1:
			*((char*)backBuffer->pixels+y*backBuffer->w+x) = *p1;
			break;
		case 2:
			*((short*)backBuffer->pixels+y*backBuffer->w+x) = *sp;
			break;
		case 3:
			*((char*)backBuffer->pixels+y*backBuffer->w*3+x*3)
			                    = *p1;
			*((char*)backBuffer->pixels+y*backBuffer->w*3+x*3+1)
			                    = *p2;
			*((char*)backBuffer->pixels+y*backBuffer->w*3+x*3+2)
			                    = *p3;
			break;
		case 4:
			*((int*)backBuffer->pixels+y*backBuffer->w+x) = pixel;
			break;
		default:
			fprintf( stderr, "Unknown colour depth\n" );
	}
}

void CVideo::update( int x, int y, int w, int h )
{
	if( w == 0 || h == 0 )
		return;

	if( x < 0 ) {
		w += x;
		x = 0;
	}

	if( y < 0 ) {
		h += y;
		y = 0;
	}

	if( x+w > frameBuffer->w )
		w = frameBuffer->w - x;

	if( y+h > frameBuffer->h )
		h = frameBuffer->h - y;

	SRectangle rect = {x,y,w,h};
	SDL_BlitSurface( backBuffer, &rect, frameBuffer, &rect );
	SDL_UpdateRect( frameBuffer, x, y, w, h );
}

void CVideo::update( SRectangle* rect )
{
	SDL_BlitSurface( backBuffer, rect, frameBuffer, rect );
	SDL_UpdateRect( frameBuffer, rect->x, rect->y, rect->w, rect->h );
}

SDL_Surface* CVideo::getSurface( void )
{
	return backBuffer;
}

void CVideo::drawChar(int x, int y, int pixel, int bg, char c, int sz)
{
	const char* const data = text_ + c*8;
	for(int i = 0; i != 8*sz; ++i) {
		if(y+i >= backBuffer->h)
			return;

		for(int j = 0; j != 8*sz; ++j) {
			if(x+j >= backBuffer->w)
				break;

			if(data[i/sz] & (128 >> (j/sz))) {
				setPixel(x+j,y+i,pixel);
			} else {
				if(bg != pixel)
					setPixel(x+j,y+i,bg);
			}
		}
	}
}

int CVideo::drawText(int x, int y, int pixel, int bg, const char* text, int sz)
{
	const int good_colour = 0x0F00;
	const int bad_colour = 0xF000;
	int colour = pixel;
	const int startx = x;
	const int starty = y;
	while(*text) {
		if(*text == '@') {
			colour = good_colour;
		} else if(*text == '#') {
			colour = bad_colour;
		} else if(*text == '\n') {
			colour = pixel;
			y += 8*sz;
			x = startx;
		} else {
			drawChar(x,y,colour,bg == pixel ? colour : bg,*text,sz);
			x += 8*sz;
		}
		++text;
	}

	y += 8*sz;

	return y - starty;
}

bool CVideo::isFullScreen() const { return fullScreen; }
