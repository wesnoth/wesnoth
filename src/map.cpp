/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "config.hpp"
#include "log.hpp"
#include "map.hpp"
#include "pathutils.hpp"
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

const std::string& gamemap::underlying_terrain(TERRAIN terrain) const
{
	const std::map<TERRAIN,terrain_type>::const_iterator i = letterToTerrain_.find(terrain);
	if(i == letterToTerrain_.end()) {
		static std::string res;
		res.resize(1);
		res[0] = terrain;
		return res;
	} else {
		return i->second.type();
	}
}

bool gamemap::is_village(gamemap::TERRAIN terrain) const
{
	return get_terrain_info(terrain).is_village();
}

bool gamemap::gives_healing(gamemap::TERRAIN terrain) const
{
	return get_terrain_info(terrain).gives_healing();
}

bool gamemap::is_castle(gamemap::TERRAIN terrain) const
{
	return get_terrain_info(terrain).is_castle();
}

bool gamemap::is_keep(gamemap::TERRAIN terrain) const
{
	return get_terrain_info(terrain).is_keep();
}

bool gamemap::is_village(const gamemap::location& loc) const
{
	return on_board(loc) && is_village(get_terrain(loc));
}

bool gamemap::gives_healing(const gamemap::location& loc) const
{
	return on_board(loc) && gives_healing(get_terrain(loc));
}

bool gamemap::is_castle(const gamemap::location& loc) const
{
	return on_board(loc) && is_castle(get_terrain(loc));
}

bool gamemap::is_keep(const gamemap::location& loc) const
{
	return on_board(loc) && is_keep(get_terrain(loc));
}

bool gamemap::filter_location(const gamemap::location &loc,const config &con) const
{ //need to fill this in
  return on_board(loc);
}

gamemap::location::DIRECTION gamemap::location::parse_direction(const std::string& str)
{
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
	} else {
		return NDIRECTIONS;
	}
}

gamemap::location::location(const config& cfg) : x(-1), y(-1)
{
	const std::string& xstr = cfg["x"];
	const std::string& ystr = cfg["y"];

	//the co-ordinates in config files will be 1-based, while we
	//want them as 0-based
	if(xstr.empty() == false)
		x = atoi(xstr.c_str()) - 1;

	if(ystr.empty() == false)
		y = atoi(ystr.c_str()) - 1;
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
	if( diff.y < 0 && diff.x < 0 && abs(diff.x) >= abs(diff.y)) return NORTH_WEST;
	if( diff.y < 0 && abs(diff.x) < abs(diff.y)) return NORTH;

	if( diff.y >= 0 && diff.x >= 0 && abs(diff.x) >= abs(diff.y)) return SOUTH_EAST;
	if( diff.y >= 0 && diff.x < 0 && abs(diff.x) >= abs(diff.y)) return SOUTH_WEST;
	if( diff.y >= 0 && abs(diff.x) < abs(diff.y)) return SOUTH;
	
	//impossible
	wassert(false);
	return NDIRECTIONS;


}
gamemap::gamemap(const config& cfg, const std::string& data) : tiles_(1)
{
	LOG_G << "loading map: '" << data << "'\n";
	const config::child_list& terrains = cfg.get_children("terrain");
	create_terrain_maps(terrains,terrainList_,letterToTerrain_,terrain_);

	read(data);
}

