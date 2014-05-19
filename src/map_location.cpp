/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
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

#include "map_location.hpp"

#include "config.hpp"
#include "formula_string_utils.hpp"
#include "map.hpp"
#include "resources.hpp"
#include "util.hpp"


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

const map_location map_location::null_location;

map_location::DIRECTION map_location::parse_direction(const std::string& str)
{
	if(!str.empty()) {
		if(str == "n") {
			return NORTH;
		} else if(str == "ne") {
			return NORTH_EAST;
		} else if(str == "se") {
			return SOUTH_EAST;
		} else if(str == "s") {
			return SOUTH;
		} else if(str == "sw") {
			return SOUTH_WEST;
		} else if(str == "nw") {
			return NORTH_WEST;
		} else if(str[0] == '-' && str.length() <= 10) {
			// A minus sign reverses the direction
			return get_opposite_dir(parse_direction(str.substr(1)));
		}
	}
	return NDIRECTIONS;
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

map_location map_location::vector_negation() const
{
	return map_location(-x, -y - (x & 1)); //subtract one if we're on an odd x coordinate
}

map_location map_location::vector_sum(const map_location& a) const
{
	return map_location(*this).vector_sum_assign(a);
}

map_location& map_location::vector_sum_assign(const map_location &a)
{
	y += ((x & 1) && (a.x & 1)); //add one if both x coords are odd
	x += a.x;
	y += a.y;
	return *this;
}

map_location& map_location::vector_difference_assign(const map_location &a)
{
	return vector_sum_assign(a.vector_negation());
}

map_location map_location::get_direction(
			map_location::DIRECTION dir, int n) const
{
	if (n < 0 ) {
		dir = get_opposite_dir(dir);
		n = -n;
	}
	switch(dir) {
		case NORTH:      return map_location(x, y - n);
		case SOUTH:      return map_location(x, y + n);
		case SOUTH_EAST: return map_location(x + n, y + (n+is_odd(x))/2 );
		case SOUTH_WEST: return map_location(x - n, y + (n+is_odd(x))/2 );
		case NORTH_EAST: return map_location(x + n, y - (n+is_even(x))/2 );
		case NORTH_WEST: return map_location(x - n, y - (n+is_even(x))/2 );
		default:
			assert(false);
			return map_location();
	}
}

map_location::DIRECTION map_location::get_relative_dir(const map_location & loc) const
{
	map_location::DIRECTION dir = NDIRECTIONS;
	
	int dx = loc.x - x;
	int dy = loc.y - y;
	if (loc.x%2==1 && x%2==0) dy--;
	
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
}

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

void get_adjacent_tiles(const map_location& a, map_location* res)
{
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
}


bool tiles_adjacent(const map_location& a, const map_location& b)
{
	// Two tiles are adjacent:
	// if y is different by 1, and x by 0,
	// or if x is different by 1 and y by 0,
	// or if x and y are each different by 1,
	// and the x value of the hex with the greater y value is even.

	const int xdiff = abs(a.x - b.x);
	const int ydiff = abs(a.y - b.y);
	return (ydiff == 1 && a.x == b.x) || (xdiff == 1 && a.y == b.y) ||
	       (xdiff == 1 && ydiff == 1 && (a.y > b.y ? is_even(a.x) : is_even(b.x)));
}

size_t distance_between(const map_location& a, const map_location& b)
{
	const size_t hdistance = abs(a.x - b.x);

	const size_t vpenalty = ( (is_even(a.x) && is_odd(b.x) && (a.y < b.y))
		|| (is_even(b.x) && is_odd(a.x) && (b.y < a.y)) ) ? 1 : 0;

	// For any non-negative integer i, i - i/2 - i%2 == i/2
	// previously returned (hdistance + vdistance - vsavings)
	// = hdistance + vdistance - minimum(vdistance,hdistance/2+hdistance%2)
	// = maximum(hdistance, vdistance+hdistance-hdistance/2-hdistance%2)
	// = maximum(hdistance,abs(a.y-b.y)+vpenalty+hdistance/2)

	return std::max<int>(hdistance, abs(a.y - b.y) + vpenalty + hdistance/2);
}

std::vector<map_location> parse_location_range(const std::string &x, const std::string &y,
	bool with_border)
{
	std::vector<map_location> res;
	const std::vector<std::string> xvals = utils::split(x);
	const std::vector<std::string> yvals = utils::split(y);
	gamemap *map = resources::game_map;
	assert(map);
	int xmin = 1, xmax = map->w(), ymin = 1, ymax = map->h();
	if (with_border) {
		int bs = map->border_size();
		xmin -= bs;
		xmax += bs;
		ymin -= bs;
		ymax += bs;
	}

	for (unsigned i = 0; i < xvals.size() || i < yvals.size(); ++i)
	{
		std::pair<int,int> xrange, yrange;

		if (i < xvals.size()) {
			xrange = utils::parse_range(xvals[i]);
			if (xrange.first < xmin) xrange.first = xmin;
			if (xrange.second > xmax) xrange.second = xmax;
		} else {
			xrange.first = xmin;
			xrange.second = xmax;
		}

		if (i < yvals.size()) {
			yrange = utils::parse_range(yvals[i]);
			if (yrange.first < ymin) yrange.first = ymin;
			if (yrange.second > ymax) yrange.second = ymax;
		} else {
			yrange.first = ymin;
			yrange.second = ymax;
		}

		for(int x = xrange.first; x <= xrange.second; ++x) {
			for(int y = yrange.first; y <= yrange.second; ++y) {
				res.push_back(map_location(x-1,y-1));
			}
		}
	}
	return res;
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

map_location read_locations_helper(const std::string & xi, const std::string & yi) 
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
