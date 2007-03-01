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

#include "actions.hpp"
#include "config.hpp"
#include "gamestatus.hpp"
#include "log.hpp"
#include "map.hpp"
#include "pathutils.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "wassert.hpp"
#include "game_events.hpp"
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

void gamemap::location::init(const std::string &xstr, const std::string &ystr)
{
	std::string xs = xstr, ys = ystr;
	if (game_events::get_state_of_game())
	{
		xs = utils::interpolate_variables_into_string( xs, *game_events::get_state_of_game());
		ys = utils::interpolate_variables_into_string( ys, *game_events::get_state_of_game());
	}
	//the co-ordinates in config files will be 1-based, while we
	//want them as 0-based
	if(xs.empty() == false)
		x = atoi(xs.c_str()) - 1;

	if(ys.empty() == false)
		y = atoi(ys.c_str()) - 1;
}

gamemap::location::location(const config& cfg) : x(-1), y(-1)
{
	init(cfg["x"], cfg["y"]);
}

gamemap::location::location(const vconfig& cfg) : x(-1), y(-1)
{
	init(cfg["x"], cfg["y"]);
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
gamemap::location::DIRECTION gamemap::location::get_opposite_dir(gamemap::location::DIRECTION d) const {
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

gamemap::gamemap(const config& cfg, const std::string& data) : tiles_(1), x_(-1), y_(-1)
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
		// we re-throw the error but as map error, since all codepaths test 
		// for this, it's the least work
		throw incorrect_format_exception(e.message.c_str());
	}

	//convert the starting positions to the array
	std::map<int, t_translation::coordinate>::const_iterator itor = 
		starting_positions.begin();

	for(; itor != starting_positions.end(); ++itor) {

		// check for valid position, the first valid position is
		// 1 so the offset 0 in the array is never used
		if(itor->first < 1 || itor->first >= STARTING_POSITIONS) { 
			ERR_CF << "Starting position " << itor->first << " out of range\n"; 
			throw incorrect_format_exception("Illegal starting position found in map. The scenario cannot be loaded.");
		}

		// add to the starting position array
		startingPositions_[itor->first] = location(itor->second.x, itor->second.y);
	}
	
	// post processing on the map
	const int width = tiles_.size();
	const int height = tiles_[0].size();
    x_ = width;
    y_ = height;
	for(int x = 0; x < width; ++x) {
		for(int y = 0; y < height; ++y) {
			
			// is the terrain valid? 
			if(letterToTerrain_.count(tiles_[x][y]) == 0) {
				ERR_CF << "Illegal character in map: (" << t_translation::write_letter(tiles_[x][y]) 
					<< ") '" << tiles_[x][y] << "'\n"; 
				throw incorrect_format_exception("Illegal character found in map. The scenario cannot be loaded.");
			} 
			
			// is it a village
			if(is_village(tiles_[x][y])) {
				villages_.push_back(location(x, y));
			}
		}
	}
}

std::string gamemap::write() const
{
	std::map<int, t_translation::coordinate> starting_positions = std::map<int, t_translation::coordinate>();

	// convert the starting positions to a map
	for(int i = 0; i < STARTING_POSITIONS; ++i) {
    	if(on_board(startingPositions_[i])) {
			const struct t_translation::coordinate position = 
				{startingPositions_[i].x, startingPositions_[i].y};
			
			 starting_positions.insert(std::pair<int, t_translation::coordinate>(i, position));
		}
	}
	
	// let the low level convertor do the conversion
	return t_translation::write_game_map(tiles_, starting_positions);
}

