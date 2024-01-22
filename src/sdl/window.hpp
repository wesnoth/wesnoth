/*
	Copyright (C) 2014 - 2024
	by Mark de Wever <koraq@xs4all.nl>
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

/**
 * @file
 * Contains a wrapper class for the SDL_Window class.
 */

#include "sdl/point.hpp"

#include <SDL2/SDL_video.h>

#include <string>

class surface;
struct SDL_Renderer;

namespace sdl
{

/**
 * The wrapper class for the SDL_Window class.
 *
 * At the moment of writing it is not certain yet how many windows will be
 * created. At least one as main window, but maybe the GUI dialogs will have
 * their own window. Once that is known it might be a good idea to evaluate
 * whether the class should become a singleton or not.
 *
 * The class also wraps several functions operating on SDL_Window objects.
 * For functions not wrapped the class offers an implicit conversion operator
 * to a pointer to the SDL_Window object it owns.
 */
class window
{
public:
	window(const window&) = delete;
	window& operator=(const window&) = delete;

	/***** ***** ***** Constructor and destructor. ***** ***** *****/

	/**
	 * Constructor.
	 *
	 * The function calls SDL_CreateWindow and SDL_CreateRenderer.
	 *
	 * @param title               Used as title for SDL_CreateWindow.
	 * @param x                   Used as x for SDL_CreateWindow.
	 * @param y                   Used as y for SDL_CreateWindow.
	 * @param w                   Used as w for SDL_CreateWindow.
	 * @param h                   Used as x for SDL_CreateWindow.
	 * @param window_flags        Used as flags for SDL_CreateWindow.
	 * @param render_flags        Used as flags for SDL_CreateRenderer.
	 */
	window(const std::string& title,
			const int x,
			const int y,
			const int w,
			const int h,
			const uint32_t window_flags,
			const uint32_t render_flags);

	~window();


	/***** ***** ***** Operations. ***** ***** *****/

	/**
	 * Wrapper for SDL_SetWindowSize.
	 *
	 * @param w                   Used as w for SDL_SetWindowSize.
	 * @param h                   Used as x for SDL_SetWindowSize.
	 */
	void set_size(const int w, const int h);

	/**
	 * Gets the window's size, in screen coordinates.
	 *
	 * For the most part, this seems to return the same result as @ref get_output_size. However,
	 * SDL indicates for high DPI displays these two functions could differ. I could not observe
	 * any change in behavior with DPI virtualization on or off, but to be safe, I'm keeping the
	 * two functions separate and using this for matters of window resolution.
	 *
	 * - vultraz, 6/27/2017
	 */
	SDL_Point get_size();

	/** Gets the window's renderer output size, in pixels */
	SDL_Point get_output_size();

	/**
	 * Dummy function for centering the window.
	 */
	void center();

	/**
	 * Dummy function for maximizing the window.
	 */
	void maximize();

	/**
	 * Dummy function for restoring the window.
	 */
	void restore();

	/**
	 * Dummy function for returning the window to windowed mode.
	 */
	void to_window();

	/**
	 * Dummy function for setting the window to fullscreen mode.
	 */
	void full_screen();

	/**
	 * Clears the contents of the window with a given color.
	 *
	 * @param r                   Red value of the color.
	 * @param g                   Green value of the color.
	 * @param b                   Blue value of the color.
	 * @param a                   Alpha value.
	 */
	void fill(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0);

	/** Renders the contents of the window. */
	void render();

	/**
	 * Sets the title of the window.
	 *
	 * This is a wrapper for SDL_SetWindowTitle.
	 *
	 * @param title               The new title for the window.
	 */
	void set_title(const std::string& title);

	/**
	 * Sets the icon of the window.
	 *
	 * This is a wrapper for SDL_SetWindowIcon.
	 *
	 * @note The @p icon is a SDL_Surface and not a SDL_Texture, this
	 * is part of the SDL 2 API.
	 *
	 * @param icon                The new icon for the window.
	 */
	void set_icon(const surface& icon);

	uint32_t get_flags();

	/**
	 * Set minimum size of the window.
	 *
	 * This is a wrapper for SDL_SetWindowMinimumWindowSize.
	 */
	void set_minimum_size(int min_w, int min_h);

	int get_display_index();

	/**
	 * Sets the desired size of the rendering surface. Input event coordinates
	 * will be scaled as if the window were also of this size. For best
	 * results this should be an integer fraction of the window size.
	 *
	 * This is a wrapper for SDL_RenderSetLogicalSize.
	 *
	 * @param w              Width of the window's rendering surface
	 * @param h              Height of the window's rendering surface
	 */
	void set_logical_size(int w, int h);
	void set_logical_size(const point& p);

	point get_logical_size() const;
	void get_logical_size(int& w, int& h) const;

	/** The current pixel format of the renderer. */
	uint32_t pixel_format();

	/***** ***** ***** Conversion operators. ***** ***** *****/

	/**
	 * Conversion operator to a SDL_Window*.
	 */
	operator SDL_Window*();

	/**
	 * Conversion operator to a SDL_Renderer*.
	 */
	operator SDL_Renderer*();

private:
	/***** ***** ***** Members. ***** ***** *****/

	/** The SDL_Window we own. */
	SDL_Window* window_;

	/** The preferred pixel format for the renderer. */
	uint32_t pixel_format_;
};

} // namespace sdl
