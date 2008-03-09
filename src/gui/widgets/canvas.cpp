/* $Id$ */
/*
   Copyright (C) 2007 - 2008 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file canvas.cpp
//! Implementation of canvas.hpp.

#include "gui/widgets/canvas.hpp"

#include "config.hpp"
#include "font.hpp"
#include "image.hpp"
#include "log.hpp"
#include "serialization/parser.hpp"
#include "variable.hpp"

#include <algorithm>
#include <cassert>

#define DBG_GUI LOG_STREAM(debug, widget)
#define LOG_GUI LOG_STREAM(info, widget)
#define WRN_GUI LOG_STREAM(warn, widget)
#define ERR_GUI LOG_STREAM(err, widget)

static Uint32 decode_colour(const std::string& colour);

static Uint32 decode_colour(const std::string& colour)
{
	std::vector<std::string> fields = utils::split(colour);

	// make sure we have four fields
	while(fields.size() < 4) fields.push_back("0");

	Uint32 result = 0;
	for(int i = 0; i < 4; ++i) {
		// shift the previous value before adding, since it's a nop on the
		// first run there's no need for an if.
		result = result << 8;
		result |= lexical_cast_default<int>(fields[i]);
	}

	return result;
}

namespace gui2{



tcanvas::tcanvas() :
	shapes_(),
	dirty_(true),
	w_(0),
	h_(0),
	canvas_()
{
}

tcanvas::tcanvas(const config& cfg) :
	dirty_(true),
	w_(0),
	h_(0)
{
	parse_cfg(cfg);
}

tcanvas::~tcanvas()
{
	clear_shapes();
}

void tcanvas::draw(const config& cfg)
{
	parse_cfg(cfg);
	draw(true);
}

void tcanvas::draw(const bool force)
{
	log_scope2(widget, "Drawing canvas");
	if(!dirty_ && !force) {
		DBG_GUI << "Nothing to do stop.\n";
		return;
	}

	// set surface sizes (do nothing now)
#if 0	
	if(fixme test whether -> can be used otherwise we crash w_ != canvas_->w || h_ != canvas_->h) {
		// create new

	} else {
		// fill current
	}
#endif
	// instead we overwrite the entire thing for now
	DBG_GUI << "Create canvas.\n";
	canvas_.assign(SDL_CreateRGBSurface(SDL_SWSURFACE, w_, h_, 32, 0xFF0000, 0xFF00, 0xFF, 0xFF000000));

	// draw items 
	for(std::vector<tshape*>::iterator itor = 
			shapes_.begin(); itor != shapes_.end(); ++itor) {
		log_scope2(widget, "Draw shape");
		
		(*itor)->draw(canvas_);
	}

	DBG_GUI << "Ready.\n";
	dirty_ = false;
}

void tcanvas::parse_cfg(const config& cfg)
{
	log_scope2(widget, "Parsing config");
	clear_shapes();

	for(config::all_children_iterator itor = 
			cfg.ordered_begin(); itor != cfg.ordered_end(); ++itor) {

		const std::string& type = *((*itor).first);;
		const vconfig data(&(*((*itor).second)));

		DBG_GUI << "Found type " << type << '\n';

		if(type == "line") {
			shapes_.push_back(new tline(data));
		} else if(type == "rectangle") {
			shapes_.push_back(new trectangle(data));
		} else if(type == "image") {
			shapes_.push_back(new timage(data));
		} else if(type == "text") {
			shapes_.push_back(new ttext(data));
		} else {
			ERR_GUI << "Type of shape is unknown : " << type << '\n';
			assert(false); // FIXME remove in production code.
		}
	}
}

void tcanvas::clear_shapes()
{
	for(std::vector<tshape*>::iterator itor = 
			shapes_.begin(); itor != shapes_.end(); ++itor) {
		
		delete (*itor);
	}
	shapes_.clear();
}

void tcanvas::tshape::put_pixel(unsigned start, Uint32 colour, unsigned w, unsigned x, unsigned y)
{
	// fixme the 4 is true due to Uint32..
//	DBG_GUI << "Put pixel at x " << x << " y " << y << " w " << w << '\n';
	*reinterpret_cast<Uint32*>(start + (y * w * 4) + x * 4) = colour;
}

// the surface should be locked
// the colour should be a const and the value send should already
// be good for the wanted surface
void tcanvas::tshape::draw_line(surface& canvas, Uint32 colour, 
		const int x1, int y1, const int x2, int y2)
{
	colour = SDL_MapRGBA(canvas->format, 
		((colour & 0xFF000000) >> 24),
		((colour & 0x00FF0000) >> 16),
		((colour & 0x0000FF00) >> 8),
		((colour & 0x000000FF)));

	unsigned start = reinterpret_cast<unsigned>(canvas->pixels);
	unsigned w = canvas->w;

	DBG_GUI << "Draw line from :" 
		<< x1 << ',' << y1 << " to : " << x2 << ',' << y2
		<< " canvas width " << w << " canvas height "
		<< canvas->h << '\n';

	// use a special case for vertical lines
	if(x1 == x2) {
		if(y2 < y1) {
			std::swap(y1, y2);	
		}

		for(int y = y1; y <= y2; ++y) {
			put_pixel(start, colour, w, x1, y);
		}
		return;
	} 

	// use a special case for horizontal lines
	if(y1 == y2) {
		for(int x  = x1; x <= x2; ++x) {
			put_pixel(start, colour, w, x, y1);
		}
		return;
	} 

	// draw based on Bresenham on wikipedia
	int dx = x2 - x1;
	int dy = y2 - y1;
	int slope = 1;
	if (dy < 0) {
		slope = -1;
		dy = -dy;
	}

	// Bresenham constants
	int incE = 2 * dy;
	int incNE = 2 * dy - 2 * dx;
	int d = 2 * dy - dx;
	int y = y1;

	// Blit
	for (int x = x1; x <= x2; ++x) {
		put_pixel(start, colour, w, x, y);
		if (d <= 0) {
			d += incE;
		} else {
			d += incNE;
			y += slope;
		}
	}
}


tcanvas::tline::tline(const int x1, const int y1, const int x2,
		const int y2, const Uint32 colour, const unsigned thickness) :
	x1(x1),
	y1(y1),
	x2(x2),
	y2(y2),
	colour(colour),
	thickness(thickness) 
{

}

tcanvas::tline::tline(const vconfig& cfg) :
	x1(0),
	y1(0),
	x2(0),
	y2(0),
	colour(0),
	thickness(0)
{
/*WIKI
 * [line]
 *     x1, y1 = (int = 0), (int = 0) The startpoint of the line.
 *     x2, y2 = (int = 0), (int = 0) The endpoint of the line.
 *     colour = (widget.colour = "") The colour of the line.
 *     thickness = (uint = 0)        The thickness of the line.
 *     debug = (string = "")         Debug message to show upon creation
 *                                   this message is not stored.
 * [/line]
 */

