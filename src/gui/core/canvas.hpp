/*
	Copyright (C) 2007 - 2024
	by Mark de Wever <koraq@xs4all.nl>
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
#include "sdl/texture.hpp"
#include "sdl/rect.hpp"

namespace wfl { class variant; }
struct point;

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
		virtual void draw(wfl::map_formula_callable& variables) = 0;

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

	/**
	 * Update the background blur texture, if relevant and necessary.
	 *
	 * This should be called sometime before draw().
	 * Updating it later is less important as it's quite expensive.
	 *
	 * @param screen_region     The area of the screen underneath the canvas.
	 * @param force             Regenerate the blur even if we already did it.
	 *
	 * @returns                 True if draw should continue, false otherwise.
	 */
	bool update_blur(const rect& screen_region, const bool force = false);

	/** Clear the cached blur texture, forcing it to regenerate. */
	void queue_reblur();

	/**
	 * Draw the canvas' shapes onto the screen.
	 *
	 * It makes sure the image on the canvas is up to date. Also executes the
	 * pre-blitting functions.
	 */
	void draw();

	/**
	 * Sets the config.
	 *
	 * @param cfg                 The config object with the data to draw.
	 * @param force               Whether to clear all shapes or not.
	 */
	void set_cfg(const config& cfg, const bool force = false)
	{
		clear_shapes(force);
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

	/** Update WFL size variables. */
	void update_size_variables();

	/***** ***** ***** setters / getters for members ***** ****** *****/

	unsigned get_width() const
	{
		return w_;
	}

	unsigned get_height() const
	{
		return h_;
	}

	void set_size(const point& size);

	void set_variable(const std::string& key, wfl::variant&& value)
	{
		variables_.add(key, std::move(value));
	}

	wfl::variant get_variable(const std::string& key)
	{
		return variables_.query_value(key);
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

	/** Blurred background texture. */
	texture blur_texture_;

	/** The region of the screen we have blurred (if any). */
	rect blur_region_;

	/** Whether we have deferred rendering so we can capture for blur. */
	bool deferred_;

	/** The full width of the canvas. */
	unsigned w_;

	/** The full height of the canvas. */
	unsigned h_;

	/** The variables of the canvas. */
	wfl::map_formula_callable variables_;

	/** Action function definitions for the canvas. */
	wfl::action_function_symbol_table functions_;

	/**
	 * Parses a config object.
	 *
	 * The config object is parsed and serialized by this function after which
	 * the config object is no longer required and thus not stored in the
	 * object.
	 *
	 * @param cfg                 The config object with the data to draw
	 */
	void parse_cfg(const config& cfg);

	void clear_shapes(const bool force);
};

} // namespace gui2
