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

/**
 * @file
 * Routines related to game-maps, terrain, locations, directions. etc.
 */

#include <cassert>

#include "map/location.hpp"

#include "config.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "utils/math.hpp"


static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)

std::ostream& operator<<(std::ostream& s, const map_location& l) {
	s << (l.wml_x()) << ',' << (l.wml_y());
	return s;
}

std::ostream& operator<<(std::ostream& s, const std::vector<map_location>& v) {
	std::vector<map_location>::const_iterator i = v.begin();
	for(; i!= v.end(); ++i) {
		s << "(" << *i << ") ";
	}
	return s;
}

/** Print a direction to stream. */
std::ostream& operator<<(std::ostream& s, map_location::direction dir)
{
	s << map_location::write_direction(dir);
	return s;
}

map_location::map_location(const config_attribute_value& x, const config_attribute_value& y, wml_loc)
	: map_location(x.to_int(), y.to_int(), wml_loc{})
{
}

auto map_location::all_directions() -> std::vector<direction>
{
	return {
		map_location::direction::north,
		map_location::direction::north_east,
		map_location::direction::south_east,
		map_location::direction::south,
		map_location::direction::south_west,
		map_location::direction::north_west
	};
}

std::size_t hash_value(const map_location& a){
	std::hash<std::size_t> h;
	return h( (static_cast<uint32_t>(a.x) << 16) ^ static_cast<uint32_t>(a.y) );
}


map_location::direction map_location::parse_direction(const std::string& str)
{
	if(str.empty()) {
		return direction::indeterminate;
	}

	// Syntax: [-] (n|ne|se|s|sw|nw) [:cw|:ccw]
	// - means "take opposite direction" and has higher precedence
	// :cw and :ccw mean "one step (counter-)clockwise"
	// Parentheses can be used for grouping or to apply an operator more than once

	const std::size_t open = str.find_first_of('('), close = str.find_last_of(')');
	if (open != std::string::npos && close != std::string::npos) {
		std::string sub = str.substr(open + 1, close - open - 1);
		map_location::direction dir = parse_direction(sub);
		sub = str;
		sub.replace(open, close - open + 1, write_direction(dir));
		return parse_direction(sub);
	}

	const std::size_t start = str[0] == '-' ? 1 : 0;
	const std::size_t end = str.find_first_of(':');
	const std::string& main_dir = str.substr(start, end - start);
	map_location::direction dir;

	if (main_dir == "n") {
		dir = direction::north;
	} else if (main_dir == "ne") {
		dir = direction::north_east;
	} else if (main_dir == "se") {
		dir = direction::south_east;
	} else if (main_dir == "s") {
		dir = direction::south;
	} else if (main_dir == "sw") {
		dir = direction::south_west;
	} else if (main_dir == "nw") {
		dir = direction::north_west;
	} else {
		return direction::indeterminate;
	}

	if (start == 1) {
		dir = get_opposite_direction(dir);
	}

	if (end != std::string::npos) {
		const std::string rel_dir = str.substr(end + 1);
		if (rel_dir == "cw") {
			dir = rotate_direction(dir, 1);
		} else if (rel_dir == "ccw") {
			dir = rotate_direction(dir, -1);
		} else {
			return direction::indeterminate;
		}
	}

	return dir;
}

std::vector<map_location::direction> map_location::parse_directions(const std::string& str)
{
	map_location::direction temp;
	std::vector<map_location::direction> to_return;
	std::vector<std::string> dir_strs = utils::split(str);
	std::vector<std::string>::const_iterator i, i_end=dir_strs.end();
	for(i = dir_strs.begin(); i != i_end; ++i) {
		temp = map_location::parse_direction(*i);
		// Filter out any invalid directions
		if(temp != direction::indeterminate) {
			to_return.push_back(temp);
		}
	}
	return to_return;
}

std::string map_location::write_direction(map_location::direction dir)
{
	switch(dir) {
		case direction::north:
			return std::string("n");
		case direction::north_east:
			return std::string("ne");
		case direction::north_west:
			return std::string("nw");
		case direction::south:
			return std::string("s");
		case direction::south_east:
			return std::string("se");
		case direction::south_west:
			return std::string("sw");
		default:
			return std::string();

	}
}

