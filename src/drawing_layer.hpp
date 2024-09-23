
/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

enum class drawing_layer {
	/** Terrain drawn behind the unit */
	terrain_bg,

	/** Top half part of grid image */
	grid_top,

	/** Mouseover overlay used by editor */
	mouseover_overlay,

	/** Footsteps showing path from unit to mouse */
	footsteps,

	/** Top half of image following the mouse */
	mouseover_top,

	/** Reserve layers to be selected for wml. */
	unit_first,

	/** Ellipse behind the unit. */
	unit_bg = unit_first + 10,

	/** Default layer for drawing units */
	unit_default = unit_first + 40,

	/** Terrain drawn in front of the unit */
	terrain_fg = unit_first + 50,

	/** Bottom half part of grid image. Should be under moving units, to avoid masking south move */
	grid_bottom,

	/** Default layer for drawing moving units */
	unit_move_default = unit_first + 60,

	/** Ellipse in front of the unit */
	unit_fg = unit_first + 80,

	/** Default layer for missile frames */
	unit_missile_default = unit_first + 90,

	unit_last = unit_first + 100,

	/** Overlay on unreachable hexes */
	reachmap,

	/** Bottom half of image following the mouse */
	mouseover_bottom,

	/** Fog and shroud. */
	fog_shroud,

	/** Arrows from the arrows framework. Used for planned moves display */
	arrows,

	/** Move numbering for the whiteboard */
	actions_numbering,

	/** Image on the selected unit */
	selected_hex,

	/** Layer which holds the attack indicator */
	attack_indicator,

	/** Unit bars and overlays are drawn on this layer (for testing here) */
	unit_bar,

	/** Movement info (defense%, etc...) */
	move_info,

	/** The overlay used for the linger mode */
	linger_overlay,

	/** The border of the map */
	border,
};
