/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef TEXT_HPP_INCLUDED
#define TEXT_HPP_INCLUDED

#include "sdl_utils.hpp"
#include "tstring.hpp"

#include <pango/pango.h>
#include <pango/pangocairo.h>

class language_def;

namespace gui2 {
	class tpoint;
} // namespace gui2;

namespace font {

// add background colour and also font markup.

/**
 * Text class .
 *
 * This class stores the text to draw and uses pango with the cairo backend to
 * render the text. See http://pango.org for more info.
 */
class ttext {
public:

	ttext();

	~ttext();

	/** 
	 * Returns the rendered text.
	 *
	 * Before rendering it tests whether a redraw is needed and if so it first
	 * redraws the surface before returning it.
	 */
	surface render();

	/** Returns the width needed for the text. */
	int get_width() ;

	/** Returns the height needed for the text. */
	int get_height() ;

	/** Returns the size needed for the text. */
	gui2::tpoint get_size() ;

	/** Has the text been truncated? */
	bool is_truncated();

	/***** ***** ***** ***** Font flags ***** ***** ***** *****/

	/**
	 * The flags have the same values as the ones in SDL_TTF so it's easy to mix
	 * them for now. To avoid including SDL_TTF in the header they'r only
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
		const unsigned column, const unsigned line = 0);

	/**
	 * Gets the column of line of the character at the position.
	 *
	 * @param position            The pixel position in the text area.
	 *
	 * @returns                   A point with the x value the column and the y
	 *                            value the line of the character found (or last
	 *                            character if not found.
	 */
	gui2::tpoint get_column_line(const gui2::tpoint& position);

	/**
	 * Gets the length of the text in characters. 
	 *
	 * The text set is utf-8 so the length of the string might not be the length
	 * of the text.
	 */
	size_t get_length() const;

	/***** ***** ***** ***** Setters ***** ***** ***** *****/

	ttext& set_text(const t_string& text, const bool markedup);

	ttext& set_font_size(const unsigned font_size);

	ttext& set_font_style(const unsigned font_style);

	ttext& set_foreground_colour(const Uint32 colour);

	ttext& set_maximum_width(const int width);

	ttext& set_maximum_height(const int height);

	ttext& set_ellipse_mode(const PangoEllipsizeMode ellipse_mode);

private:

	/***** ***** ***** *****  Pango variables ***** ***** ***** *****/
	PangoContext* context_;
	PangoLayout* layout_;
	PangoRectangle rect_;

	/** The surface to render upon used as a cache. */
	mutable surface surface_;

	/** The text to draw (stored as utf-8). */
	t_string text_;

	/** Is the text markedup if so the markedup render routines need to be used. */
	bool markedup_text_;

	/** The font size to draw. */
	unsigned font_size_;

	/** The style of the font, this is an orred mask of the font flags. */
	unsigned font_style_;

	/** The foreground colour. */
	Uint32 foreground_colour_;

	/** 
	 * The maximum width of the text, -1 means no maximum. */
	int maximum_width_;

	/** The maximum height of the text, -1 means no maximum . */
	int maximum_height_;

	/** The way too long text is shown depends on this mode. */
	PangoEllipsizeMode ellipse_mode_;

	/** 
	 * The text has two dirty states:
	 * - The setting of the state and the size calculations.
	 * - The rendering of the surface.
	 */

	/** The dirty state of the calculations. */
	bool calculation_dirty_;

	/**
	 * Recalculates the text.
	 *
	 * When the text is recalculated the surface is dirtied.
	 *
	 * @param force               Recalculate even if not dirty?
	 */
	void recalculate(const bool force = false);

	/** The dirty state of the surface. */
	bool surface_dirty_;

	/**
	 * Renders the text.
	 *
	 * It will do a recalculation first so no need to call both.
	 *
	 * @param force               Render even if not dirty? This parameter is
	 *                            also send to recalculate().
	 */
	void rerender(const bool force = false);

	/** 
	 * Buffer to store the image on.
	 *
	 * We use a cairo surface to draw on this buffer and then use the buffer as
	 * data source for the SDL_Surface. This means the buffer needs to be stored
	 * in the object.
	 */
	unsigned char* surface_buffer_;
	
	/**
	 * Creates a new buffer.
	 *
	 * If needed frees the other surface and then creates a new buffer and
	 * initializes the entire buffer with values 0.
	 *
	 * @param size                The required size of the buffer.
	 */
	void create_surface_buffer(const size_t size); 
};

} // namespace font 

#endif
