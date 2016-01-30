/*
   Copyright (C) 2007 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * This file contains the canvas object which is the part where the widgets
 * draw (temporally) images on.
 */

#ifndef GUI_AUXILIARY_CANVAS_HPP_INCLUDED
#define GUI_AUXILIARY_CANVAS_HPP_INCLUDED

#include "formula_callable.hpp"
#include "sdl/utils.hpp"

class config;
class variant;

namespace gui2
{

/**
 * A simple canvas which can be drawn upon.
 *
 * The class has a config which contains what to draw.
 *
 * NOTE we might add some caching in a later state, for now every draw cycle
 * does a full redraw.
 *
 * The copy constructor does a shallow copy of the shapes to draw.
 * a clone() will be implemented if really needed.
 */
class tcanvas
{
public:
	/**
	 * Abstract base class for all other shapes.
	 *
	 * The other shapes are declared and defined in canvas.cpp, since the
	 * implementation details are not interesting for users of the canvas.
	 */
	class tshape : public reference_counted_object
	{
	public:
		virtual ~tshape()
		{
		}

		/**
		 * Draws the canvas.
		 *
		 * @param canvas          The resulting image will be blitted upon this
		 *                        canvas.
		 * @param variables       The canvas can have formulas in it's
		 *                        definition, this parameter contains the values
		 *                        for these formulas.
		 */
		virtual void draw(surface& canvas,
						  const game_logic::map_formula_callable& variables)
				= 0;
	};

	typedef boost::intrusive_ptr<tshape> tshape_ptr;
	typedef boost::intrusive_ptr<const tshape> const_tshape_ptr;

	tcanvas();

	/**
	 * Draws the canvas.
	 *
	 * @param force               If the canvas isn't dirty it isn't redrawn
	 *                            unless force is set to true.
	 */
	void draw(const bool force = false);

	/**
	 * Blits the canvas unto another surface.
	 *
	 * It makes sure the image on the canvas is up to date. Also executes the
	 * pre-blitting functions.
	 *
	 * @param surf                The surface to blit upon.
	 * @param rect                The place to blit to.
	 */
	void blit(surface& surf, SDL_Rect rect);

	/**
	 * Sets the config.
	 *
	 * @param cfg                 The config object with the data to draw, see
	 *                            http://www.wesnoth.org/wiki/GUICanvasWML for
	 *                            more information.
	 */
	void set_cfg(const config& cfg)
	{
		parse_cfg(cfg);
	}

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_width(const unsigned width)
	{
		w_ = width;
		set_is_dirty(true);
	}
	unsigned get_width() const
	{
		return w_;
	}

	void set_height(const unsigned height)
	{
		h_ = height;
		set_is_dirty(true);
	}
	unsigned get_height() const
	{
		return h_;
	}

	surface& surf()
	{
		return canvas_;
	}

	void set_variable(const std::string& key, const variant& value)
	{
		variables_.add(key, value);
		set_is_dirty(true);
	}

private:
	/** Vector with the shapes to draw. */
	std::vector<tshape_ptr> shapes_;

	/**
	 * The depth of the blur to use in the pre committing.
	 *
	 * @note at the moment there's one pre commit function, namely the
	 * blurring so use a variable here, might get more functions in the
	 * future. When that happens need to evaluate whether variables are the
	 * best thing to use.
	 */
	unsigned blur_depth_;

	/** Width of the canvas. */
	unsigned w_;

	/** Height of the canvas. */
	unsigned h_;

	/** The surface we draw all items on. */
	surface canvas_;

	/** The variables of the canvas. */
	game_logic::map_formula_callable variables_;

	/** The dirty state of the canvas. */
	bool is_dirty_;

	void set_is_dirty(const bool is_dirty)
	{
		is_dirty_ = is_dirty;
	}

	/**
	 * Parses a config object.
	 *
	 * The config object is parsed and serialized by this function after which
	 * the config object is no longer required and thus not stored in the
	 * object.
	 *
	 * @param cfg                 The config object with the data to draw, see
	 *                            http://www.wesnoth.org/wiki/GUICanvasWML
	 */
	void parse_cfg(const config& cfg);
};

} // namespace gui2

#endif
