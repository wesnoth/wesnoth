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

class drawing_queue
{
public:
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
};
