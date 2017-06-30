/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "map/location.hpp"
#include "sdl/texture.hpp"

#include <SDL_rect.h>

#include <array>
#include <list>
#include <vector>

class drawing_queue
{
public:
	drawing_queue()
		: buffer_()
	{
	}

	/** Draws the contents of the buffer to screen and clears it. */
	void render_buffer();

	/** Adds a new item to the buffer. */
	template<typename... T>
	void add_item(T&&... args)
	{
		buffer_.emplace_back(std::forward<T>(args)...);
	}

	/**
	 * The various map rendering layers. This controls the internal rendering order.
	 */
	enum layer {
		/** Layer for the terrain drawn behind units. */
		LAYER_TERRAIN_BG,

		/** Top half part of grid image */
		LAYER_GRID_TOP,

		/** Mouseover overlay used by editor*/
		LAYER_MOUSEOVER_OVERLAY,

		/** Footsteps showing path from unit to mouse */
		LAYER_FOOTSTEPS,

		/** Top half of image following the mouse */
		LAYER_MOUSEOVER_TOP,

		/** Reserve layers to be selected for WML. */
		LAYER_UNIT_FIRST,

		/** Used for the ellipse behind units. */
		LAYER_UNIT_BG = LAYER_UNIT_FIRST + 10,

		/**default layer for drawing units */
		LAYER_UNIT_DEFAULT = LAYER_UNIT_FIRST + 40,

		/** Layer for the terrain drawn in front of units. */
		LAYER_TERRAIN_FG = LAYER_UNIT_FIRST + 50,

		/** Used for the bottom half part of grid image. Should be under moving units to avoid masking south move. */
		LAYER_GRID_BOTTOM,

		/** Default layer for drawing moving units */
		LAYER_UNIT_MOVE_DEFAULT = LAYER_UNIT_FIRST + 60,

		/** Used for the ellipse in front of units. */
		LAYER_UNIT_FG = LAYER_UNIT_FIRST + 80,

		/** Default layer for missile frames*/
		LAYER_UNIT_MISSILE_DEFAULT = LAYER_UNIT_FIRST + 90,

		LAYER_UNIT_LAST = LAYER_UNIT_FIRST + 100,

		/** "Black stripes" on unreachable hexes. */
		LAYER_REACHMAP,

		/** Bottom half of image following the mouse */
		LAYER_MOUSEOVER_BOTTOM,

		/** Fog and shroud. */
		LAYER_FOG_SHROUD,

		/** Arrows from the arrows framework. Used for planned moves display. */
		LAYER_ARROWS,

		/** Move numbering for the whiteboard. */
		LAYER_ACTIONS_NUMBERING,

		/** Image on the selected unit */
		LAYER_SELECTED_HEX,

		/** Layer which holds the attack indicator. */
		LAYER_ATTACK_INDICATOR,

		/** Unit bars and overlays are drawn at this layer (for testing here). */
		LAYER_UNIT_BAR,

		/** Movement info (defense %, etc...). */
		LAYER_MOVE_INFO,

		/** The overlay used for the linger mode. */
		LAYER_LINGER_OVERLAY,

		/** The map border. */
		LAYER_BORDER,
	};

private:
	/**
	 * In order to render a hex properly it needs to be rendered per row. On
	 * this row several layers need to be drawn at the same time. Mainly the
	 * unit and the background terrain. This is needed since both can spill
	 * in the next hex. The foreground terrain needs to be drawn before to
	 * avoid decapitation a unit.
	 *
	 * In other words:
	 * for every layer
	 *   for every row (starting from the top)
	 *     for every hex in the row
	 *       ...
	 *
	 * this is modified to:
	 * for every layer group
	 *   for every row (starting from the top)
	 *     for every layer in the group
	 *       for every hex in the row
	 *         ...
	 *
	 * * Surfaces are rendered per level in a map.
	 * * Per level the items are rendered per location these locations are
	 *   stored in the drawing order required for units.
	 * * every location has a vector with surfaces, each with its own screen
	 *   coordinate to render at.
	 * * every vector element has a vector with surfaces to render.
	 */
	class buffer_key
	{
	public:
		buffer_key(const map_location& loc, layer layer);

		bool operator<(const buffer_key& rhs) const
		{
			return key_ < rhs.key_;
		}

	private:
		unsigned int key_;

		using layer_group_array = const std::array<layer, 4>;

		// The drawing is done per layer_group, with the range per group being [low, high].
		// FIXME: better documentation.
		static layer_group_array layer_groups;
	};

	/** Helper structure for rendering the buffer contents. */
	class blit_helper
	{
	public:
		blit_helper(const layer layer,
				const map_location& loc,
				const int x,
				const int y,
				const surface& surf,
				const SDL_Rect& clip)
			: x_(x)
			, y_(y)
			, surf_(1, surf)
			, clip_(clip)
			, key_(loc, layer)
		{
		}

		blit_helper(const layer layer,
				const map_location& loc,
				const int x,
				const int y,
				const std::vector<surface>& surf,
				const SDL_Rect& clip)
			: x_(x)
			, y_(y)
			, surf_(surf)
			, clip_(clip)
			, key_(loc, layer)
		{
		}

		int x() const
		{
			return x_;
		}

		int y() const
		{
			return y_;
		}

		const std::vector<surface>& surfaces() const
		{
			return surf_;
		}

		const SDL_Rect& clip() const
		{
			return clip_;
		}

		bool operator<(const blit_helper& rhs) const
		{
			return key_ < rhs.key_;
		}

	private:
		/** x screen coordinate to render at. */
		int x_;

		/** y screen coordinate to render at. */
		int y_;

		/** surface(s) to render. */
		std::vector<surface> surf_;

		/** The clipping area of the source. If omitted the entire source is used. */
		SDL_Rect clip_;

		buffer_key key_;
	};

	std::list<blit_helper> buffer_;
};
