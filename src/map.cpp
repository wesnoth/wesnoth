/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file map.cpp 
//! Routines related to game-maps, terrain, locations, directions. etc.

#include "global.hpp"

#include "config.hpp"
#include "log.hpp"
#include "map.hpp"
#include "pathfind.hpp"
#include "util.hpp"
#include "wassert.hpp"
#include "serialization/string_utils.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>

#define ERR_CF LOG_STREAM(err, config)
#define LOG_G LOG_STREAM(info, general)

std::ostream &operator<<(std::ostream &s, gamemap::location const &l) {
	s << (l.x + 1) << ',' << (l.y + 1);
	return s;
}

gamemap::location gamemap::location::null_location;

const t_translation::t_list& gamemap::underlying_mvt_terrain(t_translation::t_letter terrain) const
{
	const std::map<t_translation::t_letter,terrain_type>::const_iterator i =
		letterToTerrain_.find(terrain);

	if(i == letterToTerrain_.end()) {
		static t_translation::t_list result(1);
		result[0] = terrain;
		return result;
	} else {
		return i->second.mvt_type();
	}
}

const t_translation::t_list& gamemap::underlying_def_terrain(t_translation::t_letter terrain) const
{
	const std::map<t_translation::t_letter, terrain_type>::const_iterator i =
		letterToTerrain_.find(terrain);

	if(i == letterToTerrain_.end()) {
		static t_translation::t_list result(1);
		result[0] = terrain;
		return result;
	} else {
		return i->second.def_type();
	}
}

const t_translation::t_list& gamemap::underlying_union_terrain(t_translation::t_letter terrain) const
{
	const std::map<t_translation::t_letter,terrain_type>::const_iterator i =
		letterToTerrain_.find(terrain);

	if(i == letterToTerrain_.end()) {
		static t_translation::t_list result(1);
		result[0] = terrain;
		return result;
	} else {
		return i->second.union_type();
	}
}

void gamemap::write_terrain(const gamemap::location &loc, config& cfg) const
{
	cfg["terrain"] = t_translation::write_letter(get_terrain(loc));
}

gamemap::location::DIRECTION gamemap::location::parse_direction(const std::string& str)
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

std::vector<gamemap::location::DIRECTION> gamemap::location::parse_directions(const std::string& str)
{
	gamemap::location::DIRECTION temp;
	std::vector<gamemap::location::DIRECTION> to_return;
	std::vector<std::string> dir_strs = utils::split(str);
	std::vector<std::string>::const_iterator i, i_end=dir_strs.end();
	for(i = dir_strs.begin(); i != i_end; ++i) {
		temp = gamemap::location::parse_direction(*i);
		// Filter out any invalid directions
		if(temp != NDIRECTIONS) {
			to_return.push_back(temp);
		}
	}
	return to_return;
}

std::string gamemap::location::write_direction(gamemap::location::DIRECTION dir)
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

gamemap::location::location(const config& cfg, const variable_set *variables) :
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
	if(xs.empty() == false)
		x = atoi(xs.c_str()) - 1;

	if(ys.empty() == false)
		y = atoi(ys.c_str()) - 1;
}

void gamemap::location::write(config& cfg) const
{
	char buf[50];
	snprintf(buf,sizeof(buf),"%d",x+1);
	cfg["x"] = buf;
	snprintf(buf,sizeof(buf),"%d",y+1);
	cfg["y"] = buf;
}

gamemap::location gamemap::location::operator-() const
{
	location ret;
	ret.x = -x;
	ret.y = -y;

	return ret;
}

gamemap::location gamemap::location::operator+(const gamemap::location& a) const
{
	gamemap::location ret = *this;
	ret += a;
	return ret;
}

gamemap::location& gamemap::location::operator+=(const gamemap::location &a)
{
	bool parity = (x & 1) != 0;

	x += a.x;
	y += a.y;

	if((a.x > 0) && (a.x % 2) && parity)
		y++;
	if((a.x < 0) && (a.x % 2) && !parity)
		y--;

	return *this;
}

gamemap::location gamemap::location::operator-(const gamemap::location &a) const
{
	return operator+(-a);
}

gamemap::location& gamemap::location::operator-=(const gamemap::location &a)
{
	return operator+=(-a);
}

