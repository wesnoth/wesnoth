/*
	Copyright (C) 2003 - 2022
	by David White <dave@whitevine.net>
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
#include "sdl/point.hpp"
#include "sdl/rect.hpp"
#include "sdl/texture.hpp"

#include <SDL2/SDL_render.h>

#include <cassert>
#include <memory>

class surface;
class texture;
struct SDL_Texture;
struct color_t;

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

	// TODO: refactor? only used by build_info.cpp
	static bool setup_completed()
	{
		return singleton_ != nullptr;
	}

	// TODO: remove / replace
	static CVideo& get_singleton()
	{
		return *singleton_;
	}

	/***** ***** ***** ***** Unit test-related functions ***** ***** ****** *****/

	void make_fake();

	/**
	 * Creates a fake frame buffer for the unit tests.
	 *
	 * @param width               The width of the buffer.
	 * @param height              The height of the buffer.
	 */
	void make_test_fake(const unsigned width = 1024, const unsigned height = 768);

	bool faked() const
	{
		return fake_screen_;
	}

	bool any_fake() const;

	bool non_interactive() const;

	/***** ***** ***** ***** Window-related functions ***** ***** ****** *****/

	/** Initializes a new SDL window instance, taking into account any preiously saved states. */
	void init_window();
private:
	void init_fake_window();

public:
	/** Returns a pointer to the underlying window's renderer. */
	SDL_Renderer* get_renderer();

	// TODO: refactor? only used by build_info.cpp
	bool has_window()
	{
		return window != nullptr;
	}

	static std::string current_driver();

	static std::vector<std::string> enumerate_drivers();

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

	/**
	 * Set the window resolution.
	 *
	 * @param resolution          The new width and height.
	 *
	 * @returns                   Whether the resolution was successfully changed.
	 */
	bool set_resolution(const point& resolution);

	point current_resolution();

	// TODO: refactor? only used by preferences_dialog.cpp
	/**
	 * Update buffers to match current resolution and pixel scale settings.
	 * Also triggers a full redraw.
	 */
	void update_buffers();

	/** Returns the list of available screen resolutions. */
	std::vector<point> get_available_resolutions(const bool include_current = false);

	/**
	 * Returns the size of the final render target. This is irrelevant
	 * for most purposes. Use draw_area() in stead.
	 */
	SDL_Point output_size() const;

	/**
	 * Returns the size of the window in display units / screen coordinates.
	 * This should match the value sent by window resize events, and also
	 * those used for setting resolution.
	 */
	SDL_Point window_size() const;

	/**
	 * Returns the size and location of the current drawing area in pixels.
	 * This will usually be an SDL_Rect indicating the full drawing surface.
	 */
	rect draw_area() const;

	/**
	 * Returns the size and location of the window's input area in pixels.
	 * We use SDL_RendererSetLogicalSize to ensure this always matches
	 * draw_area(), but for clarity there are two separate functions.
	 */
	SDL_Rect input_area() const;

	/**
	 * Get the current active pixel scale multiplier.
	 * This is equal to output_size() / draw_area().
	 * Currently it is always integer, and the same in both dimensions.
	 *
	 * This may differ from preferences::pixel_scale() in some cases,
	 * For example if the window is too small to fit the desired scale.
	 *
	 * @returns     The currently active pixel scale multiplier.
	 */
	int get_pixel_scale() const { return pixel_scale_; }

	/**
	 * Convert coordinates in draw space to coordinates in render space.
	 */
	rect to_output(const rect& draw_space_rect) const;

	// TODO: split use into individual functions to avoid use of SDL enums
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

	// TODO: this should be more clever, depending on usage
	int current_refresh_rate() const
	{
		return refresh_rate_;
	}

	/***** ***** ***** ***** Drawing functions ***** ***** ****** *****/

	// TODO: hide this so it's not tempting to use
	/** Renders the screen. Should normally not be called directly! */
	void render_screen();

	// TODO: refactor? this shouldn't be public. why does events.cpp use it?
	/**
	 * Updates and ensures the framebuffer surface is valid.
	 * This needs to be invoked immediately after a resize event or the game will crash.
	 */
	void update_framebuffer();
private:
	void update_framebuffer_fake();

public:
	// TODO: this should probably be in draw
	/** Clear the screen contents */
	void clear_screen();

	/**
	 * Copy back a portion of the render target that is already drawn.
	 *
	 * This area is specified in draw coordinates, not render coordinates.
	 * Thus the size of the retrieved surface may not match the size of r.
	 *
	 * If not null, r will be automatically clipped to the drawing area.
	 *
	 * Note: This is a very slow function! Its use should be phased out
	 * for everything except maybe screenshots.
	 *
	 * @param r       The portion of the render target to retrieve, in
	 *                draw coordinates.
	 *                If not null, this will be modified to reflect the
	 *                portion of the draw area that has been returned.
	 */
	surface read_pixels(SDL_Rect* r = nullptr);

	/**
	 * The same as read_pixels, but returns a low-resolution surface
	 * suitable for use with the old drawing system.
	 *
	 * This should be considered deprecated, and phased out ASAP.
	 */
	surface read_pixels_low_res(SDL_Rect* r = nullptr);

	/**
	 * Copy a portion of the render target to another texture.
	 *
	 * This area is specified in draw coordinates, not render coordinates.
	 * Thus the size of the retrieved texture may not match the size of r.
	 *
	 * If not null, r will be automatically clipped to the drawing area.
	 *
	 * Note: This is a very slow function! Its use should be phased out
	 * for everything except maybe screenshots.
	 *
	 * @param r       The portion of the render target to retrieve, in
	 *                draw coordinates.
	 *                If not null, this will be modified to reflect the
	 *                portion of the draw area that has been returned.
	 */
	texture read_texture(SDL_Rect* r = nullptr);

	/***** ***** ***** ***** State management ***** ***** ****** *****/

	/**
	 * Set the render target, without any provided way of setting it back.
	 *
	 * End-users should not use this function directly. In stead use
	 * draw::set_render_target(), which returns a setter object which
	 * will automatically restore the render target upon leaving scope.
	 *
	 * @param t     The new render target. This must be a texture created
	 *              with SDL_TEXTUREACCESS_TARGET, or an empty texture to
	 *              indicate the underlying window.
	 */
	void force_render_target(const texture& t);

	/** Reset the render target to the main window / screen. */
	void clear_render_target();

	/** Get the current render target.
	 *
	 * Will return an empty texture if the render target is the underlying
	 * window.
	 */
	texture get_render_target();

	/***** ***** ***** ***** General utils ***** ***** ****** *****/

	// TODO: this should not be here
	/** Waits a given number of milliseconds before returning. */
	static void delay(unsigned int milliseconds);

	struct error : public game::error
	{
		error() : game::error("unspecified video subsystem error") {}
		error(const std::string& msg) : game::error(msg) {}
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
	static inline CVideo* singleton_ = nullptr;

	/** The SDL window object. */
	std::unique_ptr<sdl::window> window;

	/** The main offscreen render target. */
	texture render_texture_ = {};

	/** The current offscreen render target. */
	texture current_render_target_ = {};

	/** Initializes the SDL video subsystem. */
	void initSDL();

	// if there is no display at all, but we 'fake' it for clients
	bool fake_screen_;

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

	int refresh_rate_;
	int offset_x_, offset_y_;
	point logical_size_;
	int pixel_scale_;
};