std::string map_location::write_translated_direction(map_location::direction dir)
{
	switch(dir) {
		case direction::north:
			return _("North");
		case direction::north_east:
			return _("North East");
		case direction::north_west:
			return _("North West");
		case direction::south:
			return _("South");
		case direction::south_east:
			return _("South East");
		case direction::south_west:
			return _("South West");
		default:
			return std::string();

	}
}

map_location::map_location(const config& cfg, const variable_set *variables) :
		x(-1000),
		y(-1000)
{
	std::string xs = cfg["x"], ys = cfg["y"];
	if (variables)
	{
		xs = utils::interpolate_variables_into_string( xs, *variables);
		ys = utils::interpolate_variables_into_string( ys, *variables);
	}
	// The co-ordinates in config files will be 1-based,
	// while we want them as 0-based.
	if(xs.empty() == false && xs != "recall") {
		try {
			x = std::stoi(xs) - 1;
		} catch(const std::invalid_argument&) {
			ERR_CF << "Invalid map coordinate: " << xs;
		}
	}

	if(ys.empty() == false && ys != "recall") {\
		try {
			y = std::stoi(ys) - 1;
		} catch(const std::invalid_argument&) {
			ERR_CF << "Invalid map coordinate: " << ys;
		}
	}
}

void map_location::write(config& cfg) const
{
	cfg["x"] = x + 1;
	cfg["y"] = y + 1;
}

static bool is_vertically_higher_than (const map_location& m1, const map_location& m2) {
	return (is_odd(m1.wml_x()) && is_even(m2.wml_x())) ? (m1.wml_y() <= m2.wml_y()) : (m1.wml_y() < m2.wml_y());
}

map_location::direction map_location::get_relative_dir(const map_location& loc) const
{
	return get_relative_dir(loc, map_location::RADIAL_SYMMETRY);
}

map_location::direction map_location::get_relative_dir(const map_location& loc, map_location::RELATIVE_DIR_MODE opt) const
{
	if (opt == map_location::DEFAULT) {
		map_location::direction dir = direction::indeterminate;

		int dx = loc.x - x;
		int dy = loc.y - y;
		if (loc.x%2==0 && x%2==1) dy--;

		if (dx==0 && dy==0) return direction::indeterminate;

		int dist = std::abs(dx);                                   // Distance from north-south line
		int dist_diag_SW_NE = std::abs(dy + (dx + (dy>0?0:1) )/2); // Distance from diagonal line SW-NE
		int dist_diag_SE_NW = std::abs(dy - (dx - (dy>0?0:1) )/2); // Distance from diagonal line SE-NW

		if (dy > 0) dir = direction::south;
		else        dir = direction::north;

		if (dist_diag_SE_NW < dist) {
		if (dx>0) dir = direction::south_east;
		else      dir = direction::north_west;
		dist = dist_diag_SE_NW;
		}
		if (dist_diag_SW_NE < dist) {
			if (dx>0) dir = direction::north_east;
			else      dir = direction::south_west;
		}
		return dir;
	} else {
		map_location temp(loc);

		if (is_vertically_higher_than(temp,*this)) {
			temp = temp.rotate_right_around_center(*this,1u);
			if (!is_vertically_higher_than(temp,*this)) {
				return map_location::direction::north_east;
			}
			temp = temp.rotate_right_around_center(*this,1u);
			if (!is_vertically_higher_than(temp,*this)) {
				return map_location::direction::north;
			}
			return map_location::direction::north_west;
		} else if (is_vertically_higher_than(*this,temp)) {
			temp = temp.rotate_right_around_center(*this,1u);
			if (!is_vertically_higher_than(*this,temp)) {
				return map_location::direction::south_west;
			}
			temp = temp.rotate_right_around_center(*this,1u);
			if (!is_vertically_higher_than(*this,temp)) {
				return map_location::direction::south;
			}
			return map_location::direction::south_east;
		} else if (temp.x > x) {
			return map_location::direction::south_east;
		} else if (temp.x < x) {
			return map_location::direction::north_west;
		} else {
			return map_location::direction::indeterminate;
		}
	}
}

