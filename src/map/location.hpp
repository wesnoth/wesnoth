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

/** @file */

#ifndef MAP_LOCATION_H_INCLUDED
#define MAP_LOCATION_H_INCLUDED

class config;
class variable_set;

#include <cmath>
#include <cstdlib>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>

struct wml_loc {};

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
	enum DIRECTION { NORTH=0, NORTH_EAST=1, SOUTH_EAST=2, SOUTH=3,
					 SOUTH_WEST=4, NORTH_WEST=5, NDIRECTIONS=6 };

	static const std::vector<DIRECTION> & default_dirs();

	static DIRECTION rotate_right(DIRECTION d, unsigned int k = 1u);
	static DIRECTION rotate_right(DIRECTION d, signed int k);

	static DIRECTION get_opposite_dir(DIRECTION d);

	static DIRECTION parse_direction(const std::string& str);

	/**
	 * Parse_directions takes a comma-separated list, and filters out any
	 * invalid directions
	 */
	static std::vector<DIRECTION> parse_directions(const std::string& str);
	static std::string write_direction(DIRECTION dir);
	static std::string write_translated_direction(DIRECTION dir);

	map_location() : x(-1000), y(-1000) {}
	map_location(int x, int y) : x(x), y(y) {}
	map_location(int x, int y, wml_loc) : x(x - 1), y(y - 1) {}
	map_location(const config& cfg, const variable_set *variables = nullptr);

	static const map_location & ZERO();
	static const map_location & null_location();

	void write(config& cfg) const;

	inline bool valid() const { return x >= 0 && y >= 0; }

	inline bool valid(const int parWidth, const int parHeight) const
	{ return ((x >= 0) && (y >= 0) && (x < parWidth) && (y < parHeight)); }

	inline bool valid(const int parWidth, const int parHeight, const int border) const
	{ return ((x + border >= 0) && (y + border >= 0) && (x < parWidth + border) && (y < parHeight + border)); }

	bool matches_range(const std::string& xloc, const std::string& yloc) const;

	// Inlining those for performance reasons
	bool operator<(const map_location& a) const { return x < a.x || (x == a.x && y < a.y); }
	bool operator==(const map_location& a) const { return x == a.x && y == a.y; }
	bool operator!=(const map_location& a) const { return !operator==(a); }

        /** three-way comparator */
	int do_compare(const map_location& a) const {return x == a.x ? y - a.y : x - a.x; }

	// Location arithmetic operations treating the locations as vectors in
	// a hex-based space. These operations form an abelian group, i.e.
	// everything works as you would expect addition and subtraction to
	// work, with associativity and commutativity.
	map_location vector_negation() const;
	map_location vector_sum(const map_location &a) const;
	map_location& vector_sum_assign(const map_location &a);
	map_location& vector_difference_assign(const map_location &a);

	// Do n step in the direction d
	map_location get_direction(DIRECTION d, unsigned int n = 1u) const;
	map_location get_direction(DIRECTION d, signed int n) const;

	enum RELATIVE_DIR_MODE { DEFAULT , RADIAL_SYMMETRY };
	DIRECTION get_relative_dir(const map_location & loc, map_location::RELATIVE_DIR_MODE mode /*= map_location::RADIAL_SYMMETRY*/ ) const;
	DIRECTION get_relative_dir(const map_location & loc) const; //moved the default setting to .cpp file for ease of testing

	// Express as a vector in the basis N, NE. N, and NE may be obtained by zero.get_direction(NORTH), ...(NORTH_EAST), respectively.
	std::pair<int,int> get_in_basis_N_NE() const;

	// Rotates the map_location clockwise in 60 degree increments around a center point. Negative numbers of steps are permitted.
	map_location rotate_right_around_center(const map_location & center, int k) const;

	friend std::size_t hash_value(map_location const &a);

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

/**
 * Function which gives the number of hexes between two tiles
 * (i.e. the minimum number of hexes that have to be traversed
 * to get from one hex to the other).
 */
size_t distance_between(const map_location& a, const map_location& b);

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
std::ostream &operator<<(std::ostream &s, map_location const &l);
/** Dumps a vector of positions on a stream, for debug purposes. */
std::ostream &operator<<(std::ostream &s, std::vector<map_location> const &v);

