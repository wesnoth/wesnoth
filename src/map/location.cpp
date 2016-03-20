/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
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
 * @file
 * Routines related to game-maps, terrain, locations, directions. etc.
 */

#include "global.hpp"

#include <cassert>

#include "map/location.hpp"

#include "config.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "util.hpp"

#include <boost/assign/list_of.hpp>
#include <boost/functional/hash.hpp>

#define ERR_CF LOG_STREAM(err, config)
#define LOG_G LOG_STREAM(info, general)
#define DBG_G LOG_STREAM(debug, general)

std::ostream &operator<<(std::ostream &s, map_location const &l) {
	s << (l.x + 1) << ',' << (l.y + 1);
	return s;
}
std::ostream &operator<<(std::ostream &s, std::vector<map_location> const &v) {
	std::vector<map_location>::const_iterator i = v.begin();
	for(; i!= v.end(); ++i) {
		s << "(" << *i << ") ";
	}
	return s;
}

/**
 * Default list of directions
 *
 * Moved out of inline, because boost assign list_of is somewhat expensive...
 *
 **/
const std::vector<map_location::DIRECTION> & map_location::default_dirs() {
	static const std::vector<map_location::DIRECTION> dirs = boost::assign::list_of(map_location::NORTH)
				(map_location::NORTH_EAST)(map_location::SOUTH_EAST)(map_location::SOUTH)
				(map_location::SOUTH_WEST)(map_location::NORTH_WEST);
	return dirs;
}

/** Moved out of inline because of the boost dependency **/
std::size_t hash_value(map_location  const & a){
	boost::hash<size_t> h;
	return h( (a.x << 16) ^ a.y );
}


map_location::DIRECTION map_location::parse_direction(const std::string& str)
{
	if(str.empty()) {
		return NDIRECTIONS;
	}

	// Syntax: [-] (n|ne|se|s|sw|nw) [:cw|:ccw]
	// - means "take opposite direction" and has higher precedence
	// :cw and :ccw mean "one step (counter-)clockwise"
	// Parentheses can be used for grouping or to apply an operator more than once

	const size_t open = str.find_first_of('('), close = str.find_last_of(')');
	if (open != std::string::npos && close != std::string::npos) {
		std::string sub = str.substr(open + 1, close - open - 1);
		map_location::DIRECTION dir = parse_direction(sub);
		sub = str;
		sub.replace(open, close - open + 1, write_direction(dir));
		return parse_direction(sub);
	}

	const size_t start = str[0] == '-' ? 1 : 0;
	const size_t end = str.find_first_of(':');
	const std::string& main_dir = str.substr(start, end - start);
	map_location::DIRECTION dir;

	if (main_dir == "n") {
		dir = NORTH;
	} else if (main_dir == "ne") {
		dir = NORTH_EAST;
	} else if (main_dir == "se") {
		dir = SOUTH_EAST;
	} else if (main_dir == "s") {
		dir = SOUTH;
	} else if (main_dir == "sw") {
		dir = SOUTH_WEST;
	} else if (main_dir == "nw") {
		dir = NORTH_WEST;
	} else {
		return NDIRECTIONS;
	}

	if (start == 1) {
		dir = get_opposite_dir(dir);
	}

	if (end != std::string::npos) {
		const std::string rel_dir = str.substr(end + 1);
		if (rel_dir == "cw") {
			dir = rotate_right(dir, 1);
		} else if (rel_dir == "ccw") {
			dir = rotate_right(dir, -1);
		} else {
			return NDIRECTIONS;
		}
	}

	return dir;
}

std::vector<map_location::DIRECTION> map_location::parse_directions(const std::string& str)
{
	map_location::DIRECTION temp;
	std::vector<map_location::DIRECTION> to_return;
	std::vector<std::string> dir_strs = utils::split(str);
	std::vector<std::string>::const_iterator i, i_end=dir_strs.end();
	for(i = dir_strs.begin(); i != i_end; ++i) {
		temp = map_location::parse_direction(*i);
		// Filter out any invalid directions
		if(temp != NDIRECTIONS) {
			to_return.push_back(temp);
		}
	}
	return to_return;
}

