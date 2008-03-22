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

//! @file canvas.hpp
//! This file contains the canvas object which is the part where the widgets
//! draw (tempory) images on.

// FIXME look at SDL_gfx it might have some nice functions that same me work.

#ifndef __GUI_WIDGETS_CANVAS_HPP_INCLUDED__
#define __GUI_WIDGETS_CANVAS_HPP_INCLUDED__

#include "formula_callable.hpp"
#include "reference_counted_object.hpp"
#include "sdl_utils.hpp"
#include "tstring.hpp"
#include "variant.hpp"

#include <vector>

class config;
class surface;
class vconfig;

namespace gui2 {

//! Base class for the canvas which allows drawing, a later version may implement
//! a cache which allows the same scripts with the same input to store their
//! output surface. But that will be looked into later.


//! The copy constructor does a shallow copy of the shapes to draw.
//! a clone() will be implemented if really needed.

// maybe inherit from surface...
class tcanvas 
{
public:

	//! Base class for all other shapes.
	class tshape : public reference_counted_object
	{
	public:
		virtual void draw(surface& canvas, 
			const game_logic::map_formula_callable& variables) = 0;

		virtual ~tshape() {}
	protected:

		// draw basic primitives

		void put_pixel(unsigned start, Uint32 colour, unsigned w, unsigned x, unsigned y);
		void draw_line(surface& canvas, Uint32 colour, 
			const unsigned x1, unsigned y1, const unsigned x2, unsigned y2);

	};

	typedef boost::intrusive_ptr<tshape> tshape_ptr;
	typedef boost::intrusive_ptr<const tshape> const_tshape_ptr;

	//! Definition of a line shape.
	class tline : public tshape
	{
	public:
		tline(const vconfig& cfg);

		//! Implement shape::draw().
		void draw(surface& canvas,
			const game_logic::map_formula_callable& variables);

	private:
		unsigned x1_, y1_;
		unsigned x2_, y2_;

		std::string
			x1_formula_,
			y1_formula_,
			x2_formula_,
			y2_formula_;

		Uint32 colour_;
		//! The thickness of the line:
		//! if the value is odd the x and y are the middle of the line.
		//! if the value is even the x and y are the middle of a line
		//! with width - 1. (0 is special case, does nothing.)
		unsigned thickness_;
	};

	//! Definition of a rectangle shape.
	class trectangle : public tshape
	{
	public:
		trectangle(const vconfig& cfg);

		//! Implement shape::draw().
		void draw(surface& canvas,
			const game_logic::map_formula_callable& variables);

	private:
		unsigned x_, y_;
		unsigned w_, h_;
	
		std::string
			x_formula_,
			y_formula_,
			w_formula_,
			h_formula_;

		//! Border thickness if 0 the fill colour is used for the entire 
		//! widget.
		unsigned border_thickness_;
		Uint32 border_colour_;

		Uint32 fill_colour_;
	};

	//! Definition of an image shape.
	class timage : public tshape
	{
	public:
		timage(const vconfig& cfg);
		
		//! Implement shape::draw().
		void draw(surface& canvas,
			const game_logic::map_formula_callable& variables);

	private:
		SDL_Rect src_clip_;
		SDL_Rect dst_clip_;
		surface image_;
	};

	//! Definition of a text shape.
	class ttext : public tshape
	{
	public:
		ttext(const vconfig& cfg);
		
		//! Implement shape::draw().
		void draw(surface& canvas,
			const game_logic::map_formula_callable& variables);

	private:
		unsigned x_, y_;
		unsigned w_, h_;

		std::string
			x_formula_,
			y_formula_,
			w_formula_,
			h_formula_;

		unsigned font_size_;
		Uint32 colour_;
		t_string text_;
		std::string text_formula_; 
	};

	tcanvas();
	tcanvas(const config& cfg);

	void draw(const config& cfg);
	void draw(const bool force = false);

	void set_width(const unsigned width) { w_ = width; set_dirty(); }
	unsigned get_width() const { return w_; }

	void set_height(const unsigned height) { h_ = height; set_dirty(); }
	unsigned get_height() const { return h_; }

	surface& surf() { return canvas_; }

	void set_cfg(const config& cfg) { parse_cfg(cfg); }

	void set_variable(const std::string& key, const variant& value)
		{ variables_.add(key, value); }

private:
	void set_dirty(const bool dirty = true) { dirty_ = dirty; }

	void parse_cfg(const config& cfg);

	std::vector<tshape_ptr> shapes_;

	bool dirty_;
	unsigned w_;
	unsigned h_;

	surface canvas_;

	game_logic::map_formula_callable variables_;
};

} // namespace gui2


#endif
