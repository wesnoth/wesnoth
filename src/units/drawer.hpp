/*
	Copyright (C) 2014 - 2022
	by Chris Beck <render787@gmail.com>
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
 *
 * This wrapper class should be held by the display object when it needs to draw a unit.
 * The purpose of this is to improve encapsulation -- other parts of the engine like AI
 * don't need to be exposed to the unit drawing code, and encapsulation like this will
 * help us to reduce unnecessary includes.
 *
 **/

#pragma once

#include "map/location.hpp"
#include "sdl/rect.hpp"
#include "utils/math.hpp"

#include <map>
#include <vector>

class display;
class display_context;
class gamemap;
namespace halo { class manager; }
class team;
class unit;

struct color_t;
struct SDL_Rect;
struct point;
class surface;

class unit_drawer
{
public:
	explicit unit_drawer(display & thedisp);

private:
	display & disp;
	const display_context & dc;
	const gamemap & map;
	const std::vector<team> & teams;
	halo::manager & halo_man;
	std::size_t viewing_team;
	std::size_t playing_team;
	const team & viewing_team_ref;
	const team & playing_team_ref;
	bool is_blindfolded;
	bool show_everything;
	map_location sel_hex;
	map_location mouse_hex;
	double zoom_factor;
	std::set<map_location> units_that_can_reach_goal;

	int hex_size;
	int hex_size_by_2;

public:
	/** draw a unit.  */
	void redraw_unit(const unit & u) const;

private:
	/** draw a health/xp bar of a unit */
	void draw_bar(int xpos, int ypos, const map_location& loc,
		int height, double filled, const color_t& col, uint8_t alpha) const;

	/**
	 * Find where to draw the bar on an energy bar image.
	 *
	 * Results are cached so this can be called frequently.
	 *
	 * This looks for a coloured region with significant (>0x10) alpha
	 * and blackish colour (<0x10 in RGB channels).
	 */
	rect calculate_energy_bar(const std::string& bar_image) const;

	/** Scale a rect to the current zoom level. */
	rect scaled_to_zoom(const rect& r) const;
	/** Scale a point to the current zoom level. */
	point scaled_to_zoom(const point& p) const;
};
