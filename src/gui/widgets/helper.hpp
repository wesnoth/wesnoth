/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_HELPER_HPP_INCLUDED
#define GUI_WIDGETS_HELPER_HPP_INCLUDED

#include "font/text.hpp"
#include "color.hpp"

#include <pango/pango-layout.h>

#include <cstdint>
#include <string>

struct SDL_Rect;
class surface;
class t_string;

namespace wfl
{
class map_formula_callable;
} // namespace wfl

namespace gui2
{

struct point;

/**
 * Initializes the gui subsystems.
 *
 * This function needs to be called before other parts of the gui engine are
 * used.
 */
bool init();

/**
 * Creates a rectangle.
 *
 * @param origin                  The top left corner.
 * @param size                    The width (x) and height (y).
 *
 * @returns                       SDL_Rect with the proper rectangle.
 */
SDL_Rect create_rect(const point& origin, const point& size);

/**
 * Converts a color string to a color.
 *
 * @param color                  A color string see
 *                                http://www.wesnoth.org/wiki/GUIVariable for
 *                                more info.
 *
 * @returns                       The color.
 */
color_t decode_color(const std::string& color);

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
font::pango_text::FONT_STYLE decode_font_style(const std::string& style);

/**
 * Returns a default error message if a mandatory widget is omitted.
 *
 * @param id                      The id of the omitted widget.
 * @returns                       The error message.
 */
t_string missing_widget(const std::string& id);

/**
 * Gets a formula object with the screen size.
 *
 * @param variable                A formula object in which the screen_width,
 *                                screen_height, gamemap_width and
 *                                gamemap_height variable will set to the
 *                                current values of these in settings. It
 *                                modifies the object send.
 */
void get_screen_size_variables(wfl::map_formula_callable& variable);

/**
 * Gets a formula object with the screen size.
 *
 * @returns                       Formula object with the screen_width,
 *                                screen_height, gamemap_width and
 *                                gamemap_height variable set to the current
 *                                values of these in settings.
 */
wfl::map_formula_callable get_screen_size_variables();

/** Returns the current mouse position. */
point get_mouse_position();

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

} // namespace gui2

#endif