// This code can be used by a parser to generate the wiki page
// structure
// [tag name]
// param = type_info description
//
// param = key (, key)
// 
// type_info = ( type = default_value) the info about a optional parameter
// type_info = type                        info about a mandatory parameter
// type_info = [ type_infp ]               info about a conditional parameter description should explain the reason
//
// description                             description of the parameter
//
	x1 = lexical_cast_default<int>(cfg["x1"]);
	y1 = lexical_cast_default<int>(cfg["y1"]);
	x2 = lexical_cast_default<int>(cfg["x2"]);
	y2 = lexical_cast_default<int>(cfg["y2"]);
	colour = decode_colour(cfg["colour"]);
	thickness = lexical_cast_default<unsigned>(cfg["thickness"]);

	const std::string& debug = (cfg["debug"]);
	if(!debug.empty()) {
		DBG_GUI << debug << '\n';
	}

}

void tcanvas::tline::draw(surface& canvas)
{
	DBG_GUI << "Draw line from :" 
		<< x1 << ',' << y1 << " to : " << x2 << ',' << y2 << '\n';

	// we wrap around the coordinates, this might be moved to be more
	// generic place, but leave it here for now. Note the numbers are
	// negative so adding them is subtracting them.
	
	if(x1 < 0) x1 = canvas->w + x1;
	if(x2 < 0) x2 = canvas->w + x2;
	if(y1 < 0) y1 = canvas->h + y1;
	if(y2 < 0) y2 = canvas->h + y2;

	// FIXME validate the line is on the surface !!!

	// now draw the line we use Bresenham's algorithm, which doesn't
	// support antialiasing. The advantage is that it's easy for testing.
	
	// lock the surface
	surface_lock locker(canvas);
	if(x1 > x2) {
		// invert points
		draw_line(canvas, colour, x2, y2, x1, y1);
	} else {
		draw_line(canvas, colour, x1, y1, x2, y2);
	}
	
}

tcanvas::trectangle::trectangle(const vconfig& cfg) :
	rect(),
	border_thickness(0),
	border_colour(0),
	fill_colour(0)
{
/*WIKI
 * [rectangle]
 *     x, y = (int = 0), (int = 0)    The top left corner of the rectangle.
 *     w = (int = 0)                  The width of the rectangle.
 *     h = (int = 0)                  The height of the rectangle.
 *     border_thickness = (uint = 0)  The thickness of the border if the 
 *                                    thickness is zero it's not drawn.
 *     border_colour = (widget.colour = "") 
 *                                    The colour of the border if empty it's
 *                                    not drawn.
 *     fill_colour = (widget.colour = "")
 *                                    The colour of the interior if ommitted
 *                                    it's not draw (transparent is draw but
 *                                    does nothing).
 *     debug = (string = "")          Debug message to show upon creation
 *                                    this message is not stored.
 * [/rectangle]
 */

	rect.x = lexical_cast_default<int>(cfg["x"]);
	rect.y = lexical_cast_default<int>(cfg["y"]);
	rect.w = lexical_cast_default<int>(cfg["w"]);
	rect.h = lexical_cast_default<int>(cfg["h"]);

	border_thickness = lexical_cast_default<unsigned>(cfg["border_thickness"]);
	border_colour = decode_colour(cfg["border_colour"]);

	fill_colour = decode_colour(cfg["fill_colour"]);

	const std::string& debug = (cfg["debug"]);
	if(!debug.empty()) {
		DBG_GUI << debug << '\n';
	}

}