namespace std {
template<>
struct hash<map_location> {
	size_t operator()(const map_location& l) const {
		// The 2000 bias supposedly ensures that the correct x is recovered for negative y
		// This implementation copied from the Lua location_set
		return (l.wml_x()) * 16384 + (l.wml_y()) + 2000;
	}
};
}

/** Inlined bodies **/

/** Inline direction manipulators **/
inline map_location::DIRECTION map_location::rotate_right(map_location::DIRECTION d, unsigned int k) {
	return (d == map_location::NDIRECTIONS) ? map_location::NDIRECTIONS : static_cast<map_location::DIRECTION>((d + (k%6u)) % 6u);
}

inline map_location::DIRECTION map_location::rotate_right(map_location::DIRECTION d, signed int k) {
	return (k>=0) ? rotate_right(d, static_cast<unsigned int> (k)) : rotate_right(d, (static_cast<unsigned int>(-k) % 6u) * 5u);
}

inline map_location::DIRECTION map_location::get_opposite_dir(map_location::DIRECTION d) {
	return rotate_right(d,3u);
}

/** Old implementation: **/
/*map_location::DIRECTION map_location::get_opposite_dir(map_location::DIRECTION d) {
	switch (d) {
		case NORTH:
			return SOUTH;
		case NORTH_EAST:
			return SOUTH_WEST;
		case SOUTH_EAST:
			return NORTH_WEST;
		case SOUTH:
			return NORTH;
		case SOUTH_WEST:
			return NORTH_EAST;
		case NORTH_WEST:
			return SOUTH_EAST;
		case NDIRECTIONS:
		default:
			return NDIRECTIONS;
	}
}*/


/** Inline constant map_location defns **/
inline const map_location & map_location::ZERO() {
	static const map_location z(0,0);
	return z;
}

inline const map_location & map_location::null_location() {
	static const map_location l;
	return l;
}

/** Inline vector ops **/
inline map_location map_location::vector_negation() const
{
	return map_location(-x, -y - (x & 1)); //subtract one if we're on an odd x coordinate
}

inline map_location map_location::vector_sum(const map_location& a) const
{
	return map_location(*this).vector_sum_assign(a);
}

inline map_location& map_location::vector_sum_assign(const map_location &a)
{
	y += ((x & 1) && (a.x & 1)); //add one if both x coords are odd
	x += a.x;
	y += a.y;
	return *this;
}

inline map_location& map_location::vector_difference_assign(const map_location &a)
{
	return vector_sum_assign(a.vector_negation());
}

/** Get Direction function **/

inline map_location map_location::get_direction(
			map_location::DIRECTION dir, signed int n) const
{
	return (n >= 0) ? get_direction(dir, static_cast<unsigned int> (n)) : get_direction(get_opposite_dir(dir), static_cast<unsigned int> (-n));
}

inline map_location map_location::get_direction(
			map_location::DIRECTION dir, unsigned int n) const
{
	if (dir == map_location::NDIRECTIONS) {
		return map_location::null_location();
	}

	if (dir == NORTH) {
		return map_location(x,y-n);
	}

	if (dir == SOUTH) {
		return map_location(x,y+n);
	}

	int x_factor = (static_cast<unsigned int> (dir) <= 2u) ? 1 : -1; //whether we go east + or west -

	unsigned int tmp_y = dir - 2; //South East => 0, South => 1, South West => 2, North West => 3, North => INT_MAX, North East => INT_MAX - 1
	int y_factor = (tmp_y <= 2u) ? 1 : -1; //whether we go south + or north -

	if (tmp_y <= 2u) {
		return map_location(x + x_factor * n, y + y_factor * ((n + ((x & 1) == 1)) / 2));
	} else {
		return map_location(x + x_factor * n, y + y_factor * ((n + ((x & 1) == 0)) / 2));
	}

/*
	switch(dir) {
		case NORTH:      return map_location(x, y - n);
		case SOUTH:      return map_location(x, y + n);
		case SOUTH_EAST: return map_location(x + n, y + (n+is_odd(x))/2 );
		case SOUTH_WEST: return map_location(x - n, y + (n+is_odd(x))/2 );
		case NORTH_EAST: return map_location(x + n, y - (n+is_even(x))/2 );
		case NORTH_WEST: return map_location(x - n, y - (n+is_even(x))/2 );
		default:
			assert(false);
			return map_location::null_location();
	}*/
}

