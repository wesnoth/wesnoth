/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "events.hpp"
#include "exceptions.hpp"
#include "lua_jailbreak_exception.hpp"
#include "ogl/context.hpp"

#include <SDL_render.h>

#include <memory>
#include <utility>

class surface;
class texture;
struct point;

namespace sdl
{
class window;
}

class CVideo
{
public:
	CVideo(const CVideo&) = delete;
	CVideo& operator=(const CVideo&) = delete;

	enum FAKE_TYPES { NO_FAKE, FAKE, FAKE_TEST };

	CVideo(FAKE_TYPES type = NO_FAKE);

	~CVideo();

	static CVideo& get_singleton()
	{
		return *singleton_;
	}

	/***** ***** ***** ***** Unit test-related functions ***** ***** ****** *****/

	void make_fake();

	/**
	 * Creates a fake frame buffer for the unit tests.
	 *
	 * @param width   The width of the buffer.
	 * @param height  The height of the buffer
	 */
	void make_test_fake(const unsigned width = 1024, const unsigned height = 768);

	bool faked() const
	{
		return fake_screen_;
	}

	bool non_interactive() const;

	/***** ***** ***** ***** Window-related functions ***** ***** ****** *****/

	/** Initializes a new SDL window instance, taking into account any preiously saved states. */
	void init_window();

	/** Returns a pointer to the underlying SDL window. */
	sdl::window* get_window();

	/** Returns a pointer to the underlying window's renderer. */
	SDL_Renderer* get_renderer();

private:
	enum MODE_EVENT { TO_RES, TO_FULLSCREEN, TO_WINDOWED, TO_MAXIMIZED_WINDOW };

	/**
	 * Sets the window's mode - ie, changing it to fullscreen, maximizing, etc.
	 *
	 * @param mode                The action to perform.
	 * @param size                The new window size. Utilized if @a mode is TO_RES.
	 */
	void set_window_mode(const MODE_EVENT mode, const point& size);

public:
	void set_fullscreen(bool ison);

	void toggle_fullscreen();

	bool is_fullscreen() const;

	bool set_resolution(const unsigned width, const unsigned height);

	/**
	 * Set the window resolution.
	 *
	 * @param resolution          The new width and height.
	 *
	 * @returns                   Whether the resolution was successfully changed.
	 */
	bool set_resolution(const point& resolution);

	point current_resolution();

	/** Returns the list of available screen resolutions. */
	std::vector<point> get_available_resolutions(const bool include_current = false);

	/**
	 * Returns the current window renderer area, either in pixels or screen coordinates.
	 *
	 * @param as_pixels           Whether to return the area in pixels (default true) or
	 *                            DPI-independent (DIP) screen coordinates.
	 */
	SDL_Rect screen_area(bool as_pixels = true) const;

	/** Returns the window renderer width in pixels or screen coordinates. */
	int get_width(bool as_pixels = true) const;

	/** Returns the window renderer height in pixels or in screen coordinates. */
	int get_height(bool as_pixels = true) const;

	/** The current scale factor on High-DPI screens. */
	std::pair<float, float> get_dpi_scale_factor() const;

	/**
	 * Tests whether the given flags are currently set on the SDL window.
	 *
	 * @param flags               The flags to test, OR'd together.
	 */
	bool window_has_flags(uint32_t flags) const;

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

	/***** ***** ***** ***** Drawing functions ***** ***** ****** *****/

private:
	/** Renders the screen. */
	void render_screen();

	/** events::run_event_loop() is the only place that should call render_screen(). */
	friend void events::run_event_loop();

public:
	void render_copy(const texture& txt,
		SDL_Rect* src_rect = nullptr,
		SDL_Rect* dst_rect = nullptr,
		const bool flip_h = false,
		const bool flip_v = false);

	/** Clear the screen contents */
	void clear_screen();

	/**
	 * Stop the screen being redrawn. Anything that happens while the updates are locked will
	 * be hidden from the user's view.
	 *
	 * Note that this function is re-entrant, meaning that if lock_updates(true) is called twice,
	 * lock_updates(false) must be called twice to unlock updates.
	 */
	void lock_updates(bool value);

	/** Whether the screen has been 'locked' or not. */
	bool update_locked() const;

	void lock_flips(bool);

	/***** ***** ***** ***** General utils ***** ***** ****** *****/

	/** Waits a given number of milliseconds before returning. */
	static void delay(unsigned int milliseconds);

	struct error : public game::error
	{
		error()
			: game::error("Video initialization failed")
		{
		}
	};

	/** Type that can be thrown as an exception to quit to desktop. */
	class quit : public lua_jailbreak_exception
	{
	public:
		quit()
			: lua_jailbreak_exception()
		{
		}

	private:
		IMPLEMENT_LUA_JAILBREAK_EXCEPTION(quit)
	};

private:
	static CVideo* singleton_;

	/** The SDL window object. */
	std::unique_ptr<sdl::window> window;

#ifdef USE_GL_RENDERING
	/** The OpenGL context attached to the SDL window. */
	std::unique_ptr<gl::context> gl_context;
#endif

	/** Initializes the SDL video subsystem. */
	void initSDL();

	// if there is no display at all, but we 'fake' it for clients
	bool fake_screen_;

	std::pair<unsigned, unsigned> fake_size_;

	/** Helper class to manage SDL events. */
	class video_event_handler : public events::sdl_handler
	{
	public:
		virtual void handle_event(const SDL_Event&)
		{
		}

		virtual void handle_window_event(const SDL_Event& event);

		video_event_handler()
			: sdl_handler(false)
		{
		}
	};

	video_event_handler event_handler_;

	int updated_locked_;
	int flip_locked_;
};

/** An object which will lock the display for the duration of its lifetime. */
struct update_locker
{
	update_locker(CVideo& v, bool lock = true)
		: video(v)
		, unlock(lock)
	{
		if(lock) {
			video.lock_updates(true);
		}
	}

	~update_locker()
	{
		unlock_update();
	}

	void unlock_update()
	{
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
	flip_locker(CVideo& video)
		: video_(video)
	{
		video_.lock_flips(true);
	}

	~flip_locker()
	{
		video_.lock_flips(false);
	}

private:
	CVideo& video_;
};
