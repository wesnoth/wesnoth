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
#include "exceptions.hpp"
// This file isn't needed by any of these functions, but this allows any
// standard color to be passed to span_color without an extra include.
#include "font/standard_colors.hpp"
#include "formatter.hpp"

#include <string>
#include <string_view>

class config;

namespace markup
{
/**
 * A Help markup tag corresponding to a linebreak.
 * @see gui2::rich_label for details on how this tag is parsed.
 */
constexpr std::string_view br{"<br/>"};

/**
 * Wraps the given data in the specified formatting tag.
 *
 * @param tag       The formatting tag ("b", "i", etc).
 * @param data      The content to wrap with @a tag.
 *                  Each argument must be writable to a stringstream.
 *
 * @note            Special formatting characters in the input are not escaped.
 *                  If such behavior is needed, it must be handled by the caller.
 *                  If the concatenation of @a data results in an empty string,
 *                  an empty string is returned in lieu of formatting tags.
 */
template<typename... Args>
std::string tag(std::string_view tag, Args&&... data)
{
	std::string input = (formatter() << ... << data);
	if(input.empty()) return {};
	return formatter() << "<" << tag << ">" << input << "</" << tag << ">";
}

/**
 * Wraps the given data in a @c span tag with the specified attribute and value.
 *
 * @param key       The span attribute ("color", "size", etc).
 * @param value     The attribute value.
 * @param data      The content to format.
 *
 * @note            See @ref tag for more information.
 */
template<typename Value, typename... Args>
std::string span_attribute(std::string_view key, const Value& value, Args&&... data)
{
	std::string input = (formatter() << ... << data);
	if(input.empty()) return {};
	return formatter() << "<span " << key << "='" << value << "'>" << input << "</span>";
}

/**
 * Applies Pango markup to the input specifying its display color.
 *
 * @param color     The color_t object from which to retrieve the color.
 * @param data      Variable list of content to enclose inside the span tag.
 *                  Each argument must be writable to a stringstream.
 *
 * @returns         @code `<span color='#color'>#data</span>` @endcode
 *
 * @note            Special formatting characters in the input are not escaped.
 *                  If such behavior is needed, it must be handled by the caller.
 */
template<typename... Args>
std::string span_color(const color_t& color, Args&&... data)
{
	return span_attribute("color", color.to_hex_string(), std::forward<Args>(data)...);
}

/**
 * Applies Pango markup to the input specifying its display color.
 *
 * @param color     The hex color string.
 * @param data      Variable list of content to enclose inside the span tag.
 *                  Each argument must be writable to a stringstream.
 *
 * @returns         @code `<span color='#color'>#data</span>` @endcode
 *
 * @note            Special formatting characters in the input are not escaped.
 *                  If such behavior is needed, it must be handled by the caller.
 */
template<typename... Args>
std::string span_color(std::string_view color, Args&&... data)
{
	return span_attribute("color", color, std::forward<Args>(data)...);
}

/**
 * Applies Pango markup to the input specifying its display size.
 *
 * @param size      A Pango string size specifier (large, small, x-large, etc).
 * @param data      Variable list of content to concatenate inside the span tag.
 *                  Each argument must be writable to a stringstream.
 *
 * @returns         @code `<span size='#size'>#data</span>` @endcode
 *
 * @note            Special formatting characters in the input are not escaped.
 *                  If such behavior is needed, it must be handled by the caller.
 */
template<typename... Args>
std::string span_size(std::string_view size, Args&&... data)
{
	return span_attribute("size", size, std::forward<Args>(data)...);
}

/**
 * Applies bold Pango markup to the input.
 *
 * @param data      Variable list of content to concatenate inside the bold tag.
 *                  Each argument must be writable to a stringstream.
 *
 * @note            Special formatting characters in the input are not escaped.
 *                  If such behavior is needed, it must be handled by the caller.
 */
template<typename... Args>
std::string bold(Args&&... data)
{
	return tag("b", std::forward<Args>(data)...);
}

/**
 * Applies italic Pango markup to the input.
 *
 * @param data      Variable list of content to enclose inside the italic tag.
 *                  Each argument must be writable to a stringstream.
 *
 * @note            Special formatting characters in the input are not escaped.
 *                  If such behavior is needed, it must be handled by the caller.
 */
template<typename... Args>
std::string italic(Args&&... data)
{
	return tag("i", std::forward<Args>(data)...);
}

/**
 * Generates a Help markup tag corresponding to an image.
 *
 * @param src       The WML path to the image (i.e., 'units/drakes/arbiter.png')
 * @param align     Alignment of the image. Possible values: left, right, center.
 * @param floating  Is the image a floating image or an inline image?
 *
 * @note            Special formatting characters in the input are not escaped.
 *                  If such behavior is needed, it must be handled by the caller.
 *                  @see gui2::rich_label for details on how this tag is parsed.
 */
std::string img(const std::string& src, const std::string& align = "left", bool floating = false);

/**
 * Generates a Help markup tag corresponding to a reference or link.
 *
 * @param text      User visible text/caption of the link.
 * @param dst       Destination of the link. Can be any string depending on the link handler
 *                  in the parsing @ref gui2::rich_label.
 *
 * @note            Special formatting characters in the input are not escaped.
 *                  If such behavior is needed, it must be handled by the caller.
 *                  @see gui2::rich_label for details on how this tag is parsed.
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

} // namespace markup
