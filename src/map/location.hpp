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

class config;
class config_attribute_value;
class variable_set;

#include <array>
#include <set>
#include <string>
#include <tuple>
#include <vector>
#include <utility>

struct wml_loc {};

/// Represents a map location in cubic hexagonal coordinates.
/// See <https://www.redblobgames.com/grids/hexagons/#coordinates-cube> for a more detailed explanation.
struct cubic_location {
	int q, r, s;
};

/**
 * Encapsulates the map of the game.
 *
 * Although the game is hexagonal, the map is stored as a grid.
 * Each type of terrain is represented by a multiletter terrain code.
 * @todo Update for new map-format.
 */
/** Represents a location on the map. */
struct map_location {
	/** Valid directions which can be moved in our hexagonal world. */
	enum class direction {
		north,
		north_east,
		south_east,
		south,
		south_west,
		north_west,
		indeterminate
	};

	static std::vector<direction> all_directions();

	/** Returns the direction one would face having taken @a steps clockwise around an undefined center. */
	static constexpr direction rotate_direction(direction d, int steps = 1)
	{
		if(d == direction::indeterminate) {
			return direction::indeterminate;
		}

		// Compensate for weirdness with the C++ % operator.
		// The plain formula (d + (steps % 6)) % 6 works for almost every case, but it returns incorrect
		// results for a counterclockwise rotation on direction::north. Instead, for any negative steps,
		// cancel out the sign and scale by the highest possible direction value (viz., 5). This has the
		// effect of a clockwise rotation ending at the same value as the specified counter-clockwise op.
		int adjusted_steps = steps >= 0 ? steps : steps * -5;
		return direction{(static_cast<int>(d) + (adjusted_steps % 6)) % 6};
	}

	static constexpr direction get_opposite_direction(direction d)
	{
		return rotate_direction(d, 3);
	}

	static direction parse_direction(const std::string& str);

	/**
	 * Parse_directions takes a comma-separated list, and filters out any
	 * invalid directions
	 */
	static std::vector<direction> parse_directions(const std::string& str);
	static std::string write_direction(direction dir);
	static std::string write_translated_direction(direction dir);

	map_location() : x(-1000), y(-1000) {}
	map_location(int x, int y) : x(x), y(y) {}
	map_location(int x, int y, wml_loc) : x(x - 1), y(y - 1) {}
	map_location(const config_attribute_value& x, const config_attribute_value& y, wml_loc);
	map_location(const config& cfg, const variable_set *variables = nullptr);

	static const map_location& ZERO()
	{
		static const map_location z(0,0);
		return z;
	}

	static const map_location& null_location()
	{
		static const map_location l;
		return l;
	}

	void write(config& cfg) const;

	bool valid() const { return x >= 0 && y >= 0; }

	bool valid(const int parWidth, const int parHeight) const
	{ return ((x >= 0) && (y >= 0) && (x < parWidth) && (y < parHeight)); }

	bool valid(const int parWidth, const int parHeight, const int border) const
	{ return ((x + border >= 0) && (y + border >= 0) && (x < parWidth + border) && (y < parHeight + border)); }

	bool matches_range(const std::string& xloc, const std::string& yloc) const;

	// Inlining those for performance reasons
	bool operator<(const map_location& a) const { return std::tie(x, y) < std::tie(a.x, a.y); }
	bool operator==(const map_location& a) const { return x == a.x && y == a.y; }
	bool operator!=(const map_location& a) const { return !operator==(a); }

	/** three-way comparator */
	int do_compare(const map_location& a) const {return x == a.x ? y - a.y : x - a.x; }

	// Location arithmetic operations treating the locations as vectors in
	// a hex-based space. These operations form an abelian group, i.e.
	// everything works as you would expect addition and subtraction to
	// work, with associativity and commutativity.
	map_location vector_negation() const
	{
		return map_location(-x, -y - (x & 1)); //subtract one if we're on an odd x coordinate
	}

	map_location vector_sum(const map_location& a) const
	{
		return map_location(*this).vector_sum_assign(a);
	}

