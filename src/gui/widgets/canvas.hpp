/* $Id: boilerplate-header.cpp 20001 2007-08-31 19:09:40Z soliton $ */
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

//! @file canvas.hpp
//! This file contains the canvas object which is the part where the widgets
//! draw (tempory) images on.

// FIXME look at SDL_gfx it might have some nice functions that same me work.

#ifndef __GUI_WIDGETS_CANVAS_HPP_INCLUDED__
#define __GUI_WIDGETS_CANVAS_HPP_INCLUDED__

#include "sdl_utils.hpp"

#include <vector>

class config;
class surface;
class vconfig;

namespace gui2 {

//! Base class for the canvas which allows drawing, a later version may implement
//! a cache which allows the same scripts with the same input to store their
//! output surface. But that will be looked into later.

// maybe inherit from surface...
class tcanvas 
{
	// FIXME write a copy constructor to copy the members
	// for now every object needs to parse the config and we 
	// need to remember not to copy things.
public:

	//! Base class for all other shapes.
	class tshape 
	{
	public:
		virtual void draw(surface& canvas) = 0;

		virtual ~tshape() {}
	protected:

		// draw basic primitives

		void put_pixel(unsigned start, Uint32 colour, unsigned w, unsigned x, unsigned y);
		void draw_line(surface& canvas, Uint32 colour, 
			const int x1, int y1, const int x2, int y2);

	};

	//! Definition of a line shape.
	class tline : public tshape
	{
	public:
		tline(const int x1, const int y1, const int x2,
			const int y2, const Uint32 colour, const unsigned thickness);
		tline(const vconfig& cfg);

		//! Implement shape::draw().
		void draw(surface& canvas);

	private:
		int x1, y1;
		int x2, y2;
		Uint32 colour;
		//! The thickness of the line:
		//! if the value is odd the x and y are the middle of the line.
		//! if the value is even the x and y are the middle of a line
		//! with width - 1. (0 is special case, does nothing.)
		unsigned thickness;
	};

	//! Definition of a rectangle shape.
	class trectangle : public tshape
	{
	public:
		trectangle(const vconfig& cfg);

		//! Implement shape::draw().
		void draw(surface& canvas);

	private:
		SDL_Rect rect;

		//! Border thickness if 0 the fill colour is used for the entire 
		//! widget.
		unsigned border_thickness;
		Uint32 border_colour;

		Uint32 fill_colour;
	};

	//! Definition of an image shape.
	class timage : public tshape
	{
	public:
		timage(const vconfig& cfg);
		
		//! Implement shape::draw().
		void draw(surface& canvas);
	private:
		SDL_Rect src_clip_;
		SDL_Rect dst_clip_;
		surface image_;
	};

	tcanvas();
	tcanvas(const config& cfg);
	~tcanvas();

	void draw(const config& cfg);
	void draw(const bool force = false);

	void set_width(const int width) { w_ = width; set_dirty(); }
	int get_width() const { return w_; }

	void set_height(const int height) { h_ = height; set_dirty(); }
	int get_height() const { return h_; }

	surface& surf() { return canvas_; }

	void set_cfg(const config& cfg) { parse_cfg(cfg); }

private:
	void set_dirty(const bool dirty = true) { dirty_ = dirty; }

	void parse_cfg(const config& cfg);
	void clear_shapes();

	std::vector<tshape*> shapes_;

	bool dirty_;
	unsigned w_;
	unsigned h_;

	surface canvas_;
};

} // namespace gui2


#endif
