/*
   Copyright (C) 2003 - 2015 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef VIDEO_HPP_INCLUDED
#define VIDEO_HPP_INCLUDED

#include "events.hpp"
#include "exceptions.hpp"
#include "lua_jailbreak_exception.hpp"

#include <boost/utility.hpp>

#if SDL_VERSION_ATLEAST(2,0,0)
#include "sdl/window.hpp"
#endif

struct surface;
#ifdef SDL_GPU
#include "sdl/shader.hpp"
#include "sdl/utils.hpp"

namespace sdl
{
class timage;
}
#endif

//possible flags when setting video modes
#if SDL_VERSION_ATLEAST(2, 0, 0)
#define SDL_APPMOUSEFOCUS	0x01		/**< The app has mouse coverage */
#define SDL_APPINPUTFOCUS	0x02		/**< The app has input focus */
#define SDL_APPACTIVE		0x04		/**< The application is active */
#endif

#ifdef SDL_GPU
struct GPU_Target;
GPU_Target *get_render_target();
#endif

surface display_format_alpha(surface surf);
surface& get_video_surface();
#if SDL_VERSION_ATLEAST(2, 0, 0)
//this needs to be invoked immediately after a resize event or the game will crash.
void update_framebuffer();
#endif


SDL_Rect screen_area();


bool non_interactive();

//which areas of the screen will be updated when the buffer is flipped?
void update_rect(size_t x, size_t y, size_t w, size_t h);
void update_rect(const SDL_Rect& rect);
void update_whole_screen();

class CVideo : private boost::noncopyable {
     public:
		 enum FAKE_TYPES {
			 NO_FAKE,
			 FAKE,
			 FAKE_TEST
		 };

	CVideo(FAKE_TYPES type = NO_FAKE);
	~CVideo();

	const static int DefaultBpp = 32;

	int bppForMode( int x, int y, int flags);
	int modePossible( int x, int y, int bits_per_pixel, int flags, bool current_screen_optimal=false);
	int setMode( int x, int y, int bits_per_pixel, int flags );

	//did the mode change, since the last call to the modeChanged() method?
	bool modeChanged();

	//functions to get the dimensions of the current video-mode
	int getx() const;
	int gety() const;

	//blits a surface with black as alpha
	void blit_surface(int x, int y, surface surf, SDL_Rect* srcrect=NULL, SDL_Rect* clip_rect=NULL);
#ifdef SDL_GPU
	GPU_Target *render_target() const;

	void draw_texture(sdl::timage &texture, int x, int y);
	void set_texture_color_modulation(int r, int g, int b, int a);
	void set_texture_submerge(float f);
	void set_texture_effects(int effects);

	void blit_to_overlay(surface surf, int x, int y);
	void clear_overlay_area(SDL_Rect area);
	void clear_overlay();
#endif
	void flip();

	surface& getSurface();

	bool isFullScreen() const;

	struct error : public game::error
	{
		error() : game::error("Video initialization failed") {}
	};

	class quit
		: public tlua_jailbreak_exception
	{
	public:

		quit()
			: tlua_jailbreak_exception()
		{
		}

	private:

		IMPLEMENT_LUA_JAILBREAK_EXCEPTION(quit)
	};

	//functions to allow changing video modes when 16BPP is emulated
#if !SDL_VERSION_ATLEAST(2, 0, 0)
	void setBpp( int bpp );
	int getBpp();
#endif

	void make_fake();
	/**
	 * Creates a fake frame buffer for the unit tests.
	 *
	 * @param width               The width of the buffer.
	 * @param height              The height of the buffer.
	 * @param bpp                 The bpp of the buffer.
	 */
	void make_test_fake(const unsigned width = 1024,
			const unsigned height = 768, const unsigned bpp = DefaultBpp);
	bool faked() const { return fake_screen_; }

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

#if SDL_VERSION_ATLEAST(2, 0, 0)
	/**
	 * Wrapper for SDL_GetAppState.
	 */
	static Uint8 window_state();

	/**
	 * Sets the title of the main window.
	 *
	 * @param title               The new title for the window.
	 */
	static void set_window_title(const std::string& title);

	/**
	 * Sets the icon of the main window.
	 *
	 * @param icon                The new icon for the window.
	 */
	static void set_window_icon(surface& icon);

	static sdl::twindow *get_window();
#endif

	/**
	 * Returns the list of available screen resolutions.
	 */
	std::vector<std::pair<int, int> > get_available_resolutions();

private:

	void initSDL();
#ifdef SDL_GPU
	void update_overlay(SDL_Rect *rect = NULL);

	sdl::shader_program shader_;
	surface overlay_;
#endif

	bool mode_changed_;

	int bpp_;	// Store real bits per pixel

	//if there is no display at all, but we 'fake' it for clients
	bool fake_screen_;

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

class resize_monitor : public events::pump_monitor {
	void process(events::pump_info &info);
};

//an object which prevents resizing of the screen occurring during
//its lifetime.
struct resize_lock {
	resize_lock();
	~resize_lock();
};

#endif
