/*
	Copyright (C) 2008 - 2024
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

#include "color.hpp"
#include "font/text.hpp"

#include <string>
#include <string_view>

struct point;
class t_string;

namespace wfl
{
class map_formula_callable;
} // namespace wfl

namespace gui2
{
/**
 * Converts a color string to a color.
 *
 * @param color                   A color string, @see color_t::from_rgba_string.
 *
 * @returns                       The color.
 */
color_t decode_color(const std::string& color);

/**
 * Converts a text alignment string to a text alignment.
 *
 * @param alignment               An alignment string, possible values: "left", "right", "center".
 *
 * @returns                       The text alignment.
 */
PangoAlignment decode_text_alignment(const std::string& alignment);

/**
 * Converts a text weight string to a PangoWeight.
 *
 * @param weight                  A weight string, possible values: "thin", "light", "normal", "semibold", "bold", "heavy".
 *
 * @returns                       The corresponding PangoWeight.
 */
PangoWeight decode_text_weight(const std::string& weight);

/**
 * Converts a text style string to a PangoStyle.
 *
 * @param style                  A style string, possible values: "normal", "italic", "oblique".
 *
 * @returns                      The corresponding PangoStyle.
 */
PangoStyle decode_text_style(const std::string& style);

/**
 * Converts a font style string to a font style.
 *
 * @param style                   A font style string, possible values: "bold", "italic", "normal", "underline".
 *
 * @returns                       The font style.
 */
font::pango_text::FONT_STYLE decode_font_style(const std::string& style);

/**
 * Converts a text ellipsize mode string to a PangoEllipsizeMode.
 *
 * @param ellipsize_mode          A ellipsize mode string, possible values: "none", "start", "middle", "end".
 *
 * @returns                       The corresponding PangoEllipsizeMode.
 */
PangoEllipsizeMode decode_ellipsize_mode(const std::string& ellipsize_mode);

/**
 * Converts a PangoAlignment to its string representation.
 *
 * @param alignment               A PangoAlignment.
 *
 * @returns                       An alignment string, possible values: "left", "right", "center".
 */
std::string encode_text_alignment(const PangoAlignment alignment);

/**
 * Converts a PangoEllipsizeMode to its string representation.
 *
 * @param ellipsize_mode          A PangoEllipsizeMode.
 *
 * @returns                       An pango ellipsize mode string, possible values: "none", "start", "middle", "end".
 */
std::string encode_ellipsize_mode(const PangoEllipsizeMode ellipsize_mode);

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
std::string_view debug_truncate(std::string_view text);

} // namespace gui2