void tcanvas::trectangle::draw(surface& canvas)
{

	//FIXME wrap the points and validate the input

	surface_lock locker(canvas);

	// draw the border
	for(unsigned i = 0; i < border_thickness; ++i) {

		const unsigned left = rect.x + i;
		const unsigned right = rect.x + rect.w - 2 * i;
		const unsigned top = rect.y + i;
		const unsigned bottom = rect.y + rect.h - 2 * i;

		// top horizontal (left -> right)
		draw_line(canvas, border_colour, left, top, right, top);

		// right vertical (top -> bottom)
		draw_line(canvas, border_colour, right, top, right, bottom);

		// bottom horizontal (left -> right)
		draw_line(canvas, border_colour, left, bottom, right, bottom);

		// left vertical (top -> bottom)
		draw_line(canvas, border_colour, left, top, left, bottom);

	}

	const unsigned left = rect.x + border_thickness + 1;
	const unsigned top = rect.y + border_thickness + 1;
	const unsigned width = rect.w - (2 * border_thickness) - 2;
	const unsigned height = rect.h - (2 * border_thickness) - 2;
	SDL_Rect rect = create_rect(left, top, width, height);

	const Uint32 colour = fill_colour & 0xFFFFFF00;
	const Uint8 alpha = fill_colour & 0xFF;

	// fill
	fill_rect_alpha(rect, colour, alpha, canvas);

}


tcanvas::timage::timage(const vconfig& cfg) :
	src_clip_(),
	dst_clip_(),
	image_()
{
/*WIKI
 * [image]
 *     name = (string)                The name of the image.
 *     debug = (string = "")          Debug message to show upon creation
 * [/image]
 */

	image_.assign(image::get_image(image::locator(cfg["name"])));
	src_clip_ = create_rect(0, 0, image_->w, image_->h);

	const std::string& debug = (cfg["debug"]);
	if(!debug.empty()) {
		DBG_GUI << debug << '\n';
	}

}

void tcanvas::timage::draw(surface& canvas)
{
	SDL_Rect src_clip = src_clip_;
	SDL_Rect dst_clip = dst_clip_;
	SDL_BlitSurface(image_, &src_clip, canvas, &dst_clip);
}

tcanvas::ttext::ttext(const vconfig& cfg) :
	x_(lexical_cast_default<unsigned>(cfg["x"])),
	y_(lexical_cast_default<unsigned>(cfg["y"])),
	w_(lexical_cast_default<unsigned>(cfg["w"])),
	h_(lexical_cast_default<unsigned>(cfg["h"])),
	font_size_(lexical_cast_default<unsigned>(cfg["font_size"])),
	colour_(decode_colour(cfg["colour"])),
	text_(cfg["text"])
{	
/*WIKI
 * [text]
 *     x, y = (unsigned = 0), (unsigned = 0)    
 *                                    The top left corner of the bounding
 *                                    rectangle.
 *     w = (unsigned = 0)             The width of the bounding rectangle.
 *     h = (unsigned = 0)             The height of the bounding rectangle.
 *     font_size = (unsigned = 0)     The size of the font.
 *     colour = (widget.colour = "")  The colour of the text.
 *     text = (t_string = "")         The text to print, for now always printed
 *                                    centered in the area.
 *     debug = (string = "")          Debug message to show upon creation
 *                                    this message is not stored.
 * [/rectangle]
 */

	const std::string& debug = (cfg["debug"]);
	if(!debug.empty()) {
		DBG_GUI << debug << '\n';
	}

}

void tcanvas::ttext::draw(surface& canvas)
{
	SDL_Color col = { (colour_ >> 24), (colour_ >> 16), (colour_ >> 8), colour_ };
	surface surf(font::get_rendered_text(text_, font_size_, col, TTF_STYLE_NORMAL));

	if(surf->w > w_) {
		WRN_GUI << "Text to wide, will be clipped.\n";
	}
	
	if(surf->h > h_) {
		WRN_GUI << "Text to high, will be clipped.\n";
	}
	
	unsigned x_off = (surf->w >= w_) ? 0 : ((w_ - surf->w) / 2);
	unsigned y_off = (surf->h >= h_) ? 0 : ((h_ - surf->h) / 2);
	unsigned w_max = w_ - x_ - x_off;
	unsigned h_max = h_ - y_ - y_off;

	SDL_Rect dst = { x_ + x_off, y_ + y_off, w_max, h_max };
	SDL_BlitSurface(surf, 0, canvas, &dst);
}

} // namespace gui2
