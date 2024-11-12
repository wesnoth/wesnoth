/*
	Copyright (C) 2024
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
#include "config.hpp"
// This file isn't needed by any of these functions, but this allows any
// standard color to be passed to span_color without an extra include.
#include "font/standard_colors.hpp"
#include "formatter.hpp"
#include "serialization/string_utils.hpp"

#include <string>

namespace markup {

/**
 * Returns a Help markup tag corresponding to a linebreak.
 * See @ref gui2::rich_label for details on how this tag is parsed.
 */
const static std::string br = "<br/>";

/**
 * Returns the contents enclosed inside `<tag_name>` and `</tag_name>`.
 * Does not escape its contents.
 */
template<typename... Args>
std::string tag(const std::string& tag_name, Args&&... contents)
{
	return ((formatter()
		<< "<" << tag_name << ">")
		<< ...
		<< contents)
		<< "</" << tag_name << ">";
}

/**
 * Returns a Pango formatting string using the provided color_t object.
 *
 * The string returned will be in format: `<span foreground=#color>#data</span>`
 *
 * @param color        The color_t object from which to retrieve the color.
 * @param data         The string to enclose inside the tag. All elements in this list
 *                     will be concatenated to formatter(). This function does not escape
 *                     data internally, so data should be escaped by the caller if needed.
 */
template<typename... Args>
std::string span_color(const color_t& color, Args&&... data)
{
	return ((formatter() << "<span color='" << color.to_hex_string() << "'>") << ... << data) << "</span>";
}

/**
 * Returns a Pango formatting string using the provided hex color string.
 *
 * The string returned will be in format: `<span foreground=#color>#data</span>`
 *
 * @param color        The hex color string.
 * @param data         The string to enclose inside the tag. All elements in this list
 *                     will be concatenated to formatter(). This function does not escape
 *                     data internally, so data should be escaped by the caller if needed.
 */
template<typename... Args>
std::string span_color(const std::string& color, Args&&... data)
{
	return ((formatter() << "<span color='" << color << "'>") << ... << data) << "</span>";
}

/**
 * Returns a Pango formatting string that set the font size of the enclosed data.
 *
 * The string returned will be in format: `<span size=#size>#data</span>`
 *
 * @param size         The font size. String so values like x-large, large etc could be used.
 * @param data         The string to enclose inside the tag. All elements in this list
 *                     will be concatenated to formatter(). This function does not escape
 *                     data internally, so data should be escaped by the caller if needed.
 */
template<typename... Args>
std::string span_size(const std::string& size, Args&&... data)
{
	return ((formatter() << "<span size='" << size << "'>") << ... << data) << "</span>";
}

/**
 * Returns a Pango formatting string corresponding to bold formatting.
 *
 * @param data      The string to enclose in bold tag. This function does not escape
 *                  data internally, so data should be escaped by the caller if needed.
 */
template<typename... Args>
std::string bold(Args&&... data)
{
	return tag("b", (formatter() << ... << data).str());
}

/**
 * Returns a Pango formatting string corresponding to italic formatting.
 *
 * @param data      The string to enclose in italic tag. This function does not escape
 *                  data internally, so data should be escaped by the caller if needed.
 */
template<typename... Args>
std::string italic(Args&&... data)
{
	return tag("i", (formatter() << ... << data).str());
}

/**
 * Returns a Help markup tag corresponding to an image. This function does not escape
 * strings internally, so should be escaped by the caller if needed.
 * See @ref gui2::rich_label for details on how this tag is parsed.
 *
 * @param src       The WML path to where the image is located. (i.e., 'units/drakes/arbiter.png')
 * @param align     Alignment of this image. Possible values: left, right, center.
 * @param floating  Is the image a floating image or an inline image?
 *
 */
std::string img(const std::string& src, const std::string& align = "left", const bool floating = false);

/**
 * Returns a Help markup tag corresponding to a reference or link. This function does not
 * escape strings internally, so should be escaped by the caller if needed.
 * See @ref gui2::rich_label for details on how this tag is parsed.
 *
 * @param text      User visible text/caption of the link.
 * @param dst       Destination of the link. Can be any string depending on the link handler in the parsing @ref gui2::rich_label.
 *
 */
std::string make_link(const std::string& text, const std::string& dst);

//
// Markup Parser
//

/** Thrown when the help system fails to parse something. */
struct parse_error : public game::error
{
	parse_error(const std::string& msg) : game::error(msg) {}
};

/**
 * Parse a xml style marked up text string. Return a config with the different parts of the
 * text. Each markup item is a separate part while the text between
 * markups are separate parts.
 */
config parse_text(const std::string &text);

} //end namespace markup
