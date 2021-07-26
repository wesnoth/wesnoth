/*
	Copyright (C) 2007 - 2021
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "sdl/surface.hpp"

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
		 * @param canvas          The resulting image will be blitted upon this
		 *                        canvas.
		 * @param renderer        The SDL_Renderer to use.
		 * @param view_bounds     Part of the shape to render - this is the location of @a canvas
		 *                        within the coordinates of the shape.
		 * @param variables       The canvas can have formulas in it's
		 *                        definition, this parameter contains the values
		 *                        for these formulas.
		 */
		virtual void draw(surface& canvas, SDL_Renderer* renderer,
		                  const SDL_Rect& view_bounds,
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

	canvas();
	canvas(const canvas&) = delete;
	canvas& operator=(const canvas&) = delete;
	canvas(canvas&& c) noexcept;

	private:
	/**
	 * Internal part of the blit() function - prepares the contents of the internal viewport_
	 * surface, reallocating that surface if necessary.
	 *
	 * @param area_to_draw        Currently-visible part of the widget, any area outside here won't be blitted to the parent.
	 * @param force               If the canvas isn't dirty it isn't redrawn
	 *                            unless force is set to true.
	 */
	void draw(const SDL_Rect& area_to_draw, const bool force = false);

	public:
	/**
	 * Draw the canvas' shapes onto another surface.
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
	 * @param cfg                 The config object with the data to draw.
	 * @param force               Whether to clear all shapes or not.
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
	 * @param cfg                 The config object with the data to draw.
	 */
	void append_cfg(const config& cfg)
	{
		parse_cfg(cfg);
	}

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_width(const unsigned width)
	{
		w_ = width;
		set_is_dirty(true);
		invalidate_cache();
	}

	unsigned get_width() const
	{
		return w_;
	}

	void set_height(const unsigned height)
	{
		h_ = height;
		set_is_dirty(true);
		invalidate_cache();
	}

	unsigned get_height() const
	{
		return h_;
	}

	void set_variable(const std::string& key, wfl::variant&& value)
	{
		variables_.add(key, std::move(value));
		set_is_dirty(true);
		invalidate_cache();
	}

	void set_is_dirty(const bool is_dirty)
	{
		is_dirty_ = is_dirty;
	}

private:
	/** Vector with the shapes to draw. */
	std::vector<std::unique_ptr<shape>> shapes_;

	/**
	 * The depth of the blur to use in the pre committing.
	 *
	 * @note at the moment there's one pre commit function, namely the
	 * blurring so use a variable here, might get more functions in the
	 * future. When that happens need to evaluate whether variables are the
	 * best thing to use.
	 */
	unsigned blur_depth_;

	/** Width of the canvas (the full size, not limited to the view_bounds_). */
	unsigned w_;

	/** Height of the canvas (the full size, not limited to the view_bounds_). */
	unsigned h_;

	/** The surface we draw all items on. */
	surface viewport_;

	/**
	 * The placement and size of viewport_ in the coordinates of this widget; value is not useful when bool(viewport_) is false.
	 *
	 * For large widgets, a small viewport_ may be used that contains only the currently-visible part of the widget.
	 */
	SDL_Rect view_bounds_;

	/** The variables of the canvas. */
	wfl::map_formula_callable variables_;

	/** Action function definitions for the canvas. */
	wfl::action_function_symbol_table functions_;

	/** The dirty state of the canvas. */
	bool is_dirty_;

	/**
	 * Parses a config object.
	 *
	 * The config object is parsed and serialized by this function after which
	 * the config object is no longer required and thus not stored in the
	 * object.
	 *
	 * @param cfg                 The config object with the data to draw, see @ref GUICanvasWML
	 */
	void parse_cfg(const config& cfg);

	void clear_shapes(const bool force);

	void invalidate_cache()
	{
		viewport_ = nullptr;
	}
};

} // namespace gui2
