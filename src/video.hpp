/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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

#include <memory>

#include "sdl/window.hpp"

class surface;

SDL_Rect screen_area();

class CVideo {
public:
	CVideo(const CVideo&) = delete;
	CVideo& operator=(const CVideo&) = delete;

	enum FAKE_TYPES {
		  NO_FAKE
		, FAKE
		, FAKE_TEST
	};

	enum MODE_EVENT {
		  TO_RES
		, TO_FULLSCREEN
		, TO_WINDOWED
		, TO_MAXIMIZED_WINDOW
	};

	CVideo(FAKE_TYPES type = NO_FAKE);
	~CVideo();

	static CVideo& get_singleton() { return *singleton_; }

	bool non_interactive() const;

	const static int DefaultBpp = 32;

	/**
	 * Initializes a new window, taking into account any preiously saved states.
	 */
	void init_window();

	void setMode( int x, int y, const MODE_EVENT mode );

	void set_fullscreen(bool ison);

	/**
	 * Set the resolution.
	 *
	 * @param width               The new width.
	 * @param height              The new height.
	 *
	 * @returns                   The status true if width and height are the
	 *                            size of the framebuffer, false otherwise.
	 */
	void set_resolution(const std::pair<int,int>& res);
	void set_resolution(const unsigned width, const unsigned height);

	std::pair<int,int> current_resolution();

	//functions to get the dimensions of the current video-mode
	int getx() const;
	int gety() const;

	//blits a surface with black as alpha
	void blit_surface(int x, int y, surface surf, SDL_Rect* srcrect=nullptr, SDL_Rect* clip_rect=nullptr);
	void flip();
	static void delay(unsigned int milliseconds);

	surface& getSurface();

	bool isFullScreen() const;

	struct error : public game::error
	{
		error() : game::error("Video initialization failed") {}
	};

	class quit
		: public lua_jailbreak_exception
	{
	public:

		quit()
			: lua_jailbreak_exception()
		{
		}

	private:

		IMPLEMENT_LUA_JAILBREAK_EXCEPTION(quit)
	};

	//functions to allow changing video modes when 16BPP is emulated

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

	//this needs to be invoked immediately after a resize event or the game will crash.
	void update_framebuffer();

	/**
	 * Sets the title of the main window.
	 *
	 * @param title               The new title for the window.
	 */
	void set_window_title(const std::string& title);

	/**
	 * Sets the icon of the main window.
	 *
	 * @param icon                The new icon for the window.
	 */
	void set_window_icon(surface& icon);

	sdl::window *get_window();

	/**
	 * Returns the list of available screen resolutions.
	 */
	std::vector<std::pair<int, int> > get_available_resolutions(const bool include_current = false);

	void lock_flips(bool);

private:
	static CVideo* singleton_;

	std::unique_ptr<sdl::window> window;
	class video_event_handler : public events::sdl_handler {
	public:
		virtual void handle_event(const SDL_Event &) {}

		virtual void handle_window_event(const SDL_Event &event);

		video_event_handler() :	sdl_handler(false) {}
	};

	void initSDL();

	//if there is no display at all, but we 'fake' it for clients
	bool fake_screen_;

	video_event_handler event_handler_;

	//variables for help strings
	int help_string_;

	int updatesLocked_;
	int flip_locked_;
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

class flip_locker
{
public:
	flip_locker(CVideo &video) : video_(video) {
		video_.lock_flips(true);
	}
	~flip_locker() {
		video_.lock_flips(false);
	}

private:
	CVideo& video_;
};


namespace video2 {
class draw_layering: public events::sdl_handler {
protected:
	draw_layering(const bool auto_join=true);
	virtual ~draw_layering();
};
void trigger_full_redraw();
}
#endif
