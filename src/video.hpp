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
#include "sdl_utils.hpp"

//possible flags when setting video modes
#define FULL_SCREEN SDL_FULLSCREEN
#define VIDEO_MEMORY SDL_HWSURFACE
#define SYSTEM_MEMORY SDL_SWSURFACE

surface display_format_alpha(surface surf);
surface get_video_surface();
SDL_Rect screen_area();

bool non_interactive();


void update_rect(size_t x, size_t y, size_t w, size_t h);
void update_rect(const SDL_Rect& rect);
void update_whole_screen();

class CVideo {
     public:
	CVideo();
	CVideo(int x, int y, int bits_per_pixel, int flags);
	~CVideo();

	int modePossible( int x, int y, int bits_per_pixel, int flags );
	int setMode( int x, int y, int bits_per_pixel, int flags );

	//did the mode change, since the last call to the modeChanged() method?
	bool modeChanged();

	int setGamma(float gamma);

	//functions to get the dimensions of the current video-mode
	int getx() const;
	int gety() const;
	int getBitsPerPixel();
	int getBytesPerPixel();
	int getRedMask();
	int getGreenMask();
	int getBlueMask();

	//functions to access the screen
	void lock();
	void unlock();
	int mustLock();

	//blits a surface with black as alpha
	void blit_surface(int x, int y, surface surf, SDL_Rect* srcrect=NULL, SDL_Rect* clip_rect=NULL);
	void flip();

	surface getSurface( void );

	bool isFullScreen() const;

	struct error {};

	struct quit {};

	//functions to allow changing video modes when 16BPP is emulated
	void setBpp( int bpp );
	int getBpp();

	void make_fake();
	bool faked() { return fake_screen; }

	//functions to set and clear 'help strings'. A 'help string' is like a tooltip, but it appears
	//at the bottom of the screen, so as to not be intrusive. Setting a help string sets what
	//is currently displayed there.
	int set_help_string(const std::string& str);
	void clear_help_string(int handle);
	void clear_all_help_strings();

	//function to stop the screen being redrawn. Anything that happens while
	//the update is locked will be hidden from the user's view.
	//note that this function is re-entrant, meaning that if lock_updates(true)
	//is called twice, lock_updates(false) must be called twice to unlock
	//updates.
	void lock_updates(bool value);
	bool update_locked() const;

private:

	bool mode_changed_;

	int bpp;	// Store real bits per pixel

	//if there is no display at all, but we 'fake' it for clients
	bool fake_screen;

	//variables for help strings
	int help_string_;

	int updatesLocked_;
};

//an object which will lock the display for the duration of its lifetime.
struct update_locker
{
	update_locker(CVideo& v, bool lock=true) : video(v), unlock(lock) {
		if(lock) {
			video.lock_updates(true);
		}
	}

	~update_locker() {
		unlock_update();
	}

	void unlock_update() {
		if(unlock) {
			video.lock_updates(false);
			unlock = false;
		}
	}

private:
	CVideo& video;
	bool unlock;
};

#endif
