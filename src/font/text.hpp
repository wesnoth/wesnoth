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

#pragma once

#include "font/font_options.hpp"
#include "color.hpp"
#include "sdl/surface.hpp"
#include "sdl/texture.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode_types.hpp"

#include <pango/pango.h>
#include <pango/pangocairo.h>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

/**
 * Note: This is the cairo-pango code path, not the SDL_TTF code path.
 */

struct language_def;

namespace gui2
{
	struct point;
} // namespace gui2;

namespace font
{
// add background color and also font markup.

/**
 * Text class.
 *
 * This class represents text which is rendered using Pango.
 *
 * It takes text, as a utf-8 std::string, plus formatting options including
 * font and color. It provides a surface object which holds the rendered text.
 *
 * Besides this, it can do some additional calculations using the font layout.
 *
 * It can take an index into the text, and convert it to pixel coordinates,
 * so that if we want to draw a cursor in an editbox, we know where to draw it.
 *
 * It can also take a pixel coordinate with respect to the text layout, and
 * translate it back to an index into the original text. This is useful if the
 * user clicks on the text, and we want to know where to move the cursor.
 *
 * The get_token method takes a pixel coordinate, which we assume represents a
 * click position, and gets the corresponding "token" from the string. The default
 * token delimiters are whitespace " \n\r\t". So, this returns the "word" that the
 * user clicked on.
 *
 * Finally, the get_link method represents special support for hyperlinks in text.
 * A token "looks like a link" if it begins "http://" or "https://".
 * If a text has link_aware enabled, then any such token is rendered with an
 * underline and in a special color, see `link_color`.
 * The get_link method calls get_token and further checks if the clicked token
 * looks like a link.
 *
 * This class stores the text to draw and uses pango with the cairo backend to
 * render the text. See http://pango.org for more info.
 *
 */
class pango_text
{
public:

	pango_text();

    pango_text(const pango_text &) = delete;
    pango_text & operator = (const pango_text &) = delete;

	/**
	 * Returns the rendered text texture from the cache.
	 *
	 * If the surface is flagged dirty it will first be re-rendered and a new
	 * texture added to the cache upon redraw.
	 */
	texture& render_and_get_texture();

	/**
	 * Returns the rendered text surface directly.
	 *
	 * If the surface is flagged dirty it will first be re-rendered and a new
	 * texture added to the cache upon redraw.
	 */
	surface& render_and_get_surface();

	/** Returns the width needed for the text. */
	int get_width() const;

	/** Returns the height needed for the text. */
	int get_height() const;

	/** Returns the pixel size needed for the text. */
	gui2::point get_size() const;

	/** Has the text been truncated? This happens if it exceeds max width or height. */
	bool is_truncated() const;

	/**
	 * Inserts UTF-8 text.
	 *
	 * @param offset              The position to insert the text.
	 * @param text                The UTF-8 text to insert.
	 *
	 * @returns                   The number of characters inserted.
	 */
	unsigned insert_text(const unsigned offset, const std::string& text);

	/**
	 * Inserts a unicode char.
	 *
	 * @param offset              The position to insert the char.
	 * @param unicode             The character to insert.
	 *
	 * @returns                   True upon success, false otherwise.
	 */
	bool insert_unicode(const unsigned offset, ucs4::char_t unicode);

	/**
	 * Inserts unicode text.
	 *
	 * @param offset              The position to insert the text.
	 * @param unicode             Vector with characters to insert.
	 *
	 * @returns                   The number of characters inserted.
	 */
	unsigned insert_unicode(
		const unsigned offset, const ucs4::string& unicode);

	/***** ***** ***** ***** Font flags ***** ***** ***** *****/

	// NOTE: these values must be powers of 2 in order to be bit-unique
	enum FONT_STYLE {
		STYLE_NORMAL = 0,
		STYLE_BOLD = 1,
		STYLE_ITALIC = 2,
		STYLE_UNDERLINE = 4,
		STYLE_LIGHT = 8,
	};

	/***** ***** ***** ***** Query details ***** ***** ***** *****/

	/**
	 * Gets the location for the cursor.
	 *
	 * @param column              The column offset of the cursor.
	 * @param line                The line offset of the cursor.
	 *
	 * @returns                   The position of the top of the cursor. It the
	 *                            requested location is out of range 0,0 is
	 *                            returned.
	 */
	gui2::point get_cursor_position(
		const unsigned column, const unsigned line = 0) const;

