/*
   Copyright (C) 2014 - 2015 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SDL_WINDOW_HPP_INCLUDED
#define SDL_WINDOW_HPP_INCLUDED

/**
 * @file
 * Contains a wrapper class for the @ref SDL_Window class.
 */

#include <SDL_version.h>

#if SDL_VERSION_ATLEAST(2, 0, 0)

#include "sdl/utils.hpp"

#include <boost/noncopyable.hpp>

#include <SDL_video.h>

#include <string>

struct surface;
struct SDL_Renderer;

namespace sdl
{

/**
 * The wrapper class for the @ref SDL_Window class.
 *
 * At the moment of writing it is not certain yet how many windows will be
 * created. At least one as main window, but maybe the GUI dialogues will have
 * their own window. Once that is known it might be a good idea to evaluate
 * whether the class should become a singleton or not.
 *
 * The class also wraps several functions operating on @ref SDL_Window objects.
 * For functions not wrapped the class offers an implicit conversion operator
 * to a pointer to the @ref SDL_Window object it owns.
 */
class twindow : private boost::noncopyable
{
public:
	/***** ***** ***** Constructor and destructor. ***** ***** *****/

	/**
	 * Constructor.
	 *
	 * The function calls @ref SDL_CreateWindow and @ref SDL_CreateRenderer.
	 *
	 * @param title               Used as title for @ref SDL_CreateWindow.
	 * @param x                   Used as x for @ref SDL_CreateWindow.
	 * @param y                   Used as y for @ref SDL_CreateWindow.
	 * @param w                   Used as w for @ref SDL_CreateWindow.
	 * @param h                   Used as x for @ref SDL_CreateWindow.
	 * @param window_flags        Used as flags for @ref SDL_CreateWindow.
	 * @param render_flags        Used as flags for @ref SDL_CreateRenderer.
	 */
	twindow(const std::string& title,
			const int x,
			const int y,
			const int w,
			const int h,
			const Uint32 window_flags,
			const Uint32 render_flags);

	~twindow();


	/***** ***** ***** Operations. ***** ***** *****/

	/**
	 * Wrapper for @ref SDL_SetWindowSize.
	 *
	 * @param w                   Used as w for @ref SDL_SetWindowSize.
	 * @param h                   Used as x for @ref SDL_SetWindowSize.
	 */
	void set_size(const int w, const int h);

	/**
	 * Dummy function for setting the screen to full screen mode.
	 *
	 * @todo Implement this function properly.
	 */
	void full_screen();

	/**
	 * Clears the contents of the window with a given color.
	 *
	 * @param r                   Red value of the color.
	 * @param g                   Green value of the color.
	 * @param b                   Blue value of the color.
	 */
	void fill(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 0);

	/** Renders the contents of the window. */
	void render();

	/**
	 * Sets the title of the window.
	 *
	 * This is a wrapper for @ref SDL_SetWindowTitle.
	 *
	 * @param title               The new title for the window.
	 */
	void set_title(const std::string& title);

	/**
	 * Sets the icon of the window.
	 *
	 * This is a wrapper for @ref SDL_SetWindowIcon.
	 *
	 * @note The @p icon is a @ref SDL_Surface and not a @ref SDL_Texture, this
	 * is part of the SDL 2 API.
	 *
	 * @param icon                The new icon for the window.
	 */
	void set_icon(const surface& icon);

	/***** ***** ***** Conversion operators. ***** ***** *****/

	/**
	 * Conversion operator to a SDL_Window*.
	 */
	operator SDL_Window*();


private:
	/**
	 * Conversion operator to a SDL_Renderer*.
	 *
	 * @todo Evaluate whether the function should become public or not.
	 */
	operator SDL_Renderer*();

	/***** ***** ***** Members. ***** ***** *****/

	/** The @ref SDL_Window we own. */
	SDL_Window* window_;

	/** The preferred pixel format for the renderer. */
	Uint32 pixel_format_;
};

} // namespace sdl

#endif

#endif