void gamemap::overlay(const gamemap& m, const config& rules_cfg, const int xpos, const int ypos)
{
	const config::child_list& rules = rules_cfg.get_children("rule");

	const int xstart = maximum<int>(0, -xpos);
	const int ystart = maximum<int>(0, -ypos-((xpos & 1) ? 1 : 0));
	const int xend = minimum<int>(m.x(),x()-xpos);
	const int yend = minimum<int>(m.y(),y()-ypos);
	for(int x1 = xstart; x1 < xend; ++x1) {
		for(int y1 = ystart; y1 < yend; ++y1) {
			const int x2 = x1 + xpos;
			const int y2 = y1 + ypos +
				((xpos & 1) && (x1 & 1) ? 1 : 0);
			if (y2 < 0 || y2 >= y()) {
				continue;
			}
			const t_translation::t_letter t = m[x1][y1];
			const t_translation::t_letter current = (*this)[x2][y2];

			if(t == t_translation::FOGGED || t == t_translation::VOID_TERRAIN) {
				continue;
			}

			//see if there is a matching rule
			config::child_list::const_iterator rule = rules.begin();
			for( ; rule != rules.end(); ++rule) {
				static const std::string src_key = "old", src_not_key = "old_not",
				                         dst_key = "new", dst_not_key = "new_not";
				const config& cfg = **rule;
				const t_translation::t_list& src = 
					t_translation::read_list(cfg[src_key], 0, t_translation::T_FORMAT_AUTO);

				if(!src.empty() && std::find(src.begin(),src.end(),current) == src.end()) {
					continue;
				}

				const t_translation::t_list& src_not = 
					t_translation::read_list(cfg[src_not_key], 0, t_translation::T_FORMAT_AUTO);

				if(!src_not.empty() && std::find(src_not.begin(),src_not.end(),current) != src_not.end()) {
					continue;
				}

				const t_translation::t_list& dst = 
					t_translation::read_list(cfg[dst_key], 0, t_translation::T_FORMAT_AUTO);

				if(!dst.empty() && std::find(dst.begin(),dst.end(),t) == dst.end()) {
					continue;
				}

				const t_translation::t_list& dst_not = 
					t_translation::read_list(cfg[dst_not_key], 0, t_translation::T_FORMAT_AUTO);

				if(!dst_not.empty() && std::find(dst_not.begin(),dst_not.end(),t) != dst_not.end()) {
					continue;
				}

				break;
			}


			if(rule != rules.end()) {
				const config& cfg = **rule;
				const t_translation::t_list& terrain = 
					t_translation::read_list(cfg["terrain"], 0, t_translation::T_FORMAT_AUTO);

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
	if(on_board(loc))
		return tiles_[loc.x][loc.y];

	const std::map<location, t_translation::t_letter>::const_iterator itor = borderCache_.find(loc);
	if(itor != borderCache_.end())
		return itor->second;

	//if not on the board, decide based on what surrounding terrain is
	t_translation::t_letter items[6];
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
	t_translation::t_letter used_terrain = 0;
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

bool gamemap::terrain_matches_filter(const gamemap::location& loc, const config& cfg, 
		const gamestatus& game_status, const unit_map& units, const bool flat_tod) const
{
	/* *
	 * The abilities use a comma separated list of terrains, this code has been
	 * used and also needs to be backwards compatible. This should happen 
	 * independant of the map so added this hack. Obiously it needs to be removed
	 * as soon as possible.
	 */
	const int terrain_format = lexical_cast_default(cfg["terrain_format"], -1);

/*  enable when the hack is no longer used	
	if(terrain_format != -1) {
		std::cerr << "key terrain_format in filter_location is obsolete old format no longer supported";
	}
*/	
#ifdef TERRAIN_TRANSLATION_COMPATIBLE
	if(terrain_format == 0 || terrain_format == -1 && !cfg["terrain"].empty()) {
		std::cerr << "Warning deprecated terrain format in filter_location \n";
		const std::string& terrain = cfg["terrain"];
		// Any of these may be a CSV
		std::string terrain_letter;
		terrain_letter += t_translation::get_old_letter(get_terrain_info(loc).number());
		if(!terrain.empty() && !terrain_letter.empty()) {
			if(terrain != terrain_letter) {
				if(std::find(terrain.begin(),terrain.end(),',') != terrain.end() &&
					std::search(terrain.begin(),terrain.end(),
					terrain_letter.begin(),terrain_letter.end()) != terrain.end()) {
					const std::vector<std::string>& vals = utils::split(terrain);
					if(std::find(vals.begin(),vals.end(),terrain_letter) == vals.end()) {
						return false;
					}
				} else {
					return false;
				}
			}
		}
	} else {
#endif
		const t_translation::t_list& terrain = 
			t_translation::read_list(cfg["terrain"], -1, t_translation::T_FORMAT_STRING);
		if(! terrain.empty()) {

			const t_translation::t_letter letter = get_terrain_info(loc).number();
			if(! t_translation::terrain_matches(letter, terrain)) {
					return false;
			}
		}
#ifdef TERRAIN_TRANSLATION_COMPATIBLE
	}
#endif
	
	//Allow filtering on location ranges 
	if(!cfg["x"].empty() && !cfg["y"].empty()){
	  bool found=false;
	  std::vector<gamemap::location> locs = parse_location_range(cfg["x"],cfg["y"]);
	  for(std::vector<gamemap::location>::iterator ll = locs.begin(); ll != locs.end(); ll++){
	    if((*ll)==loc){
	      found=true;
	      break;
	    }
	  }
	  if(!found)return(false);
	}

	const std::string& tod_type = cfg["time_of_day"];
	const std::string& tod_id = cfg["time_of_day_id"];
	static config const dummy_cfg;
	time_of_day tod(dummy_cfg);
	if(!tod_type.empty() || !tod_id.empty()) {
		if(flat_tod) {
			tod = game_status.get_time_of_day(0,loc);
		} else {
			tod = timeofday_at(game_status,units,loc,*this);
		}
	}
	if(!tod_type.empty()) {
		const std::vector<std::string>& vals = utils::split(tod_type);
		if(tod.lawful_bonus<0) {
			if(std::find(vals.begin(),vals.end(),"chaotic") == vals.end()) {
				return false;
			}
		} else if(tod.lawful_bonus>0) {
			if(std::find(vals.begin(),vals.end(),"lawful") == vals.end()) {
				return false;
			}
		} else {
			if(std::find(vals.begin(),vals.end(),"neutral") == vals.end()) {
				return false;
			}
		}
	}
	if(!tod_id.empty()) {
		if(tod_id != tod.id) {
			if(std::find(tod_id.begin(),tod_id.end(),',') != tod_id.end() &&
				std::search(tod_id.begin(),tod_id.end(),
				tod.id.begin(),tod.id.end()) != tod_id.end()) {
				const std::vector<std::string>& vals = utils::split(tod_id);
				if(std::find(vals.begin(),vals.end(),tod.id) == vals.end()) {
					return false;
				}
			} else {
				return false;
			}
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

	//If the terrain is set under a starting position, do also erase the
	//starting position.
	for(int i = 0; i < STARTING_POSITIONS; ++i) {
		if(loc == startingPositions_[i])
			startingPositions_[i] = location();
	}

	tiles_[loc.x][loc.y] = terrain;

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

const std::map<t_translation::t_letter, size_t>& gamemap::get_weighted_terrain_frequencies() const
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
