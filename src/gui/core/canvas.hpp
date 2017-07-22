/*
   Copyright (C) 2007 - 2018 by Mark de Wever <koraq@xs4all.nl>
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

#pragma once

#include "config.hpp"
#include "formula/callable.hpp"
#include "formula/function.hpp"
#include "sdl/texture.hpp"

namespace wfl { class variant; }

namespace gui2
{

/**
 * A simple canvas which can be drawn upon.
 *
 * The class has a config which contains what to draw.
 *
 * The copy constructor does a shallow copy of the shapes to draw.
 * a clone() will be implemented if really needed.
 */
class canvas
{
public:
	/**
	 * Abstract base class for all other shapes.
	 *
	 * The other shapes are declared and defined in canvas_private.hpp, since the
	 * implementation details are not interesting for users of the canvas.
	 */
	class shape
	{
	public:
		explicit shape(const config& cfg) : immutable_(cfg["immutable"].to_bool(false))
		{
		}

		virtual ~shape()
		{
		}

		/**
		 * Draws the canvas.
		 *
		 * @param variables       The canvas can have formulas in it's
		 *                        definition, this parameter contains the values
		 *                        for these formulas.
		 */
		virtual void draw(
				const int canvas_w,
				const int canvas_h,
				SDL_Renderer* renderer,
				wfl::map_formula_callable& variables) = 0;

		bool immutable() const
		{
			return immutable_;
		}

	private:
		/**
		 * If this is true, this shape will not be removed from the canvas even if
		 * the canvas's content is reset.
		 */
		bool immutable_;
	};

	typedef std::shared_ptr<shape> shape_ptr;
	typedef std::shared_ptr<const shape> const_shape_ptr;

	canvas();
	canvas(const canvas&) = delete;
	canvas(canvas&& c);

	~canvas();

	/**
	 * Draws the canvas.
	 *
	 * @param force               If the canvas isn't dirty it isn't redrawn
	 *                            unless force is set to true.
	 */
	void draw(const bool force = false);

	/**
	 * Copies the canvas texture to the screen renderer.
	 *
	 * This will re-render the canvas texture if necessary (ie, if marked dirty).
	 * It also executes the pre-commit functions such as blurring (@todo: reenable).
	 */
	void render();

	/**
	 * Sets the config.
	 *
	 * @param cfg                 The config object with the data to draw, see
	 *                            http://www.wesnoth.org/wiki/GUICanvasWML for
	 *                            more information.
	 */
	void set_cfg(const config& cfg, const bool force = false)
	{
		clear_shapes(force);
		invalidate_cache();
		parse_cfg(cfg);
	}

	/**
	 * Appends data to the config.
	 *
	 * @param cfg                 The config object with the data to draw, see
	 *                            http://www.wesnoth.org/wiki/GUICanvasWML for
	 *                            more information.
	 */
	void append_cfg(const config& cfg)
	{
		parse_cfg(cfg);
	}

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_width(const unsigned width)
	{
		update_size(w_, width);
		invalidate_cache();
	}

	unsigned get_width() const
	{
		return w_;
	}

	void set_height(const unsigned height)
	{
		update_size(h_, height);
		invalidate_cache();
	}

	unsigned get_height() const
	{
		return h_;
	}

	void set_variable(const std::string& key, const wfl::variant& value)
	{
		variables_.add(key, value);
		set_is_dirty(true);
		invalidate_cache();
	}

	void set_is_dirty(const bool is_dirty)
	{
		is_dirty_ = is_dirty;
	}

private:
	/** Vector with the shapes to draw. */
	std::vector<shape_ptr> shapes_;

	/** All shapes which have been already drawn. Kept around in case
	 * the cache needs to be invalidated. */
	std::vector<shape_ptr> drawn_shapes_;

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

	/** The texture onto which items are drawn. */
	texture texture_;

	/** A pointer to the window renderer. */
	SDL_Renderer* renderer_;

	/** The variables of the canvas. */
	wfl::map_formula_callable variables_;

	/** Action function definitions for the canvas. */
	wfl::action_function_symbol_table functions_;

	/** The dirty state of the canvas. */
	bool is_dirty_;

	/** Whether canvas dimensions changed. */
	bool size_changed_;

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

	void clear_shapes(const bool force);

	void invalidate_cache();

	/** Small helper to handle size variable update logic. */
	void update_size(unsigned int& value, unsigned int new_value);
};

} // namespace gui2