	/**
	 * Gets the largest collection of characters, including the token at position,
	 * and not including any characters from the delimiters set.
	 *
	 * @param position            The pixel position in the text area.
	 *
	 * @returns                   The token containing position, and none of the
	 * 			      delimiter characters. If position is out of bounds,
	 *			      it returns the empty string.
	 */
	std::string get_token(const gui2::point & position, const char * delimiters = " \n\r\t") const;

	/**
	 * Checks if position points to a character in a link in the text, returns it
	 * if so, empty string otherwise. Link-awareness must be enabled to get results.
	 * @param position            The pixel position in the text area.
	 *
	 * @returns                   The link if one is found, the empty string otherwise.
	 */
	std::string get_link(const gui2::point & position) const;

	/**
	 * Gets the column of line of the character at the position.
	 *
	 * @param position            The pixel position in the text area.
	 *
	 * @returns                   A point with the x value the column and the y
	 *                            value the line of the character found (or last
	 *                            character if not found.
	 */
	gui2::point get_column_line(const gui2::point& position) const;

	/**
	 * Gets the length of the text in bytes.
	 *
	 * The text set is UTF-8 so the length of the string might not be the length
	 * of the text.
	 */
	size_t get_length() const { return length_; }

	/**
	 * Sets the text to render.
	 *
	 * @param text                The text to render.
	 * @param markedup            Should the text be rendered with pango
	 *                            markup. If the markup is invalid it's
	 *                            rendered as text without markup.
	 *
	 * @returns                   The status, if rendered as markup and the
	 *                            markup contains errors, false is returned
	 *                            else true.
	 */
	bool set_text(const std::string& text, const bool markedup);

	/***** ***** ***** ***** Setters / getters ***** ***** ***** *****/

	const std::string& text() const { return text_; }

	pango_text& set_family_class(font::family_class fclass);

	pango_text& set_font_size(const unsigned font_size);

	pango_text& set_font_style(const FONT_STYLE font_style);

	pango_text& set_foreground_color(const color_t& color);

	pango_text& set_maximum_width(int width);

	pango_text& set_characters_per_line(const unsigned characters_per_line);

	pango_text& set_maximum_height(int height, bool multiline);

	pango_text& set_ellipse_mode(const PangoEllipsizeMode ellipse_mode);

	pango_text& set_alignment(const PangoAlignment alignment);

	pango_text& set_maximum_length(const size_t maximum_length);

	bool link_aware() const { return link_aware_; }

	pango_text& set_link_aware(bool b);

	pango_text& set_link_color(const color_t& color);
private:

	/***** ***** ***** *****  Pango variables ***** ***** ***** *****/
	std::unique_ptr<PangoContext, std::function<void(void*)>> context_;
	std::unique_ptr<PangoLayout, std::function<void(void*)>> layout_;
	mutable PangoRectangle rect_;

	// Used if the text is too long to fit into a single Cairo surface.
	std::vector<std::unique_ptr<PangoLayout, std::function<void(void*)>>> sublayouts_;

	/** The SDL surface to render upon used as a cache. */
	mutable surface surface_;


	/** The text to draw (stored as UTF-8). */
	std::string text_;

	/** Does the text contain pango markup? If different render routines must be used. */
	bool markedup_text_;

	/** Are hyperlinks in the text marked-up, and will get_link return them. */
	bool link_aware_;

	/**
     * The color to render links in.
     *
     * Links are formatted using pango <span> as follows:
     *
     * "<span underline=\'single\' color=\'" + link_color_ + "\'>"
     */
	color_t link_color_;

	/** The font family class used. */
	font::family_class font_class_;

	/** The font size to draw. */
	unsigned font_size_;

	/** The style of the font, this is an orred mask of the font flags. */
	FONT_STYLE font_style_;

	/** The foreground color. */
	color_t foreground_color_;

	/**
	 * The maximum width of the text.
	 *
	 * Values less or equal to 0 mean no maximum and are internally stored as
	 * -1, since that's the value pango uses for it.
	 *
	 * See @ref characters_per_line_.
	 */
	int maximum_width_;

	/**
	 * The number of characters per line.
	 *
	 * This can be used as an alternative of @ref maximum_width_. The user can
	 * select a number of characters on a line for wrapping. When the value is
	 * non-zero it determines the maximum width based on the average character
	 * width.
	 *
	 * If both @ref maximum_width_ and @ref characters_per_line_ are set the
	 * minimum of the two will be the maximum.
	 *
	 * @note Long lines are often harder to read, setting this value can
	 * automatically wrap on a number of characters regardless of the font
	 * size. Often 66 characters is considered the optimal value for a one
	 * column text.
	 */
	unsigned characters_per_line_;

	/**
	 * The maximum height of the text.
	 *
	 * Values less or equal to 0 mean no maximum and are internally stored as
	 * -1, since that's the value pango uses for it.
	 */
	int maximum_height_;

