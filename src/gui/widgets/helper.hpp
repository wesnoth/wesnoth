/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_HELPER_HPP_INCLUDED
#define GUI_WIDGETS_HELPER_HPP_INCLUDED

#include "SDL.h"

#include <pango/pango-layout.h>

#include <string>

struct surface;
class t_string;

namespace game_logic {
class map_formula_callable;
} // namespace game_logic

namespace gui2 {

/**
 * Initializes the gui subsystems.
 *
 * This function needs to be called before other parts of the gui engine are
 * used.
 */
bool init();

/** Holds a 2D point. */
struct tpoint
{
	tpoint(const int x_, const int y_) :
		x(x_),
		y(y_)
		{}

	/** x coodinate. */
	int x;

	/** y coodinate. */
	int y;

	bool operator==(const tpoint& point) const { return x == point.x && y == point.y; }
	bool operator!=(const tpoint& point) const { return x != point.x || y != point.y; }
	bool operator<(const tpoint& point) const
		{ return x < point.x || (x == point.x && y < point.y); }

	bool operator<=(const tpoint& point) const
		{ return x < point.x || (x == point.x && y <= point.y); }

	tpoint operator+(const tpoint& point) const
		{ return tpoint(x + point.x, y + point.y); }

	tpoint& operator+=(const tpoint& point);

	tpoint operator-(const tpoint& point) const
		{ return tpoint(x - point.x, y - point.y); }

	tpoint& operator-=(const tpoint& point);
};

std::ostream &operator<<(std::ostream &stream, const tpoint& point);

/**
 * Creates a rectangle.
 *
 * @param origin                  The top left corner.
 * @param size                    The width (x) and height (y).
 *
 * @returns                       SDL_Rect with the proper rectangle.
 */
SDL_Rect create_rect(const tpoint& origin, const tpoint& size);

/**
 * Converts a colour string to a colour.
 *
 * @param colour                  A colour string see
 *                                http://www.wesnoth.org/wiki/GUIVariable for
 *                                more info.
 *
 * @returns                       The colour.
 */
Uint32 decode_colour(const std::string& colour);

/**
 * Converts a text alignment string to a text alignment.
 *
 * @param alignment               An alignment string see
 *                                http://www.wesnoth.org/wiki/GUIVariable for
 *                                more info.
 *
 * @returns                       The text alignment.
 */
PangoAlignment decode_text_alignment(const std::string& alignment);

/**
 * Converts a text alignment to its string representation.
 *
 * @param alignment              An alignment.
 *
 * @returns                       An alignment string see
 *                                http://www.wesnoth.org/wiki/GUIVariable for
 *                                more info.
 */
std::string encode_text_alignment(const PangoAlignment alignment);

/**
 * Converts a font style string to a font style.
 *
 * @param style                   A font style string see
 *                                http://www.wesnoth.org/wiki/GUIVariable for
 *                                more info.
 *
 * @returns                       The font style.
 */
unsigned decode_font_style(const std::string& style);

/**
 * Returns a default error message if a mandatory widget is ommited.
 *
 * @param id                      The id of the omitted widget.
 * @returns                       The error message.
 */
t_string missing_widget(const std::string& id);

/**
 * Gets an unique id for a widget.
 *
 * The id will have extra leading underscores so it's in the private range and
 * can't collide with user defined ids.
 *
 * @returns                       The id.
 */
std::string get_uid();

/**
 * Gets a formula object with the screen size.
 *
 * @param variable                A formula object in which the screen_width,
 *                                screen_height, gamemap_width and
 *                                gamemap_height variable will set to the
 *                                current values of these in settings. It
 *                                modifies the object send.
 */
void get_screen_size_variables(game_logic::map_formula_callable& variable);

/**
 * Gets a formula object with the screen size.
 *
 * @returns                       Formula object with the screen_width,
 *                                screen_height, gamemap_width and
 *                                gamemap_height variable set to the current
 *                                values of these in settings.
 */
game_logic::map_formula_callable get_screen_size_variables();

/**
 * Helper struct to get the same constness for T and U.
 *
 * @param T                       A type to determine the constness.
 * @param U                       Non const type to set the constness off.
 */
template<class T, class U>
struct tconst_duplicator
{
	/** The type to use, if T not const U is also not const. */
	typedef U type;
};

/** Specialialized version of tconst_duplicator when T is a const type. */
template<class T, class U>
struct tconst_duplicator<const T, U>
{
	/** The type to use, const U. */
	typedef const U type;
};

/** Returns the current mouse position. */
tpoint get_mouse_position();

/**
 * Returns a truncated version of the text.
 *
 * For debugging it's sometimes useful to get a part of the label of the
 * widget. This function shows the first part.
 *
 * @param text                    The text to truncate.
 *
 * @returns                       The truncated text.
 */
std::string debug_truncate(const std::string& text);

/**
 * Helper for function wrappers.
 *
 * For boost bind the a function sometimes needs to return a value althought
 * the function called doesn't return one. This wrapper function can return a
 * fixed result for a certain functor.
 *
 * @tparam R                      The return type.
 * @tparam F                      The type of the functor.
 *
 * @param result                  The result value.
 * @param function                The functor to call.
 *
 * @returns                       result.
 */
template<class R, class F>
R function_wrapper(const R result, const F& function)
{
	function();
	return result;
}

} // namespace gui2

#endif
