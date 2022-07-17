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

#include "exceptions.hpp"
#include "lua_jailbreak_exception.hpp"
#include "sdl/point.hpp"
#include "sdl/rect.hpp"
#include "sdl/texture.hpp"

#include <SDL2/SDL_render.h>

#include <vector>

class surface;
class texture;

namespace video
{

/******************/
/* Initialization */
/******************/

// TODO: this whole faking system is a mess
/**
 * For describing the type of faked display, if any.
 *
 * fake::window never tries to create a window, or draw anything.
 * fake::draw does create an offscreen window, but does not draw to it.
 */
enum class fake { none, window, draw };

/**
 * Initialize the video subsystem.
 *
 * This must be called before attempting to use any video functions.
 */
void init(fake fake_type = fake::none);


/********************************/
/* Unit-test and No-GUI support */
/********************************/


// TODO: tests are calling this constantly, refactor the tests to not do that
/**
 * Creates a fake frame buffer for the unit tests.
 *
 * @param width               The width of the buffer.
 * @param height              The height of the buffer.
 */
void make_test_fake(const unsigned width = 1024, const unsigned height = 768);

// TODO: rename and refactor - these don't clearly express what they mean
bool faked();
bool any_fake();
bool non_interactive();


/****************************/
/* Window-related functions */
/****************************/
// TODO: other things probably should never be calling these. remove/refactor

/** Initializes a new SDL window instance, taking into account any preiously saved states. */
void init_window();

// private:
void init_fake_window();

// TODO: refactor? only draw.cpp and texture.cpp should need to access the renderer

/** Returns a pointer to the underlying window's renderer. */
SDL_Renderer* get_renderer();

/** The current video driver in use, or else "<not initialized>". */
std::string current_driver();

/** A list of available video drivers. */
std::vector<std::string> enumerate_drivers();


/************/
/* Resizing */
/************/

// TODO: check these are okay and don't do bad things
void set_fullscreen(bool);
void toggle_fullscreen();
bool is_fullscreen();

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
std::vector<point> get_available_resolutions(bool include_current = false);


/*******/
/* IDK */
/*******/

/**
 * Update buffers to match current resolution and pixel scale settings.
 *
 * If @p autoupdate is true and buffers are changed by this call,
 * a full redraw is also triggered.
 */
void update_buffers(bool autoupdate = true);


/***********/
/* Queries */
/***********/

// TODO: revisit these for correctness and usefulness
/**
 * Returns the size of the final render target. This is irrelevant
 * for most purposes. Use draw_area() in stead.
 */
point output_size();

/**
 * Returns the size of the window in display units / screen coordinates.
 * This should match the value sent by window resize events, and also
 * those used for setting resolution.
 */
point window_size();

/**
 * Returns the size and location of the current drawing area in pixels.
 * This will usually be an SDL_Rect indicating the full drawing surface.
 */
rect draw_area();

/**
 * Returns the size and location of the window's input area in pixels.
 * We use SDL_RendererSetLogicalSize to ensure this always matches
 * draw_area(), but for clarity there are two separate functions.
 */
rect input_area();

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
int get_pixel_scale();

// TODO: move
/**
 * Convert coordinates in draw space to coordinates in render space.
 */
rect to_output(const rect& draw_space_rect);

// TODO: move these:

/** True iff the window is not hidden. */
bool window_is_visible();
/** True iff the window has mouse or input focus */
bool window_has_focus();
/** True iff the window has mouse focus */
bool window_has_mouse_focus();

/** Sets the title of the main window. */
void set_window_title(const std::string& title);

/** Sets the icon of the main window. */
void set_window_icon(surface& icon);

int current_refresh_rate();


/******************/
/* Screen capture */
/******************/
// These functions are slow, and intended only for screenshots.

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


/****************************/
/* Render target management */
/****************************/

// TODO: revisit who owns this
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


/*******************/
/* Exception types */
/*******************/

/** An error specifically indicating video subsystem problems. */
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

} // namespace video
