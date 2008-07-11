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

	/***** ***** ***** ***** Setters ***** ***** ***** *****/

	ttext& set_text(const t_string& text, const bool markedup);

	ttext& set_font_size(const unsigned font_size);

	ttext& set_foreground_colour(const Uint32 colour);

private:

	/***** ***** ***** *****  Pango variables ***** ***** ***** *****/
	PangoCairoFontMap* font_map_;
	PangoContext* context_;
	PangoLayout* layout_;

	/** The surface to render upon used as a cache. */
	mutable surface surface_;

	/** The text to draw. */
	t_string text_;

	/** Is the text markedup if so the markedup render routines need to be used. */
	bool markedup_text_;

	/** The font size to draw. */
	unsigned font_size_;

	/** The foreground colour. */
	Uint32 foreground_colour_;
	
	/** 
	 * When the text is rendered it's cached so we need to know the dirty
	 * status of the surface.
	 */
	bool dirty_;

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
