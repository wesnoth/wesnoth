/*
   Copyright (C) 2014 - 2017 by Chris Beck <render787@gmail.com>
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
 *
 * This wrapper class should be held by the display object when it needs to draw a unit.
 * The purpose of this is to improve encapsulation -- other parts of the engine like AI
 * don't need to be exposed to the unit drawing code, and encapsulation like this will
 * help us to reduce unnecessary includes.
 *
 **/

#pragma once

#include "map/location.hpp"
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
	size_t viewing_team;
	size_t playing_team;
	const team & viewing_team_ref;
	const team & playing_team_ref;
	bool is_blindfolded;
	bool show_everything;
	map_location sel_hex;
	map_location mouse_hex;
	double zoom_factor;

	int hex_size;
	int hex_size_by_2;

public:
	/** draw a unit.  */
	void redraw_unit(const unit & u) const;

private:
	/** draw a health/xp bar of a unit */
	void draw_bar(const std::string& image, int xpos, int ypos,
		const map_location& loc, size_t height, double filled,
		const color_t& col, fixed_t alpha) const;

	/**
	 * Finds the start and end rows on the energy bar image.
	 *
	 * White pixels are substituted for the color of the energy.
	 */
	const SDL_Rect& calculate_energy_bar(surface surf) const;
};