gamemap::location gamemap::location::get_direction(
                                     gamemap::location::DIRECTION dir) const
{
	switch(dir) {
		case NORTH:      return gamemap::location(x,y-1);
		case NORTH_EAST: return gamemap::location(x+1,y-is_even(x));
		case SOUTH_EAST: return gamemap::location(x+1,y+is_odd(x));
		case SOUTH:      return gamemap::location(x,y+1);
		case SOUTH_WEST: return gamemap::location(x-1,y+is_odd(x));
		case NORTH_WEST: return gamemap::location(x-1,y-is_even(x));
		default:
			wassert(false);
			return gamemap::location();
	}
}

gamemap::location::DIRECTION gamemap::location::get_relative_dir(gamemap::location loc) const {
	location diff = loc -*this;
	if(diff == location(0,0)) return NDIRECTIONS;
	if( diff.y < 0 && diff.x >= 0 && abs(diff.x) >= abs(diff.y)) return NORTH_EAST;
	if( diff.y < 0 && diff.x <  0 && abs(diff.x) >= abs(diff.y)) return NORTH_WEST;
	if( diff.y < 0 && abs(diff.x) < abs(diff.y)) return NORTH;

	if( diff.y >= 0 && diff.x >= 0 && abs(diff.x) >= abs(diff.y)) return SOUTH_EAST;
	if( diff.y >= 0 && diff.x <  0 && abs(diff.x) >= abs(diff.y)) return SOUTH_WEST;
	if( diff.y >= 0 && abs(diff.x) < abs(diff.y)) return SOUTH;

	// Impossible
	wassert(false);
	return NDIRECTIONS;


}
gamemap::location::DIRECTION gamemap::location::get_opposite_dir(gamemap::location::DIRECTION d) {
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
}

gamemap::gamemap(const config& cfg, const std::string& data) :
		tiles_(1), 
		terrainList_(),
		letterToTerrain_(),
		villages_(),
		borderCache_(),
		terrainFrequencyCache_(),
		w_(-1), 
		h_(-1)
{
	LOG_G << "loading map: '" << data << "'\n";
	const config::child_list& terrains = cfg.get_children("terrain");
	create_terrain_maps(terrains,terrainList_,letterToTerrain_);

	read(data);
}

void gamemap::read(const std::string& data)
{
	tiles_.clear();
	villages_.clear();
	std::fill(startingPositions_,startingPositions_+sizeof(startingPositions_)/sizeof(*startingPositions_),location());
	std::map<int, t_translation::coordinate> starting_positions;

	try {
		tiles_ = t_translation::read_game_map(data, starting_positions);
	} catch(t_translation::error& e) {
		// We re-throw the error but as map error. 
		// Since all codepaths test for this, it's the least work.
		throw incorrect_format_exception(e.message.c_str());
	}

	// Convert the starting positions to the array
	std::map<int, t_translation::coordinate>::const_iterator itor =
		starting_positions.begin();

	for(; itor != starting_positions.end(); ++itor) {

		// Check for valid position, 
		// the first valid position is 1,
		// so the offset 0 in the array is never used.
		if(itor->first < 1 || itor->first >= STARTING_POSITIONS) {
			ERR_CF << "Starting position " << itor->first << " out of range\n";
			throw incorrect_format_exception("Illegal starting position found in map. The scenario cannot be loaded.");
		}

		// Add to the starting position array
		startingPositions_[itor->first] = location(itor->second.x, itor->second.y);
	}

	// Post processing on the map
	const int width = tiles_.size();
	const int height = width > 0 ? tiles_[0].size() : 0;
	w_ = width;
	h_ = height;
	for(int x = 0; x < width; ++x) {
		for(int y = 0; y < height; ++y) {
			
			// Is the terrain valid? 
			if(letterToTerrain_.count(tiles_[x][y]) == 0) {
				ERR_CF << "Illegal character in map: (" << t_translation::write_letter(tiles_[x][y])
					<< ") '" << tiles_[x][y] << "'\n";
				throw incorrect_format_exception("Illegal character found in map. The scenario cannot be loaded.");
			}

			// Is it a village?
			if(is_village(tiles_[x][y])) {
				villages_.push_back(location(x, y));
			}
		}
	}
}

std::string gamemap::write() const
{
	std::map<int, t_translation::coordinate> starting_positions = std::map<int, t_translation::coordinate>();

	// Convert the starting positions to a map
	for(int i = 0; i < STARTING_POSITIONS; ++i) {
	if(on_board(startingPositions_[i])) {
			const struct t_translation::coordinate position =
				{startingPositions_[i].x, startingPositions_[i].y};

			 starting_positions.insert(std::pair<int, t_translation::coordinate>(i, position));
		}
	}

	// Let the low level convertor do the conversion
	return t_translation::write_game_map(tiles_, starting_positions);
}