void gamemap::read(const std::string& data)
{
	tiles_.clear();
	villages_.clear();
	std::fill(startingPositions_,startingPositions_+sizeof(startingPositions_)/sizeof(*startingPositions_),location());

	//ignore leading newlines
	std::string::const_iterator i = data.begin();
	while(i != data.end() && (*i == '\r' || *i == '\n')) {
		++i;
	}

	size_t x = 0, y = 0;
	for(; i != data.end(); ++i) {
		char c = *i;
		if(c == '\r') {
			continue;
		} if(c == '\n') {
			x = 0;
			++y;
		} else {
			if(letterToTerrain_.count(c) == 0) {
				if(isdigit(*i)) {
					startingPositions_[c - '0'] = location(x,y);
					c = KEEP;
				} else {
					ERR_CF << "Illegal character in map: (" << int(c) << ") '" << c << "'\n";
					throw incorrect_format_exception("Illegal character found in map. The scenario cannot be loaded.");
				}
			}

			if(is_village(c)) {
				villages_.push_back(location(int(x),int(y)));
			}

			if(x >= tiles_.size()) {
				tiles_.resize(x+1);
			}

			tiles_[x].push_back(c);

			++x;
		}
	}

	unsigned ysize = this->y();
	for(size_t n = 0; n != tiles_.size(); ++n) { // tiles_.size() is not constant
		if (tiles_[n].size() != ysize) {
			ERR_CF << "Map is not rectangular!\n";
			tiles_.erase(tiles_.begin()+n);
			--n;
		}
	}

	LOG_G << "loaded map: " << this->x() << ',' << ysize << '\n';
}

std::string gamemap::write() const
{
	std::stringstream str;
	for(int j = 0; j != y(); ++j) {
		for(int i = 0; i != x(); ++i) {
			int n;
			for(n = 0; n != STARTING_POSITIONS; ++n) {
				if(startingPositions_[n] == location(i,j))
					break;
			}

			if(n < STARTING_POSITIONS) {
				str << n;
			} else {
				str << tiles_[i][j];
			}
		}

		str << "\n";
	}

	return str.str();
}

void gamemap::overlay(const gamemap& m, const config& rules_cfg, const int xpos, const int ypos)
{
	const config::child_list& rules = rules_cfg.get_children("rule");

	const int xend = minimum<int>(xpos+m.x(),x());
	const int yend = minimum<int>(ypos+m.y(),y());
	for(int x = xpos; x < xend; ++x) {
		for(int y = ypos; y < yend; ++y) {
			const TERRAIN t = m[x-xpos][y-ypos];
			const TERRAIN current = (*this)[x][y];

			if(t == FOGGED || t == VOID_TERRAIN) {
				continue;
			}

			//see if there is a matching rule
			config::child_list::const_iterator rule = rules.begin();
			for( ; rule != rules.end(); ++rule) {
				static const std::string src_key = "old", src_not_key = "old_not",
				                         dst_key = "new", dst_not_key = "new_not";
				const config& cfg = **rule;
				const std::string& src = cfg[src_key];
				if(src != "" && std::find(src.begin(),src.end(),current) == src.end()) {
					continue;
				}

				const std::string& src_not = cfg[src_not_key];
				if(src_not != "" && std::find(src_not.begin(),src_not.end(),current) != src_not.end()) {
					continue;
				}

				const std::string& dst = cfg[dst_key];
				if(dst != "" && std::find(dst.begin(),dst.end(),t) == dst.end()) {
					continue;
				}

				const std::string& dst_not = cfg[dst_not_key];
				if(dst_not != "" && std::find(dst_not.begin(),dst_not.end(),t) != dst_not.end()) {
					continue;
				}

				break;
			}


			if(rule != rules.end()) {
				const config& cfg = **rule;
				const std::string& terrain = cfg["terrain"];
				if(terrain != "") {
					set_terrain(location(x,y),terrain[0]);
				} else if(cfg["use_old"] != "yes") {
					set_terrain(location(x,y),t);
				}
			} else {
				set_terrain(location(x,y),t);
			}
		}
	}

	for(const location* pos = m.startingPositions_; pos != m.startingPositions_ + sizeof(m.startingPositions_)/sizeof(*m.startingPositions_); ++pos) {
		if(pos->valid()) {
			startingPositions_[pos - m.startingPositions_] = *pos;
		}
	}

}

int gamemap::x() const { return tiles_.size(); }
int gamemap::y() const { return tiles_.empty() ? 0 : tiles_.front().size(); }

const std::vector<gamemap::TERRAIN>& gamemap::operator[](int index) const
{
	return tiles_[index];
}

