/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include <GL/gl.h>
#include <GL/glu.h>

#include <stdio.h>
#include <iostream>
#include <vector>

#include "font.hpp"
#include "gl_draw.hpp"
#include "image.hpp"
#include "log.hpp"
#include "video.hpp"

#define LOG_DP LOG_STREAM(info, display)
#define ERR_DP LOG_STREAM(err, display)

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
#if !(defined(_WIN32) || defined(__APPLE__) || defined(__AMIGAOS4__))
		flags |= SDL_HWSURFACE;
#endif
	if((flags&SDL_FULLSCREEN) == 0)
		flags |= SDL_RESIZABLE;

	return flags;
}

std::vector<SDL_Rect> update_rects;
bool update_all = false;

bool rect_contains(const SDL_Rect& a, const SDL_Rect& b) {
	return a.x <= b.x && a.y <= b.y && a.x+a.w >= b.x+b.w && a.y+a.h >= b.y+b.h;
}

void clear_updates()
{
	update_all = false;
	update_rects.clear();
}

surface frameBuffer = NULL;

}

bool non_interactive()
{
	return SDL_GetVideoSurface() == NULL;
}

surface display_format_alpha(surface surf)
{
	if(SDL_GetVideoSurface() != NULL)
		return SDL_DisplayFormatAlpha(surf);
	else if(frameBuffer != NULL)
		return SDL_ConvertSurface(surf,frameBuffer->format,0);
	else
		return NULL;
}

surface get_video_surface()
{
	return frameBuffer;
}

SDL_Rect screen_area()
{
	const SDL_Rect res = {0,0,frameBuffer->w,frameBuffer->h};
	return res;
}

void update_rect(size_t x, size_t y, size_t w, size_t h)
{
}

void update_rect(const SDL_Rect& rect_value)
{
}

void update_whole_screen()
{
	update_all = true;
}
CVideo::CVideo() : bpp(0), fake_screen(false), help_string_(0), updatesLocked_(0)
{
	const int res = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);

	if(res < 0) {
		ERR_DP << "Could not initialize SDL: " << SDL_GetError() << "\n";
		throw CVideo::error();
	}
}

CVideo::CVideo( int x, int y, int bits_per_pixel, int flags)
		 : bpp(0), fake_screen(false), help_string_(0), updatesLocked_(0)
{
	const int res = SDL_Init( SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);
	if(res < 0) {
		ERR_DP << "Could not initialize SDL: " << SDL_GetError() << "\n";
		throw CVideo::error();
	}

	const int mode_res = setMode( x, y, bits_per_pixel, flags );
	if (mode_res == 0) {
		ERR_DP << "Could not set Video Mode\n";
		throw CVideo::error();
	}
}

CVideo::~CVideo()
{
	LOG_DP << "calling SDL_Quit()\n";
	SDL_Quit();
	LOG_DP << "called SDL_Quit()\n";
}

void CVideo::blit_surface(int x, int y, surface surf, SDL_Rect* srcrect, SDL_Rect* clip_rect)
{
	gl::draw_surface(surf.get(),x,y);
}

void CVideo::make_fake()
{
	fake_screen = true;
	frameBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE,1,1,24,0xFF0000,0xFF00,0xFF,0);
	image::set_pixel_format(frameBuffer->format);
}

int CVideo::modePossible( int x, int y, int bits_per_pixel, int flags )
{
	return SDL_VideoModeOK( x, y, bits_per_pixel, get_flags(flags) );
}

int CVideo::setMode( int x, int y, int bits_per_pixel, int flags )
{
	update_rects.clear();
	mode_changed_ = true;

	flags = get_flags(flags);
	const int res = SDL_VideoModeOK( x, y, bits_per_pixel, flags );

	if( res == 0 )
		return 0;

	fullScreen = (flags & FULL_SCREEN) != 0;
	frameBuffer = SDL_SetVideoMode( x, y, bits_per_pixel, flags|SDL_OPENGL );

	glFrontFace(GL_CW);
	//glEnable(GL_CULL_FACE);
	//glEnable(GL_LIGHT0);
	//glLightfv(GL_LIGHT0,GL_AMBIENT,ambient_light);
	//glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuse_light);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glAlphaFunc(GL_GREATER,0.02);

	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

	if( frameBuffer != NULL ) {
		image::set_pixel_format(frameBuffer->format);
		return bits_per_pixel;
	} else	return 0;
}

bool CVideo::modeChanged()
{
	bool ret = mode_changed_;
	mode_changed_ = false;
	return ret;
}

int CVideo::setGamma(float gamma)
{
	SDL_SetGamma(gamma, gamma, gamma);

	return 0;
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
	if(fake_screen)
		return;

	gl::flip();
}

void CVideo::lock_updates(bool value)
{
	if(value == true)
		++updatesLocked_;
	else
		--updatesLocked_;
}

bool CVideo::update_locked() const
{
	return updatesLocked_ > 0;
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

surface CVideo::getSurface( void )
{
	return frameBuffer;
}

bool CVideo::isFullScreen() const { return fullScreen; }

void CVideo::setBpp( int bpp )
{
	this->bpp = bpp;
}

int CVideo::getBpp( void )
{
	return bpp;
}

int CVideo::set_help_string(const std::string& str)
{
	font::remove_floating_label(help_string_);

	const SDL_Color colour = {0x0,0x00,0x00,0x77};

#ifdef USE_TINY_GUI
	int size = font::SIZE_NORMAL;
#else
	int size = font::SIZE_LARGE;
#endif

	while(size > 0) {
		if(font::line_width(str, size) > getx()) {
			size--;
		} else {
			break;
		}
	}

#ifdef USE_TINY_GUI
	int border = 2;
#else
	int border = 5;
#endif

	help_string_ = font::add_floating_label(str,size,font::NORMAL_COLOUR,getx()/2,gety(),0.0,0.0,-1,screen_area(),font::CENTER_ALIGN,&colour,border);
	const SDL_Rect& rect = font::get_floating_label_rect(help_string_);
	font::move_floating_label(help_string_,0.0,-double(rect.h));
	return help_string_;
}

void CVideo::clear_help_string(int handle)
{
	if(handle == help_string_) {
		font::remove_floating_label(handle);
		help_string_ = 0;
	}
}

void CVideo::clear_all_help_strings()
{
	clear_help_string(help_string_);
}