void gamemap::overlay(const gamemap& m, const config& rules_cfg, const int xpos, const int ypos)
{
	const config::child_list& rules = rules_cfg.get_children("rule");

	const int xstart = maximum<int>(0, -xpos);
	const int ystart = maximum<int>(0, -ypos-((xpos & 1) ? 1 : 0));
	const int xend = minimum<int>(m.w(),w()-xpos);
	const int yend = minimum<int>(m.h(),h()-ypos);
	for(int x1 = xstart; x1 < xend; ++x1) {
		for(int y1 = ystart; y1 < yend; ++y1) {
			const int x2 = x1 + xpos;
			const int y2 = y1 + ypos +
				((xpos & 1) && (x1 & 1) ? 1 : 0);
			if (y2 < 0 || y2 >= h()) {
				continue;
			}
			const t_translation::t_letter t = m[x1][y1];
			const t_translation::t_letter current = (*this)[x2][y2];

			if(t == t_translation::FOGGED || t == t_translation::VOID_TERRAIN) {
				continue;
			}

			// See if there is a matching rule
			config::child_list::const_iterator rule = rules.begin();
			for( ; rule != rules.end(); ++rule) {
				static const std::string src_key = "old", src_not_key = "old_not",
				                         dst_key = "new", dst_not_key = "new_not";
				const config& cfg = **rule;
				const t_translation::t_list& src = t_translation::read_list(cfg[src_key]);

				if(!src.empty() && t_translation::terrain_matches(current, src) == false) {
					continue;
				}

				const t_translation::t_list& src_not = t_translation::read_list(cfg[src_not_key]);

				if(!src_not.empty() && t_translation::terrain_matches(current, src_not)) {
					continue;
				}

				const t_translation::t_list& dst = t_translation::read_list(cfg[dst_key]);

				if(!dst.empty() && t_translation::terrain_matches(t, dst) == false) {
					continue;
				}

				const t_translation::t_list& dst_not = t_translation::read_list(cfg[dst_not_key]);

				if(!dst_not.empty() && t_translation::terrain_matches(t, dst_not)) {
					continue;
				}

				break;
			}


			if(rule != rules.end()) {
				const config& cfg = **rule;
				const t_translation::t_list& terrain = t_translation::read_list(cfg["terrain"]);

				if(!terrain.empty()) {
					set_terrain(location(x2,y2),terrain[0]);
				} else if(cfg["use_old"] != "yes") {
					set_terrain(location(x2,y2),t);
				}
			} else {
				set_terrain(location(x2,y2),t);
			}
		}
	}

	for(const location* pos = m.startingPositions_;
			pos != m.startingPositions_ + sizeof(m.startingPositions_)/sizeof(*m.startingPositions_);
			++pos) {

		if(pos->valid()) {
			startingPositions_[pos - m.startingPositions_] = *pos;
		}
	}
}

t_translation::t_letter gamemap::get_terrain(const gamemap::location& loc) const
{

	if(on_board(loc)) {
		return tiles_[loc.x][loc.y];
	}

	const std::map<location, t_translation::t_letter>::const_iterator itor = borderCache_.find(loc);
	if(itor != borderCache_.end())
		return itor->second;

	// If not on the board, decide based on what surrounding terrain is
	t_translation::t_letter items[6];
	int nitems = 0;

	location adj[6];
	get_adjacent_tiles(loc,adj);
	for(int n = 0; n != 6; ++n) {
		if(on_board(adj[n])) {
			items[nitems] = tiles_[adj[n].x][adj[n].y];
			++nitems;
		} else {
			// If the terrain is off map but already in the border cache, 
			// this will be used to determine the terrain. 
			// This avoids glitches
			// * on map with an even width in the top right corner
			// * on map with an odd height in the bottom left corner.
			// It might also change the result on other map and become random,
			// but the border tiles will be determined in the future, so then
			// this will no longer be used in the game 
			// (The editor will use this feature to expand maps in a better way).
			std::map<location, t_translation::t_letter>::const_iterator itor =
				borderCache_.find(adj[n]);

			// Only add if it's in the cache and a valid terrain
			if(itor != borderCache_.end() &&
					itor->second != t_translation::NONE_TERRAIN)  {

				items[nitems] = itor->second;
				++nitems;
			}
		}

	}

	// Count all the terrain types found, 
	// and see which one is the most common, and use it.
	t_translation::t_letter used_terrain;
	int terrain_count = 0;
	for(int i = 0; i != nitems; ++i) {
		if(items[i] != used_terrain && !is_village(items[i]) && !is_keep(items[i])) {
			const int c = std::count(items+i+1,items+nitems,items[i]) + 1;
			if(c > terrain_count) {
				used_terrain = items[i];
				terrain_count = c;
			}
		}
	}

	borderCache_.insert(std::pair<location, t_translation::t_letter>(loc,used_terrain));
	return used_terrain;

}