map_location map_location::rotate_right_around_center(const map_location& center, int k) const {
	auto me_as_cube = to_cubic(), c_as_cube = center.to_cubic();
	auto vec = cubic_location{me_as_cube.q - c_as_cube.q, me_as_cube.r - c_as_cube.r, me_as_cube.s - c_as_cube.s};
	// These represent the 6 possible rotation matrices on the hex grid.
	// These are orthogonal 3x3 matrices containing only 0, 1, and -1.
	// Each element represents one row of the matrix.
	// The absolute value indicates which (1-based) column is non-zero.
	// The sign indicates whether that cell contains -1 or 1.
	static const int rotations[6][3] = {{1,2,3}, {-2,-3,-1}, {3,1,2}, {-1,-2,-3}, {2,3,1}, {-3,-1,-2}};
	int vec_temp[3] = {vec.q, vec.r, vec.s}, vec_temp2[3];
	int i = ((k % 6) + 6) % 6; // modulo-clamp rotation count to the range [0,6)
	assert(i >= 0 && i < 6);
	#define sgn(x) ((x) < 0 ? -1 : 1) // Not quite right, but we know we won't be passing in a 0
	for(int j = 0; j < 3; j++) vec_temp2[j] = sgn(rotations[i][j]) * vec_temp[abs(rotations[i][j])-1];
	#undef sgn
	vec.q = vec_temp2[0] + c_as_cube.q;
	vec.r = vec_temp2[1] + c_as_cube.r;
	vec.s = vec_temp2[2] + c_as_cube.s;
	return from_cubic(vec);
}

bool map_location::matches_range(const std::string& xloc, const std::string& yloc) const
{
	const auto xlocs = utils::split(xloc);
	const auto ylocs = utils::split(yloc);

	if(xlocs.size() == 0 && ylocs.size() == 0) {
		return true;
	}

	// Warn if both x and y were given, but they have different numbers of commas;
	// the missing entries will be assumed to be 1-infinity.
	//
	// No warning if only x or only y was given, as matching only that coordinate seems sane.
	if(xlocs.size() != ylocs.size() && xlocs.size() && ylocs.size()) {
		ERR_CF << "Different size lists when pairing coordinate ranges: " << xloc << " vs " << yloc;
	}

	std::size_t i = 0;
	for(; i < xlocs.size() && i < ylocs.size(); ++i) {
		const auto xr = utils::parse_range(xlocs[i]);
		const auto yr = utils::parse_range(ylocs[i]);
		// The ranges are 1-based, but the coordinates are 0-based. Thus the +1 s.
		if(xr.first <= x+1 && x+1 <= xr.second
			&& yr.first <= y+1 && y+1 <= yr.second) {
			return true;
		}
	}
	for(; i < xlocs.size(); ++i) {
		const auto xr = utils::parse_range(xlocs[i]);
		if(xr.first <= x+1 && x+1 <= xr.second) {
			return true;
		}
	}
	for(; i < ylocs.size(); ++i) {
		const auto yr = utils::parse_range(ylocs[i]);
		if(yr.first <= y+1 && y+1 <= yr.second) {
			return true;
		}
	}
	return false;
}

map_location map_location::get_direction(map_location::direction dir, unsigned int n) const
{
	if (dir == map_location::direction::indeterminate) {
		return map_location::null_location();
	}

	if (dir == direction::north) {
		return map_location(x,y-n);
	}

	if (dir == direction::south) {
		return map_location(x,y+n);
	}

	int x_factor = (static_cast<unsigned int> (dir) <= 2u) ? 1 : -1; //whether we go east + or west -

	unsigned int tmp_y = static_cast<unsigned int> (dir) - 2; //South East => 0, South => 1, South West => 2, North West => 3, North => INT_MAX, North East => INT_MAX - 1
	int y_factor = (tmp_y <= 2u) ? 1 : -1; //whether we go south + or north -

	if (tmp_y <= 2u) {
		return map_location(x + x_factor * n, y + y_factor * ((n + ((x & 1) == 1)) / 2));
	} else {
		return map_location(x + x_factor * n, y + y_factor * ((n + ((x & 1) == 0)) / 2));
	}

/*
	switch(dir) {
		case direction::north:      return map_location(x, y - n);
		case direction::south:      return map_location(x, y + n);
		case direction::south_east: return map_location(x + n, y + (n+is_odd(x))/2 );
		case direction::south_west: return map_location(x - n, y + (n+is_odd(x))/2 );
		case direction::north_east: return map_location(x + n, y - (n+is_even(x))/2 );
		case direction::north_west: return map_location(x - n, y - (n+is_even(x))/2 );
		default:
			assert(false);
			return map_location::null_location();
	}*/
}

