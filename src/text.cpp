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

#include "SDL_ttf.h"

#include <cassert>
#include <cstring>

#include <cairo.h>

namespace font {

const unsigned ttext::STYLE_NORMAL = TTF_STYLE_NORMAL;
const unsigned ttext::STYLE_BOLD = TTF_STYLE_BOLD;
const unsigned ttext::STYLE_ITALIC = TTF_STYLE_ITALIC;
const unsigned ttext::STYLE_UNDERLINE = TTF_STYLE_UNDERLINE;

ttext::ttext() :
	font_map_(reinterpret_cast<PangoCairoFontMap*>(pango_cairo_font_map_new())),
	context_(pango_cairo_font_map_create_context(font_map_)),
	layout_(pango_layout_new(context_)),
	rect_(),
	surface_(),
	text_(),
	markedup_text_(false),
	font_size_(14),
	font_style_(STYLE_NORMAL),
	foreground_colour_(0xFFFFFFFF), // solid white
	maximum_width_(-1),
	maximum_height_(-1),
	calculation_dirty_(true),
	surface_dirty_(true),
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
	rerender();
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
	recalculate();

	return gui2::tpoint(rect_.width, rect_.height);
}

bool ttext::is_truncated()
{
	recalculate();

	return (pango_layout_is_ellipsized(layout_) == TRUE);
}	

ttext& ttext::set_text(const t_string& text, const bool markedup) 
{
	if(markedup != markedup_text_ || text != text_) {
		if(markedup) {
			pango_layout_set_markup(layout_, text.c_str(), text.size());
		} else {
			pango_layout_set_text(layout_, text.c_str(), text.size());
		}
		text_ = text; 
		markedup_text_ = markedup;
		calculation_dirty_ = true;
		surface_dirty_ = true;
	}

	return *this;
}

ttext& ttext::set_font_size(const unsigned font_size)
{
	if(font_size != font_size_) {
		font_size_ = font_size;
		calculation_dirty_ = true;
		surface_dirty_ = true;
	}

	return *this;
}

ttext& ttext::set_font_style(const unsigned font_style)
{
	if(font_style != font_style_) {
		font_style_ = font_style;
		calculation_dirty_ = true;
		surface_dirty_ = true;
	}

	return *this;
}

ttext& ttext::set_foreground_colour(const Uint32 colour)
{
	if(colour != foreground_colour_) {
		foreground_colour_ = colour;
		surface_dirty_ = true;
	}

	return *this;
}

ttext& ttext::set_maximum_width(const int width)
{
	if(width != maximum_width_) {
		pango_layout_set_width(layout_, width == -1 ? -1 : width * PANGO_SCALE);
		maximum_width_ = width;
		calculation_dirty_ = true;
		surface_dirty_ = true;
	}

	return *this;
}

ttext& ttext::set_maximum_height(const int height)
{
	if(height != maximum_height_) {
/**
 * @todo See whether we can make pango 1.20 mandatory before Wesnoth 1.6 is
 * released.
 */
#if defined(PANGO_VERSION_CHECK) && PANGO_VERSION_CHECK(1,20,0)
		pango_layout_set_height(layout_, height == -1 ? -1 : height * PANGO_SCALE);
#endif
		maximum_height_ = height;
		calculation_dirty_ = true;
		surface_dirty_ = true;
	}

	return *this;
}

namespace {

/** Small helper class to make sure the font object is destroyed properly. */
class tfont
{
	tfont(const tfont&);
	tfont& operator=(const tfont&);
public:
	tfont(const std::string& name, const unsigned size, const unsigned style) :
		font_(pango_font_description_new())
	{
		pango_font_description_set_family(font_, name.c_str());
		pango_font_description_set_size(font_, size * PANGO_SCALE);

		if(style != ttext::STYLE_NORMAL) {
			if(style & ttext::STYLE_ITALIC) {
				pango_font_description_set_style(font_, PANGO_STYLE_ITALIC);
			}
			if(style & ttext::STYLE_BOLD) {
				pango_font_description_set_weight(font_, PANGO_WEIGHT_BOLD);
			}
			if(style & ttext::STYLE_UNDERLINE) {
				assert(false); // Not implemented yet
			}
		}
	}

	~tfont() { pango_font_description_free(font_); }

	PangoFontDescription* get() { return font_; }

private:
	PangoFontDescription *font_;
};

} // namespace

void ttext::recalculate(const bool force)
{
	if(calculation_dirty_ || force) {
		calculation_dirty_ = false;
		surface_dirty_ = true;

		tfont font(get_fonts(), font_size_, font_style_);
		pango_layout_set_font_description(layout_, font.get());

		// NOTE for now the setting of the ellipse is undocumented and
		// implicitly done, this will change later. We'll need it for the
		// textboxes.
		pango_layout_set_ellipsize(layout_, 
			maximum_width_ == -1 && maximum_height_ == -1 
			? PANGO_ELLIPSIZE_NONE : PANGO_ELLIPSIZE_END);

		pango_layout_get_pixel_extents(layout_, NULL, &rect_);
	}
}

void ttext::rerender(const bool force) 
{
	if(surface_dirty_ || force) {
		recalculate(force);
		surface_dirty_ = false;

		const unsigned stride = rect_.width * 4;
		create_surface_buffer(stride * rect_.height);

		cairo_surface_t *cairo_surface =
			cairo_image_surface_create_for_data(surface_buffer_,
				CAIRO_FORMAT_ARGB32, rect_.width, rect_.height, stride);
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
			surface_buffer_, rect_.width, rect_.height, 32, stride, 
			0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000));
	}
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

