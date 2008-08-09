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

#include "text.hpp"

#include "gui/widgets/helper.hpp"
#include "font.hpp"
#include "language.hpp"

#include <cassert>
#include <cstring>

#include <cairo.h>

namespace font {

ttext::ttext() :
	font_map_(reinterpret_cast<PangoCairoFontMap*>(pango_cairo_font_map_new())),
	context_(pango_cairo_font_map_create_context(font_map_)),
	layout_(pango_layout_new(context_)),
	surface_(),
	text_(),
	markedup_text_(false),
	font_size_(14),
	foreground_colour_(0xFFFFFFFF), // solid white
	maximum_width_(-1),
	maximum_height_(-1),
	word_wrap_(false),
	dirty_(true),
	surface_buffer_(NULL)
{	
	// With 72 dpi the sizes are the same as with SDL_TTF so hardcoded.
	pango_cairo_context_set_resolution(context_, 72.0);
}

ttext::~ttext()
{
	if(font_map_) {
		g_object_unref(font_map_);
	}
	if(context_) {
		g_object_unref(context_);
	}
	if(layout_) {
		g_object_unref(layout_);
	}
	if(surface_buffer_) {
		surface_.assign(NULL);
		delete[] surface_buffer_;
	}
}

surface ttext::render()
{
	if(dirty_) {
		dirty_ = false;

		/**
		 * @todo FIXME the font needs to be an object.
		 */
		PangoFontDescription *font = pango_font_description_new(); 
		pango_font_description_set_family(font, get_fonts().c_str());
		pango_font_description_set_size(font, font_size_ * PANGO_SCALE);
		pango_layout_set_font_description(layout_, font);
		pango_font_description_free(font);

		// NOTE for now the setting of the ellipse is undocumented and
		// implicitly done, this will change later. We'll need it for the
		// textboxes.
		pango_layout_set_ellipsize(layout_, 
			maximum_width_ == -1 && maximum_height_ == -1 
			? PANGO_ELLIPSIZE_NONE : PANGO_ELLIPSIZE_END);

		if(markedup_text_) {
			pango_layout_set_markup(layout_, text_.c_str(), text_.size());
		} else {
			pango_layout_set_text(layout_, text_.c_str(), text_.size());
		}

		PangoRectangle rect;
		pango_layout_get_pixel_extents(layout_, NULL, &rect);

		const unsigned stride = rect.width * 4;
		create_surface_buffer(stride * rect.height);

		cairo_surface_t *cairo_surface =
			cairo_image_surface_create_for_data(surface_buffer_,
				CAIRO_FORMAT_ARGB32, rect.width, rect.height, stride);
		cairo_t *cr = cairo_create(cairo_surface);

		pango_cairo_update_context (cr, context_); // Needed?
         
		/* paint background */
//		cairo_set_source_rgb(cr, 0.0, 0.0, 0.0); 
//		cairo_paint(cr);

		/* set colour (used for foreground). */
		cairo_set_source_rgba(cr, 
			 (foreground_colour_ >> 24)         / 256.0,
			((foreground_colour_ >> 16) & 0xFF) / 256.0,
			((foreground_colour_ >> 8)  & 0xFF) / 256.0,
			(foreground_colour_         & 0xFF) / 256.0);

		pango_cairo_show_layout(cr, layout_);

		// Draw twice otherwise we have some problems due to transparency
		// we overcome the problem with drawing twice which is a kind of hack.
		pango_cairo_show_layout(cr, layout_);

		surface_.assign(SDL_CreateRGBSurfaceFrom(
			surface_buffer_, rect.width, rect.height, 32, stride, 
			0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000));

	}

	return surface_;
}

int ttext::get_width() 
{
	return get_size().x;
}

int ttext::get_height() 
{
	return get_size().y;
}

gui2::tpoint ttext::get_size() 
{
	if(dirty_) {
		/**
		 * @todo We might use two dirty flags one for the layout and one for
		 * the rendering, but for now only use one.
		 */
		render();
	}

	return gui2::tpoint(surface_->w, surface_->h);
}

ttext& ttext::set_text(const t_string& text, const bool markedup) 
{
	if(markedup != markedup_text_ || text != text_) {
		text_ = text; 
		markedup_text_ = markedup;
		dirty_ = true;
	}

	return *this;
}

ttext& ttext::set_font_size(const unsigned font_size)
{
	if(font_size != font_size_) {
		font_size_ = font_size;
		dirty_ = true;
	}

	return *this;
}

ttext& ttext::set_foreground_colour(const Uint32 colour)
{
	if(colour != foreground_colour_) {
		foreground_colour_ = colour;
		dirty_ = true;
	}

	return *this;
}

ttext& ttext::set_maximum_width(const int width)
{
	if(width != maximum_width_) {
		pango_layout_set_width(layout_, width);
		maximum_width_ = width;
		dirty_ = true;
	}

	return *this;
}

ttext& ttext::set_maximum_height(const int height)
{
	if(height != maximum_height_) {
		pango_layout_set_height(layout_, height);
		maximum_height_ = height;
		dirty_ = true;
	}

	return *this;
}

ttext& ttext::set_word_wrap(const bool wrap)
{
	if(wrap != word_wrap_) {
		pango_layout_set_wrap(layout_, wrap 
			? PANGO_WRAP_WORD_CHAR : static_cast<PangoWrapMode>(-1));
		word_wrap_ = wrap;
		dirty_ = true;
	}

	return *this;
}

void ttext::create_surface_buffer(const size_t size)
{
	// clear old buffer
	if(surface_buffer_) {
		surface_.assign(NULL);
		delete[] surface_buffer_;
	}

	surface_buffer_ = new unsigned char [size];
	memset(surface_buffer_, 0, size);
}

} // namespace font 

