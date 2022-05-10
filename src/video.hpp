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

#include <SDL2/SDL_render.h>

#include <memory>

class surface;
class texture;
struct point;
struct SDL_Texture;

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

	static bool setup_completed()
	{
		return singleton_ != nullptr;
	}

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

	bool non_interactive() const;

	bool surface_initialized() const;

	/***** ***** ***** ***** Window-related functions ***** ***** ****** *****/

	/** Initializes a new SDL window instance, taking into account any preiously saved states. */
	void init_window();

	/** Returns a pointer to the underlying SDL window. */
	sdl::window* get_window();

	/** Returns a pointer to the underlying window's renderer. */
	SDL_Renderer* get_renderer();

	bool has_window()
	{
		return get_window() != nullptr;
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

	bool supports_vsync() const;

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
	SDL_Rect draw_area() const;

	/**
	 * Returns the size and location of the window's input area in pixels.
	 * We use SDL_RendererSetLogicalSize to ensure this always matches
	 * draw_area(), but for clarity there are two separate functions.
	 */
	SDL_Rect input_area() const;

	/**
	 * Returns the width of the drawing surface in pixels.
	 * Input coordinates are automatically scaled to correspond,
	 * so this also indicates the width of the input surface.
	 */
	int get_width() const;

	/**
	 * Returns the height of the drawing surface in pixels.
	 * Input coordinates are automatically scaled to correspond,
	 * so this also indicates the height of the input surface.
	 */
	int get_height() const;

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

	/** The current game screen dpi. */
	std::pair<float, float> get_dpi() const;

	/** The current scale factor on High-DPI screens. */
	std::pair<float, float> get_dpi_scale_factor() const;

	/**
	 * Clip a rectangle to the drawing area.
	 *
	 * This does not change the original
	 * @param r                   The SDL_Rect to clip.
	 * @returns                   The new clipped SDL_Rect.
	 */
	SDL_Rect clip_to_draw_area(const SDL_Rect* r) const;

	/**
	 * Convert coordinates in draw space to coordinates in render space.
	 */
	SDL_Rect to_output(const SDL_Rect& draw_space_rect) const;

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

	int current_refresh_rate() const
	{
		return refresh_rate_;
	}

	/***** ***** ***** ***** Drawing functions ***** ***** ****** *****/

	/**
	 * Fills an area with the given colour.
	 *
	 * @param rect      The area to fill, in drawing coordinates.
	 * @param r         The red   component of the fill colour, 0-255.
	 * @param g         The green component of the fill colour, 0-255.
	 * @param b         The blue  component of the fill colour, 0-255.
	 * @param a         The alpha component of the fill colour, 0-255.
	 * @returns         0 on success, a negative SDL error code on failure.
	 */
	int fill(const SDL_Rect& rect, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

	/**
	 * Draws a surface at the given location.
	 *
	 * The w and h members of dst are ignored, but will be updated
	 * to reflect the final draw extents including clipping.
	 *
	 * The surface will be rendered in game-native resolution,
	 * and all coordinates are given in this context.
	 *
	 * @param surf                The surface to draw.
	 * @param dst                 Where to draw the surface. w and h are ignored, but will be updated to reflect the final draw extents including clipping.
	 */
	void blit_surface(const surface& surf, SDL_Rect* dst);

	/**
	 * Draws a surface at the given coordinates.
	 *
	 * The surface will be rendered in game-native resolution,
	 * and all coordinates are given in this context.
	 *
	 * @param x                   The x coordinate at which to draw.
	 * @param y                   The y coordinate at which to draw.
	 * @param surf                The surface to draw.
	 */
	void blit_surface(int x, int y, const surface& surf);

	/**
	 * Draws an area of a surface at the given location.
	 *
	 * The surface will be rendered in game-native resolution,
	 * and all coordinates are given in this context.
	 *
	 * @param x                   The x coordinate at which to draw.
	 * @param y                   The y coordinate at which to draw.
	 * @param surf                The surface to draw.
	 * @param srcrect             The area of the surface to draw. If null, the entire surface is drawn.
	 * @param clip_rect           The clipping area. If not null, the surface will only be drawn
	 *                            within the bounds of the given rectangle.
	 */
	void blit_surface(int x, int y, const surface& surf, const SDL_Rect* srcrect, const SDL_Rect* clip_rect);

	/**
	 * Draws a texture, or part of a texture, at the given location.
	 *
	 * The portion of the texture to be drawn will be scaled to fill
	 * the target rectangle.
	 *
	 * This version takes coordinates in game-native resolution,
	 * which may be lower than the final output resolution in high-dpi
	 * contexts or if pixel scaling is used. The texture will be copied
	 * in high-resolution if possible.
	 *
	 * @param tex           The texture to be copied / drawn.
	 * @param dstrect       The target location to copy the texture to,
	 *                      in low-resolution game-native drawing coordinates.
	 *                      If null, this fills the entire render target.
	 * @param srcrect       The portion of the texture to copy.
	 *                      If null, this copies the entire texture.
	 */
	void blit_texture(texture& tex, const SDL_Rect* dstrect = nullptr, const SDL_Rect* srcrect = nullptr);

	/**
	 * Render a portion of the low-resolution drawing surface.
	 *
	 * @param src_rect      The portion of the drawing surface to render, in draw-space coordinates. If null, the entire drawing surface is rendered.
	 */
	void render_low_res(SDL_Rect* src_rect);

	/**
	 * Render the entire low-resolution drawing surface.
	 */
	void render_low_res();

	/** Renders the screen. Should normally not be called directly! */
	void render_screen();

	/**
	 * Updates and ensures the framebuffer surface is valid.
	 * This needs to be invoked immediately after a resize event or the game will crash.
	 */
	void update_framebuffer();

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

	/** A class to manage automatic restoration of the clipping region.
	 *
	 * While this can be constructed on its own, it is usually easier to
	 * use the CVideo::set_clip() member function.
	 */
	class clip_setter
	{
	public:
		clip_setter(CVideo& video, const SDL_Rect& clip)
			: video_(video), old_clip_()
		{
			old_clip_ = video_.get_clip();
			video_.force_clip(clip);
		}

		~clip_setter()
		{
			video_.force_clip(old_clip_);
		}
	private:
		CVideo& video_;
		SDL_Rect old_clip_;
	};

	/**
	 * Set the clipping area. All draw calls will be clipped to this region.
	 *
	 * The clipping area is specified in draw-space coordinates.
	 *
	 * The returned object will reset the clipping area when it is destroyed,
	 * so it should be kept in scope until drawing is complete.
	 *
	 * @param clip          The clipping region in draw-space coordinates.
	 * @returns             A clip_setter object. When this object is destroyed
	 *                      the clipping region will be restored to whatever
	 *                      it was before this call.
	 */
	clip_setter set_clip(const SDL_Rect& clip);

	/**
	 * Set the clipping area, without any provided way of setting it back.
	 *
	 * @param clip          The clipping area, in draw-space coordinates.
	 */
	void force_clip(const SDL_Rect& clip);

	/** Get the current clipping area, in draw coordinates. */
	SDL_Rect get_clip() const;

	/***** ***** ***** ***** Help string functions ***** ***** ****** *****/

	/**
	 * Displays a help string with the given text. A 'help string' is like a tooltip,
	 * but appears at the bottom of the screen so as to not be intrusive.
	 *
	 * @param str                 The text to display.
	 *
	 * @returns                   The handle id of the new help string.
	 */
	int set_help_string(const std::string& str);

	/** Removes the help string with the given handle. */
	void clear_help_string(int handle);

	/** Removes all help strings. */
	void clear_all_help_strings();

	/***** ***** ***** ***** General utils ***** ***** ****** *****/

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
	static CVideo* singleton_;

	/** The SDL window object. */
	std::unique_ptr<sdl::window> window;

	/** The drawing texture. */
	SDL_Texture* drawing_texture_;

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

	/** Curent ID of the help string. */
	int help_string_;

	int updated_locked_;
	int flip_locked_;
	int refresh_rate_;
	int offset_x_, offset_y_;
	int pixel_scale_;
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

namespace video2
{
class draw_layering : public events::sdl_handler
{
protected:
	draw_layering(const bool auto_join = true);
	virtual ~draw_layering();
};

void trigger_full_redraw();
}