	map_location& vector_sum_assign(const map_location& a)
	{
		y += ((x & 1) && (a.x & 1)); //add one if both x coords are odd
		x += a.x;
		y += a.y;
		return *this;
	}

	map_location& vector_difference_assign(const map_location& a)
	{
		return vector_sum_assign(a.vector_negation());
	}

	// Do n step in the direction d
	map_location get_direction(direction dir, unsigned int n = 1u) const;
	map_location get_direction(direction dir, signed int n) const
	{
		return (n >= 0) ? get_direction(dir, static_cast<unsigned int> (n)) : get_direction(get_opposite_direction(dir), static_cast<unsigned int> (-n));
	}

	enum RELATIVE_DIR_MODE { DEFAULT , RADIAL_SYMMETRY };
	direction get_relative_dir(const map_location& loc, map_location::RELATIVE_DIR_MODE mode /*= map_location::RADIAL_SYMMETRY*/ ) const;
	direction get_relative_dir(const map_location& loc) const; //moved the default setting to .cpp file for ease of testing

	cubic_location to_cubic() const {
		int q = x;
		int r = y - int((x - abs(x) % 2) / 2);
		int s = -q - r;
		return cubic_location{q, r, s};
	}
	static map_location from_cubic(cubic_location h) {
		int x = h.q;
		int y = h.r + int((h.q - abs(h.q) % 2) / 2);
		return map_location(x, y);
	}

	// Rotates the map_location clockwise in 60 degree increments around a center point. Negative numbers of steps are permitted.
	map_location rotate_right_around_center(const map_location& center, int k) const;

	friend std::size_t hash_value(const map_location& a);

	int wml_x() const { return x + 1; }
	int wml_y() const { return y + 1; }

	void set_wml_x(int v) { x = v - 1; }
	void set_wml_y(int v) { y = v - 1; }
	//on purpose these functions don't take map_location objects, if you use map_location objects to store 'differences' between 2 locations use vector_sum().
	void add(int x_diff, int y_diff) { x += x_diff; y += y_diff; }
	map_location plus(int x_diff, int y_diff) const { return map_location(x + x_diff, y + y_diff); }


	int x, y;
};

/** Function which tells if two locations are adjacent. */
bool tiles_adjacent(const map_location& a, const map_location& b);

/**
 * Function which, given a location, will place all adjacent locations in res.
 * res must point to an array of 6 location objects.
 */
void get_adjacent_tiles(const map_location& a, map_location* res);

/** Returns an array of the six hexes adjacent to @p center. */
std::array<map_location, 6> get_adjacent_tiles(const map_location& center);

/**
 * Function which gives the number of hexes between two tiles
 * (i.e. the minimum number of hexes that have to be traversed
 * to get from one hex to the other).
 */
std::size_t distance_between(const map_location& a, const map_location& b);

/**
 * Write a set of locations into a config using ranges,
 * adding keys x=x1,..,xn and y=y1a-y1b,..,yna-ynb
 */
void write_location_range(const std::set<map_location>& locs, config& cfg);

/**
 * Parse x,y keys of a config into a vector of locations
 *
 * Throws bad_lexical_cast if it fails to parse.
 */
void read_locations(const config& cfg, std::vector<map_location>& locs);

/** Write a vector of locations into a config
 *  adding keys x=x1,x2,..,xn and y=y1,y2,..,yn */
void write_locations(const std::vector<map_location>& locs, config& cfg);

/** Dumps a position on a stream, for debug purposes. */
std::ostream& operator<<(std::ostream& s, const map_location& l);
/** Dumps a vector of positions on a stream, for debug purposes. */
std::ostream& operator<<(std::ostream& s, const std::vector<map_location>& v);

/** Print a direction's string representation to stream. */
std::ostream& operator<<(std::ostream& s, map_location::direction dir);

namespace std {
template<>
struct hash<map_location> {
	std::size_t operator()(const map_location& l) const noexcept {
		// The 2000 bias supposedly ensures that the correct x is recovered for negative y
		// This implementation copied from the Lua location_set
		return (l.wml_x()) * 16384 + (l.wml_y()) + 2000;
	}
};
}