gamemap::TERRAIN gamemap::get_terrain(const gamemap::location& loc) const
{
	if(on_board(loc))
		return tiles_[loc.x][loc.y];

	const std::map<location,TERRAIN>::const_iterator itor = borderCache_.find(loc);
	if(itor != borderCache_.end())
		return itor->second;

	//if not on the board, decide based on what surrounding terrain is
	TERRAIN items[6];
	int nitems = 0;

	location adj[6];
	get_adjacent_tiles(loc,adj);
	for(int n = 0; n != 6; ++n) {
		if(on_board(adj[n])) {
			items[nitems] = tiles_[adj[n].x][adj[n].y];
			++nitems;
		}
	}

	//count all the terrain types found, and see which one
	//is the most common, and use it.
	TERRAIN used_terrain = 0;
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

	borderCache_.insert(std::pair<location,TERRAIN>(loc,used_terrain));

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

int gamemap::num_starting_positions() const
{
	return sizeof(startingPositions_)/sizeof(*startingPositions_);
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

const terrain_type& gamemap::get_terrain_info(TERRAIN terrain) const
{
	static const terrain_type default_terrain;
	const std::map<TERRAIN,terrain_type>::const_iterator i =
	                                letterToTerrain_.find(terrain);
	if(i != letterToTerrain_.end())
		return i->second;
	else
		return default_terrain;
}

const terrain_type& gamemap::get_terrain_info(const gamemap::location &loc) const
{
	return get_terrain_info(get_terrain(loc));
}

const std::vector<gamemap::TERRAIN>& gamemap::get_terrain_list() const
{
	return terrainList_;
}

void gamemap::set_terrain(const gamemap::location& loc, gamemap::TERRAIN ter)
{
	if(!on_board(loc))
		return;

	const bool old_village = is_village(loc);
	const bool new_village = is_village(ter);

	if(old_village && !new_village) {
		villages_.erase(std::remove(villages_.begin(),villages_.end(),loc),villages_.end());
	} else if(!old_village && new_village) {
		villages_.push_back(loc);
	}

	//If the terrain is set under a starting position, do also erase the
	//starting position.
	for(int i = 0; i < STARTING_POSITIONS; ++i) {
		if(loc == startingPositions_[i])
			startingPositions_[i] = location();
	}

	tiles_[loc.x][loc.y] = ter;

	location adj[6];
	get_adjacent_tiles(loc,adj);

	for(int n = 0; n < 6; ++n)
		remove_from_border_cache(adj[n]);
}

std::vector<gamemap::location> parse_location_range(const std::string& x, const std::string& y)
{
	std::vector<gamemap::location> res;
	const std::vector<std::string> xvals = utils::split(x);
	const std::vector<std::string> yvals = utils::split(y);

	for(unsigned int i = 0; i != minimum(xvals.size(),yvals.size()); ++i) {
		const std::pair<int,int> xrange = utils::parse_range(xvals[i]);
		const std::pair<int,int> yrange = utils::parse_range(yvals[i]);

		for(int x = xrange.first; x <= xrange.second; ++x) {
			for(int y = yrange.first; y <= yrange.second; ++y) {
				res.push_back(gamemap::location(x-1,y-1));
			}
		}
	}

	return res;
}

const std::map<gamemap::TERRAIN,size_t>& gamemap::get_weighted_terrain_frequencies() const
{
	if(terrainFrequencyCache_.empty() == false) {
		return terrainFrequencyCache_;
	}

	const location center(x()/2,y()/2);

	const size_t furthest_distance = distance_between(location(0,0),center);

	const size_t weight_at_edge = 100;
	const size_t additional_weight_at_center = 200;

	for(size_t i = 0; i != size_t(x()); ++i) {
		for(size_t j = 0; j != size_t(y()); ++j) {
			const size_t distance = distance_between(location(i,j),center);
			terrainFrequencyCache_[(*this)[i][j]] += weight_at_edge + (furthest_distance-distance)*additional_weight_at_center;
		}
	}

	return terrainFrequencyCache_;
}

void gamemap::remove_from_border_cache(const location &loc) {
	borderCache_.erase(loc);
}

