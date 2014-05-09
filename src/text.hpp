/*
   Copyright (C) 2008 - 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef TEXT_HPP_INCLUDED
#define TEXT_HPP_INCLUDED

#include "sdl_utils.hpp"
#include "serialization/unicode.hpp"

#include <boost/noncopyable.hpp>

#include <pango/pango.h>
#include <pango/pangocairo.h>

#include <string>

struct language_def;

namespace gui2 {
	struct tpoint;
} // namespace gui2;

namespace font {

/**
 * Escapes the markup characters in a text.
 *
 * The markups escaped are the ones used in the pango markup. The special
 * characters are: @verbatim <>'"& @endverbatim
 * The escaping is the same as for HTML.
 *
 * @param text                    The text to escape.
 *
 * @returns                       The escaped text.
 */
std::string escape_text(const std::string& text);


// add background color and also font markup.

/**
 * Text class.
 *
 * This class stores the text to draw and uses pango with the cairo backend to
 * render the text. See http://pango.org for more info.
 */
class ttext
	: private boost::noncopyable
{
public:

	ttext();

	~ttext();

	/**
	 * Returns the rendered text.
	 *
	 * Before rendering it tests whether a redraw is needed and if so it first
	 * redraws the surface before returning it.
	 */
	surface render() const;

	/** Returns the width needed for the text. */
	int get_width() const;

	/** Returns the height needed for the text. */
	int get_height() const;

	/** Returns the size needed for the text. */
	gui2::tpoint get_size() const;

	/** Has the text been truncated? */
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

	/**
	 * The flags have the same values as the ones in SDL_TTF so it's easy to mix
	 * them for now. To avoid including SDL_TTF in the header they're only
	 * declared here. Once SDL_TTF is removed they can be moved in the header.
	 */

	static const unsigned STYLE_NORMAL;     /**< Normal text. */
	static const unsigned STYLE_BOLD;       /**< Bold text. */
	static const unsigned STYLE_ITALIC;     /**< Italicized text. */
	static const unsigned STYLE_UNDERLINE;  /**< Underlined text. */

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
	gui2::tpoint get_cursor_position(
		const unsigned column, const unsigned line = 0) const;

	/**
	 * Gets the column of line of the character at the position.
	 *
	 * @param position            The pixel position in the text area.
	 *
	 * @returns                   A point with the x value the column and the y
	 *                            value the line of the character found (or last
	 *                            character if not found.
	 */
	gui2::tpoint get_column_line(const gui2::tpoint& position) const;

	/**
	 * Gets the length of the text in characters.
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

	ttext& set_font_size(const unsigned font_size);

	ttext& set_font_style(const unsigned font_style);

	ttext& set_foreground_color(const Uint32 color);

	ttext& set_maximum_width(int width);

	ttext& set_characters_per_line(const unsigned characters_per_line);

	ttext& set_maximum_height(int height, bool multiline);

	ttext& set_ellipse_mode(const PangoEllipsizeMode ellipse_mode);

	ttext &set_alignment(const PangoAlignment alignment);

	ttext& set_maximum_length(const size_t maximum_length);

private:

	/***** ***** ***** *****  Pango variables ***** ***** ***** *****/
	PangoContext* context_;
	PangoLayout* layout_;
	mutable PangoRectangle rect_;

	/** The surface to render upon used as a cache. */
	mutable surface surface_;

	/** The text to draw (stored as UTF-8). */
	std::string text_;

	/** Is the text markedup if so the markedup render routines need to be used. */
	bool markedup_text_;

	/** The font size to draw. */
	unsigned font_size_;

	/** The style of the font, this is an orred mask of the font flags. */
	unsigned font_style_;

	/** The foreground color. */
	Uint32 foreground_color_;

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
	 * Recalculates the text.
	 *
	 * When the text is recalculated the surface is dirtied.
	 *
	 * @param force               Recalculate even if not dirty?
	 */
	void recalculate(const bool force = false) const;

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
	void rerender(const bool force = false) const;

	/**
	 * Buffer to store the image on.
	 *
	 * We use a cairo surface to draw on this buffer and then use the buffer as
	 * data source for the SDL_Surface. This means the buffer needs to be stored
	 * in the object.
	 */
	mutable unsigned char* surface_buffer_;

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
	bool set_markup(const std::string& text);

};

} // namespace font

#endif