/** inline get_adjacent, and get_distance functions **/

inline void get_adjacent_tiles(const map_location& a, map_location* res)
{
	res->x = a.x;
	res->y = a.y-1;
	++res;
	res->x = a.x+1;
	res->y = a.y - (((a.x & 1)==0) ? 1:0);
	++res;
	res->x = a.x+1;
	res->y = a.y + (((a.x & 1)==1) ? 1:0);
	++res;
	res->x = a.x;
	res->y = a.y+1;
	++res;
	res->x = a.x-1;
	res->y = a.y + (((a.x & 1)==1) ? 1:0);
	++res;
	res->x = a.x-1;
	res->y = a.y - (((a.x & 1)==0) ? 1:0);
/* Changed this when I inlined it to eliminate util.hpp dependency.
	res->x = a.x;
	res->y = a.y-1;
	++res;
	res->x = a.x+1;
	res->y = a.y - (is_even(a.x) ? 1:0);
	++res;
	res->x = a.x+1;
	res->y = a.y + (is_odd(a.x) ? 1:0);
	++res;
	res->x = a.x;
	res->y = a.y+1;
	++res;
	res->x = a.x-1;
	res->y = a.y + (is_odd(a.x) ? 1:0);
	++res;
	res->x = a.x-1;
	res->y = a.y - (is_even(a.x) ? 1:0);
*/
}

inline bool tiles_adjacent(const map_location& a, const map_location& b)
{
	// Two tiles are adjacent:
	// if y is different by 1, and x by 0,
	// or if x is different by 1 and y by 0,
	// or if x and y are each different by 1,
	// and the x value of the hex with the greater y value is even.

	switch (a.y - b.y) {
		case 1 :
			switch (a.x - b.x) {
				case 1:
				case -1:
					return (a.x & 1) == 0;
				case 0:
					return true;
				default:
					return false;
			}
		case -1 :
			switch (a.x - b.x) {
				case 1:
				case -1:
					return (b.x & 1) == 0;
				case 0:
					return true;
				default:
					return false;
			}
		case 0 :
			return ((a.x - b.x) == 1) || ((a.x - b.x) == - 1);
		default:
			return false;
	}

	/*
	const int xdiff = std::abs(a.x - b.x);
	const int ydiff = std::abs(a.y - b.y);
	return (ydiff == 1 && a.x == b.x) || (xdiff == 1 && a.y == b.y) ||
	       (xdiff == 1 && ydiff == 1 && (a.y > b.y ? is_even(a.x) : is_even(b.x)));
	*/
}

inline size_t distance_between(const map_location& a, const map_location& b)
{
	const size_t hdistance = std::abs(a.x - b.x);

	const size_t vpenalty = ( (((a.x & 1)==0) && ((b.x & 1)==1) && (a.y < b.y))
		|| (((b.x & 1)==0) && ((a.x & 1)==1) && (b.y < a.y)) ) ? 1 : 0;

/* Don't want to include util.hpp in this header
	const size_t vpenalty = ( (is_even(a.x) && is_odd(b.x) && (a.y < b.y))
		|| (is_even(b.x) && is_odd(a.x) && (b.y < a.y)) ) ? 1 : 0;
*/
	// For any non-negative integer i, i - i/2 - i%2 == i/2
	// previously returned (hdistance + vdistance - vsavings)
	// = hdistance + vdistance - minimum(vdistance,hdistance/2+hdistance%2)
	// = maximum(hdistance, vdistance+hdistance-hdistance/2-hdistance%2)
	// = maximum(hdistance,std::abs(a.y-b.y)+vpenalty+hdistance/2)

	return std::max<int>(hdistance, std::abs(a.y - b.y) + vpenalty + hdistance/2);
}


#endif