	/** The way too long text is shown depends on this mode. */
	PangoEllipsizeMode ellipse_mode_;

	/** The alignment of the text. */
	PangoAlignment alignment_;

	/** The maximum length of the text. */
	size_t maximum_length_;

	/**
	 * The text has two dirty states:
	 * - The setting of the state and the size calculations.
	 * - The rendering of the surface.
	 */

	/** The dirty state of the calculations. */
	mutable bool calculation_dirty_;

	/** Length of the text. */
	mutable size_t length_;

	/**
	 * Recalculates the text layout.
	 *
	 * When the text is recalculated the surface is dirtied.
	 *
	 * @param force               Recalculate even if not dirty?
	 */
	void recalculate(const bool force = false) const;

	/** Calculates surface size. */
	PangoRectangle calculate_size(PangoLayout& layout) const;

	/** The dirty state of the surface. */
	mutable bool surface_dirty_;

	/**
	 * Renders the text.
	 *
	 * It will do a recalculation first so no need to call both.
	 *
	 * @param force               Render even if not dirty? This parameter is
	 *                            also send to recalculate().
	 */
	void rerender(const bool force = false);

	void render(PangoLayout& layout, const PangoRectangle& rect,
		const size_t surface_buffer_offset, const unsigned stride);

	/**
	 * Buffer to store the image on.
	 *
	 * We use a cairo surface to draw on this buffer and then use the buffer as
	 * data source for the SDL_Surface. This means the buffer needs to be stored
	 * in the object, since SDL_Surface doesn't own its buffer.
	 */
	mutable std::vector<unsigned char> surface_buffer_;

	/**
	 * Creates a new buffer.
	 *
	 * If needed frees the other surface and then creates a new buffer and
	 * initializes the entire buffer with values 0.
	 *
	 * NOTE eventhough we're clearly modifying function we don't change the
	 * state of the object. The const is needed so other functions can also be
	 * marked const (those also don't change the state of the object.
	 *
	 * @param size                The required size of the buffer.
	 */
	void create_surface_buffer(const size_t size) const;

	/**
	 * Sets the markup'ed text.
	 *
	 * It tries to set the text as markup. If the markup is invalid it will try
	 * a bit harder to recover from the errors and still set the markup.
	 *
	 * @param text                The text to set as markup.
	 *
	 * @returns                   Whether the markup was set or an
	 *                            unrecoverable error occurred and the text is
	 *                            set as plain text with an error message.
	 */
	bool set_markup(utils::string_view text, PangoLayout& layout);

	bool set_markup_helper(utils::string_view text, PangoLayout& layout);

	/** Splits the text to two Cairo surfaces.
	 *
	 * The implementation isn't recursive: the function only splits the text once.
	 * As a result, it only doubles the maximum surface height to 64,000 pixels
	 * or so.
	 * The reason for this is that a recursive implementation would be more complex
	 * and it's unnecessary for now, as the longest surface in the game
	 * (end credits) is only about 40,000 pixels high with the default_large widget
	 * definition.
	 * If we need even larger surfaces in the future, the implementation can be made
	 * recursive.
	 */
	void split_surface();

	bool is_surface_split() const
	{
		return sublayouts_.size() > 0;
	}

	static void copy_layout_properties(PangoLayout& src, PangoLayout& dst);

	std::string format_link_tokens(const std::string & text) const;

	std::string handle_token(const std::string & token) const;

	/** Hash for the current settings (text, size, etc) configuration. */
	size_t hash_;

	// Allow specialization of std::hash for pango_text
	friend struct std::hash<pango_text>;
};

/**
 * Returns a reference to a static pango_text object.
 *
 * Since the class is essentially a render pipeline, there's no need for individual
 * areas of the game to own their own renderers. Not to mention it isn't a trivial
 * class; constructing one is likely to be expensive.
 */
pango_text& get_text_renderer();

using pango_text_cache_t = std::map<size_t, texture>;

/**
 * The text texture cache.
 *
 * Each time a specific bit of text is rendered, a corresponding texture is created and
 * added to the cache. We don't store the surface since there isn't really any use for
 * it. If we need texture size that can be easily queried.
 *
 * @todo Figure out how this can be optimized with a texture atlas. It should be possible
 * to store smaller bits of text in the atlas and construct new textures from them.
 */
static pango_text_cache_t rendered_text_cache;

} // namespace font

// Specialize std::hash for pango_text
namespace std
{
template<>
struct hash<font::pango_text>
{
	size_t operator()(const font::pango_text& t) const;
};

} // namespace std