const gamemap::location& gamemap::starting_position(int n) const
{
	if(size_t(n) < sizeof(startingPositions_)/sizeof(*startingPositions_)) {
		return startingPositions_[n];
	} else {
		static const gamemap::location null_loc;
		return null_loc;
	}
}

int gamemap::num_valid_starting_positions() const
{
	const int res = is_starting_position(gamemap::location());
	if(res == -1)
		return num_starting_positions()-1;
	else
		return res;
}

int gamemap::is_starting_position(const gamemap::location& loc) const
{
	const gamemap::location* const beg = startingPositions_+1;
	const gamemap::location* const end = startingPositions_+num_starting_positions();
	const gamemap::location* const pos = std::find(beg,end,loc);

	return pos == end ? -1 : pos - beg;
}

void gamemap::set_starting_position(int side, const gamemap::location& loc)
{
	if(side >= 0 && side < num_starting_positions()) {
		startingPositions_[side] = loc;
	}
}

const terrain_type& gamemap::get_terrain_info(const t_translation::t_letter terrain) const
{
	static const terrain_type default_terrain;
	const std::map<t_translation::t_letter,terrain_type>::const_iterator i =
		letterToTerrain_.find(terrain);

	if(i != letterToTerrain_.end())
		return i->second;
	else
		return default_terrain;
}

bool gamemap::location::matches_range(const std::string& xloc, const std::string &yloc) const
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
		if(dash != xloc.end()) {
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

		if(dash != yloc.end()) {
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

void gamemap::set_terrain(const gamemap::location& loc, const t_translation::t_letter terrain)
{
	if(!on_board(loc))
		return;

	const bool old_village = is_village(loc);
	const bool new_village = is_village(terrain);

	if(old_village && !new_village) {
		villages_.erase(std::remove(villages_.begin(),villages_.end(),loc),villages_.end());
	} else if(!old_village && new_village) {
		villages_.push_back(loc);
	}

	tiles_[loc.x][loc.y] = terrain;

	// Temp hack, since the border can have multiple tiles.
	// Depending on each other, every change has to invalidate the cache. 
	// Once the border tiles are part of the map, this hack is no longer required.
	borderCache_.clear();
	return;

	location adj[6];
	get_adjacent_tiles(loc,adj);

	for(int n = 0; n < 6; ++n) {
		remove_from_border_cache(adj[n]);
	}
}

std::vector<gamemap::location> parse_location_range(const std::string& x, const std::string& y,
													const gamemap *const map)
{
	std::vector<gamemap::location> res;
	const std::vector<std::string> xvals = utils::split(x);
	const std::vector<std::string> yvals = utils::split(y);

	for(unsigned int i = 0; i < xvals.size() || i < yvals.size(); ++i) {
		std::pair<int,int> xrange, yrange;

		//x
		if(i < xvals.size()) {
			xrange = utils::parse_range(xvals[i]);
		} else if (map != NULL) {
			xrange.first = 1;
			xrange.second = map->w();
		} else {
			break;
		}

		//y
		if(i < yvals.size()) {
			yrange = utils::parse_range(yvals[i]);
		} else if (map != NULL) {
			yrange.first = 1;
			yrange.second = map->h();
		} else {
			break;
		}

		for(int x = xrange.first; x <= xrange.second; ++x) {
			for(int y = yrange.first; y <= yrange.second; ++y) {
				res.push_back(gamemap::location(x-1,y-1));
			}
		}
	}
	return res;
}

const std::map<t_translation::t_letter, size_t>& gamemap::get_weighted_terrain_frequencies() const
{
	if(terrainFrequencyCache_.empty() == false) {
		return terrainFrequencyCache_;
	}

	const location center(w()/2,h()/2);

	const size_t furthest_distance = distance_between(location(0,0),center);

	const size_t weight_at_edge = 100;
	const size_t additional_weight_at_center = 200;

	for(size_t i = 0; i != size_t(w()); ++i) {
		for(size_t j = 0; j != size_t(h()); ++j) {
			const size_t distance = distance_between(location(i,j),center);
			terrainFrequencyCache_[(*this)[i][j]] += weight_at_edge + (furthest_distance-distance)*additional_weight_at_center;
		}
	}

	return terrainFrequencyCache_;
}