std::string map_location::write_direction(map_location::DIRECTION dir)
{
	switch(dir) {
		case NORTH:
			return std::string("n");
		case NORTH_EAST:
			return std::string("ne");
		case NORTH_WEST:
			return std::string("nw");
		case SOUTH:
			return std::string("s");
		case SOUTH_EAST:
			return std::string("se");
		case SOUTH_WEST:
			return std::string("sw");
		default:
			return std::string();

	}
}

std::string map_location::write_translated_direction(map_location::DIRECTION dir)
{
	switch(dir) {
		case NORTH:
			return _("North");
		case NORTH_EAST:
			return _("North East");
		case NORTH_WEST:
			return _("North West");
		case SOUTH:
			return _("South");
		case SOUTH_EAST:
			return _("South East");
		case SOUTH_WEST:
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
	if(xs.empty() == false && xs != "recall")
		x = atoi(xs.c_str()) - 1;

	if(ys.empty() == false && ys != "recall")
		y = atoi(ys.c_str()) - 1;
}

void map_location::write(config& cfg) const
{
	cfg["x"] = x + 1;
	cfg["y"] = y + 1;
}

static bool is_vertically_higher_than ( const map_location & m1, const map_location & m2 ) {
	return (is_even(m1.x) && is_odd(m2.x)) ? (m1.y <= m2.y) : (m1.y < m2.y);
}

map_location::DIRECTION map_location::get_relative_dir(const map_location & loc) const
{
	return get_relative_dir(loc, map_location::RADIAL_SYMMETRY);
}

map_location::DIRECTION map_location::get_relative_dir(const map_location & loc, map_location::RELATIVE_DIR_MODE opt) const
{
	if (opt == map_location::DEFAULT) {
		map_location::DIRECTION dir = NDIRECTIONS;

		int dx = loc.x - x;
		int dy = loc.y - y;
		if (loc.x%2==0 && x%2==1) dy--;

		if (dx==0 && dy==0) return NDIRECTIONS;

		int dist = abs(dx);                                   // Distance from north-south line
		int dist_diag_SW_NE = abs(dy + (dx + (dy>0?0:1) )/2); // Distance from diagonal line SW-NE
		int dist_diag_SE_NW = abs(dy - (dx - (dy>0?0:1) )/2); // Distance from diagonal line SE-NW

		if (dy > 0) dir = SOUTH;
		else        dir = NORTH;

		if (dist_diag_SE_NW < dist) {
		if (dx>0) dir = SOUTH_EAST;
		else      dir = NORTH_WEST;
		dist = dist_diag_SE_NW;
		}
		if (dist_diag_SW_NE < dist) {
			if (dx>0) dir = NORTH_EAST;
			else      dir = SOUTH_WEST;
		}
		return dir;
	} else {
		map_location temp(loc);

		if (is_vertically_higher_than(temp,*this)) {
			temp = temp.rotate_right_around_center(*this,1u);
			if (!is_vertically_higher_than(temp,*this)) {
				return map_location::NORTH_EAST;
			}
			temp = temp.rotate_right_around_center(*this,1u);
			if (!is_vertically_higher_than(temp,*this)) {
				return map_location::NORTH;
			}
			return map_location::NORTH_WEST;
		} else if (is_vertically_higher_than(*this,temp)) {
			temp = temp.rotate_right_around_center(*this,1u);
			if (!is_vertically_higher_than(*this,temp)) {
				return map_location::SOUTH_WEST;
			}
			temp = temp.rotate_right_around_center(*this,1u);
			if (!is_vertically_higher_than(*this,temp)) {
				return map_location::SOUTH;
			}
			return map_location::SOUTH_EAST;
		} else if (temp.x > x) {
			return map_location::SOUTH_EAST;
		} else if (temp.x < x) {
			return map_location::NORTH_WEST;
		} else {
			return map_location::NDIRECTIONS;
		}
	}
}

std::pair<int,int> map_location::get_in_basis_N_NE() const {
	map_location temp(*this);
	std::pair<int, int> ret;

	ret.second = temp.x;
	temp = temp.get_direction(SOUTH_WEST,temp.x);
	assert(temp.x == 0);

	ret.first = -temp.y;
	temp = temp.get_direction(NORTH,temp.y);
	assert(temp.y == 0);

	temp = temp.get_direction(NORTH, ret.first);
	temp = temp.get_direction(NORTH_EAST, ret.second);
	assert(temp == *this);

	return ret;
}

map_location map_location::rotate_right_around_center(const map_location & center, int k) const {
	map_location temp(*this);
	temp.vector_difference_assign(center);

	std::pair<int,int> coords = temp.get_in_basis_N_NE();
	map_location::DIRECTION d1 = map_location::rotate_right(NORTH, k);
	map_location::DIRECTION d2 = map_location::rotate_right(NORTH_EAST, k);

	return center.get_direction(d1, coords.first).get_direction(d2, coords.second);
}

bool map_location::matches_range(const std::string& xloc, const std::string &yloc) const
{
	if(std::find(xloc.begin(),xloc.end(),',') != xloc.end()
	|| std::find(yloc.begin(),yloc.end(),',') != yloc.end()) {
		std::vector<std::string> xlocs = utils::split(xloc);
		std::vector<std::string> ylocs = utils::split(yloc);

		size_t size;
		for(size = xlocs.size(); size < ylocs.size(); ++size) {
			xlocs.push_back("");
		}
		while(size > ylocs.size()) {
			ylocs.push_back("");
		}
		for(size_t i = 0; i != size; ++i) {
			if(matches_range(xlocs[i],ylocs[i]))
				return true;
		}
		return false;
	}
	if(!xloc.empty()) {
		const std::string::const_iterator dash =
		             std::find(xloc.begin(),xloc.end(),'-');
		if(dash != xloc.begin() && dash != xloc.end()) {
			const std::string beg(xloc.begin(),dash);
			const std::string end(dash+1,xloc.end());

			const int bot = atoi(beg.c_str()) - 1;
			const int top = atoi(end.c_str()) - 1;

			if(x < bot || x > top)
				return false;
		} else {
			const int xval = atoi(xloc.c_str()) - 1;
			if(xval != x)
				return false;
		}
	}
	if(!yloc.empty()) {
		const std::string::const_iterator dash =
		             std::find(yloc.begin(),yloc.end(),'-');

		if(dash != yloc.begin() && dash != yloc.end()) {
			const std::string beg(yloc.begin(),dash);
			const std::string end(dash+1,yloc.end());

			const int bot = atoi(beg.c_str()) - 1;
			const int top = atoi(end.c_str()) - 1;

			if(y < bot || y > top)
				return false;
		} else {
			const int yval = atoi(yloc.c_str()) - 1;
			if(yval != y)
				return false;
		}
	}
	return true;
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
	x << (i->x + 1);
	y << (i->y + 1);

	for(++i; i != locs.end(); ++i) {
		if(i->x != first->x || i->y != last->y+1){
			if(last->y != first->y)
				y << "-" << (last->y + 1);
			x << "," << (i->x + 1);
			y << "," << (i->y + 1);
			first = i;
		}
		last = i;
	}
	// finish last range
	if(last->y != first->y)
		y << "-" << (last->y + 1);

	cfg["x"] = x.str();
	cfg["y"] = y.str();
}

static map_location read_locations_helper(const std::string & xi, const std::string & yi)
{
	return map_location(lexical_cast<int>(xi)-1, lexical_cast<int>(yi)-1);
}

void read_locations(const config& cfg, std::vector<map_location>& locs)
{
	const std::vector<std::string> xvals = utils::split(cfg["x"]);
	const std::vector<std::string> yvals = utils::split(cfg["y"]);

	if (xvals.size() != yvals.size()) {
		throw bad_lexical_cast();
	}

	std::transform(xvals.begin(), xvals.end(), yvals.begin(), std::back_inserter(locs), &read_locations_helper);
}

void write_locations(const std::vector<map_location>& locs, config& cfg)
{
	std::stringstream x, y;

	std::vector<map_location>::const_iterator i = locs.begin(),
			end = locs.end();

	for(; i != end; ++i) {
		x << (i->x + 1);
		y << (i->y + 1);
		if(i+1 != end){
			x << ",";
			y << ",";
		}
	}

	cfg["x"] = x.str();
	cfg["y"] = y.str();
}