void write_location_range(const std::set<map_location>& locs, config& cfg)
{
	if(locs.empty()){
		cfg["x"] = "";
		cfg["y"] = "";
		return;
	}

	// need that operator< uses x first
	assert(map_location(0,1) < map_location(1,0));

	std::stringstream x, y;
	std::set<map_location>::const_iterator
			i = locs.begin(),
			first = i,
			last = i;

	x << (i->wml_x());
	y << (i->wml_y());

	for(++i; i != locs.end(); ++i) {
		if(i->wml_x() != first->wml_x() || i->wml_y() - 1 != last->wml_y()) {
			if (last->wml_y() != first->wml_y()) {
				y << "-" << (last->wml_y());
			}
			x << "," << (i->wml_x());
			y << "," << (i->wml_y());
			first = i;
		}
		last = i;
	}
	// finish last range
	if(last->wml_y() != first->wml_y())
		y << "-" << (last->wml_y());

	cfg["x"] = x.str();
	cfg["y"] = y.str();
}

static map_location read_locations_helper(const std::string& xi, const std::string& yi)
{
	return map_location(std::stoi(xi)-1, std::stoi(yi)-1);
}

void read_locations(const config& cfg, std::vector<map_location>& locs)
{
	const std::vector<std::string> xvals = utils::split(cfg["x"]);
	const std::vector<std::string> yvals = utils::split(cfg["y"]);

	if (xvals.size() != yvals.size()) {
		throw std::invalid_argument("Number of x and y coordinates do not match.");
	}

	std::transform(xvals.begin(), xvals.end(), yvals.begin(), std::back_inserter(locs), &read_locations_helper);
}

void write_locations(const std::vector<map_location>& locs, config& cfg)
{
	std::stringstream x, y;

	std::vector<map_location>::const_iterator i = locs.begin(),
			end = locs.end();

	for(; i != end; ++i) {
		x << (i->wml_x());
		y << (i->wml_y());
		if(i+1 != end){
			x << ",";
			y << ",";
		}
	}

	cfg["x"] = x.str();
	cfg["y"] = y.str();
}

void get_adjacent_tiles(const map_location& a, map_location* res)
{
	res->x = a.x;
	res->y = a.y - 1;
	++res;
	res->x = a.x + 1;
	res->y = a.y - (((a.x & 1) == 0) ? 1 : 0);
	++res;
	res->x = a.x + 1;
	res->y = a.y + (((a.x & 1) == 1) ? 1 : 0);
	++res;
	res->x = a.x;
	res->y = a.y + 1;
	++res;
	res->x = a.x - 1;
	res->y = a.y + (((a.x & 1) == 1) ? 1 : 0);
	++res;
	res->x = a.x - 1;
	res->y = a.y - (((a.x & 1) == 0) ? 1 : 0);
}

std::array<map_location, 6> get_adjacent_tiles(const map_location& center)
{
	std::array<map_location, 6> res;
	get_adjacent_tiles(center, res.data());
	return res;
}

bool tiles_adjacent(const map_location& a, const map_location& b)
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

std::size_t distance_between(const map_location& a, const map_location& b)
{
	const std::size_t hdistance = std::abs(a.x - b.x);

	const std::size_t vpenalty = ( (((a.x & 1)==0) && ((b.x & 1)==1) && (a.y < b.y))
		|| (((b.x & 1)==0) && ((a.x & 1)==1) && (b.y < a.y)) ) ? 1 : 0;

/* Don't want to include util.hpp in this header
	const std::size_t vpenalty = ( (is_even(a.x) && is_odd(b.x) && (a.y < b.y))
		|| (is_even(b.x) && is_odd(a.x) && (b.y < a.y)) ) ? 1 : 0;
*/
	// For any non-negative integer i, i - i/2 - i%2 == i/2
	// previously returned (hdistance + vdistance - vsavings)
	// = hdistance + vdistance - minimum(vdistance,hdistance/2+hdistance%2)
	// = maximum(hdistance, vdistance+hdistance-hdistance/2-hdistance%2)
	// = maximum(hdistance,std::abs(a.y-b.y)+vpenalty+hdistance/2)

	return std::max<int>(hdistance, std::abs(a.y - b.y) + vpenalty + hdistance/2);
}
