/*
	Copyright (C) 2003 - 2024
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

namespace video
{

/******************/
/* Initialization */
/******************/

/**
 * For describing the type of faked display, if any.
 *
 * fake::no_window never tries to create a window, or draw anything.
 * fake::no_draw does create an offscreen window, but does not draw to it.
 * fake::hide_window creates a window as normal, but does not display it.
 */
enum class fake { none, no_window, no_draw, hide_window };

/**
 * Initialize the video subsystem.
 *
 * This must be called before attempting to use any video functions.
 */
void init(fake fake_type = fake::none);

/**
 * Deinitialize the video subsystem.
 *
 * This flushes all texture caches and disconnects the SDL video subsystem.
 */
void deinit();

/**
 * Update buffers to match current resolution and pixel scale settings.
 *
 * If @p autoupdate is true and buffers are changed by this call,
 * a full redraw is also triggered.
 *
 * If nothing has changed, it will not generate any new buffers or queue
 * the redraw.
 */
void update_buffers(bool autoupdate = true);


/**********************************/
/* Unit-test and headless support */
/**********************************/

/** The game is running headless. There is no window or renderer. */
bool headless();

/** The game is running unit tests. There is a window and offscreen
  * render buffer, but performing actual rendering is unnecessary. */
bool testing();


/***********************/
/* Windowing functions */
/***********************/

/** Whether the game has set up a window to render into */
bool has_window();

/** Whether we are currently in fullscreen mode */
bool is_fullscreen();

/**
 * Set the fullscreen state.
 *
 * If the setting matches the current fullscreen state, the window state
 * will not be changed.
 *
 * If false and the window is fullscreen, the window will be restored to
 * its last saved non-fullscreen configuration.
 */
void set_fullscreen(bool);

/**
 * Toggle fullscreen mode.
 *
 * Equivalent to set_fullscreen(!is_fullscreen()).
 */
void toggle_fullscreen();

/**
 * Set the window resolution.
 *
 * @todo this is no longer useful as fullscreen is always native resolution.
 *
 * @param resolution          The new width and height.
 *
 * @returns                   Whether the resolution was successfully changed.
 */
bool set_resolution(const point& resolution);

/** The current window size in desktop coordinates. */
point current_resolution();

/** Returns the list of available screen resolutions. */
std::vector<point> get_available_resolutions(bool include_current = false);

/** The current video driver in use, or else "<not initialized>". */
std::string current_driver();

/** A list of available video drivers. */
std::vector<std::string> enumerate_drivers();

/**
 * The refresh rate of the screen.
 *
 * In most cases, this will be the native refresh rate of the display, but
 * could be lower if FPS has been artificially capped (i.e., through --max-fps).
 *
 * If a refresh cannot be detected, this may return 0, or it may return a
 * substitute value.
 */
int current_refresh_rate();

/** The native refresh rate of display, not taking any user preferences into account. */
int native_refresh_rate();

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


/**********************/
/* Coordinate Systems */
/**********************/

/**
 * The game canvas area, in drawing coordinates.
 *
 * This is the "screen area", as seen by game systems, and as used for
 * specifying where to draw things on-screen. It may differ, in high-dpi
 * contexts, from input area, window area, and output area.
 *
 * Usually this is the only area game components should use or care about.
 *
 * The units it uses can be considered "pixels". Final output will be
 * rendered in higher resolution automatically if and when appropriate.
 */
rect game_canvas();

/** The size of the game canvas, in drawing coordinates / game pixels. */
point game_canvas_size();

/**
 * The size of the current render target in drawing coordinates.
 *
 * This will be the same as game_canvas_size() unless the render target
 * has been manually changed.
 */
point draw_size();

/**
 * The current drawable area.
 *
 * Equivalent to {0, 0, draw_size().x, draw_size().y}.
 */
rect draw_area();

/**
 * Returns the size of the final render target. This is irrelevant
 * for most purposes. Use game_canvas_size() in stead.
 */
point output_size();

/** {0, 0, output_size().x, output_size().y} */
rect output_area();

/**
 * Returns the size of the window in display units / screen coordinates.
 * This should match the value sent by window resize events, and also
 * those used for setting resolution.
 */
point window_size();

/**
 * Returns the input area of the window, in display coordinates.
 *
 * This can be slightly offset within the window, if the drawable area
 * is not the same as the full window area. This will happen if output
 * size is not a perfect multiple of the draw size.
 *
 * In general this will be almost, but not quite, equal to window_size().
 *
 * input_area() represents the portion of the window corresponding to
 * game_canvas().
 */
rect input_area();

/**
 * Get the current active pixel scale multiplier.
 * This is equal to output_size() / game_canvas_size().
 * Currently it is always integer, and the same in both dimensions.
 *
 * This may differ from prefs::get().pixel_scale() in some cases,
 * For example if the window is too small to fit the desired scale.
 *
 * @returns     The currently active pixel scale multiplier.
 */
int get_pixel_scale();

/**
 * Convert coordinates in draw space to coordinates in render space.
 */
rect to_output(const rect& draw_space_rect);


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

/** Reset the render target to the primary render buffer. */
void reset_render_target();

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
class quit final : public lua_jailbreak_exception
{
public:
	quit()
		: lua_jailbreak_exception()
	{
		this->store();
	}

private:
	IMPLEMENT_LUA_JAILBREAK_EXCEPTION(quit)
};


/***************/
/* Diagnostics */
/***************/

/**
 * Provides diagnostic information about the current renderer for the @a build_info API.
 */
std::vector<std::pair<std::string, std::string>> renderer_report();

/**
 * Retrieves the current game screen DPI for the @a build_info API.
 */
std::pair<float, float> get_dpi();


/**************************/
/* Implementation details */
/**************************/

/* This should only be used by draw.cpp for drawing, and texture.cpp for
 * texture creation. Try not to use it for anything else. */
SDL_Renderer* get_renderer();

/* This should not be used unless absolutely necessary. It's currently used
 * for Windows tray notification and that's it. If it can be refactored out
 * somehow then that would be best. */
SDL_Window* get_window();

} // namespace video
