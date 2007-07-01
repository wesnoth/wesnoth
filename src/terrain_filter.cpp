/* $Id: terrain_filter.cpp 18589 2007-07-01 04:41:00Z esr $ */
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
#include "pathfind.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "wassert.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>

#define ERR_CF LOG_STREAM(err, config)
#define LOG_G LOG_STREAM(info, general)

namespace {
	struct terrain_cache_manager {
		terrain_cache_manager() : ptr(NULL) {}
		~terrain_cache_manager() { delete ptr; }
		t_translation::t_match *ptr;
	};

	struct cfg_isor {
		bool operator() (std::pair<const std::string*,const config*> val) {
			return *(val.first) == "or";
		}
	};

} //end anonymous namespace

static bool terrain_matches_internal(const gamemap *mundi, const gamemap::location& loc, const vconfig& cfg, 
		const gamestatus& game_status, const unit_map& units, const bool flat_tod, 
		const bool ignore_xy, t_translation::t_match*& parsed_terrain)
{

	const int terrain_format = lexical_cast_default(cfg["terrain_format"], -1);
	if(terrain_format != -1) {
		lg::wml_error << "key terrain_format in filter_location is no longer used, this message will disappear in 1.3.5\n";
	}

	if(cfg.has_attribute("terrain")) {
		if(parsed_terrain == NULL) {
			parsed_terrain = new t_translation::t_match(cfg["terrain"]);
		}
		if(!parsed_terrain->is_empty) {
			const t_translation::t_letter letter = mundi->get_terrain_info(loc).number();
			if(!t_translation::terrain_matches(letter, *parsed_terrain)) {
				return false;
			}
		}
	}
	
	//Allow filtering on location ranges 
	if(!ignore_xy && !(cfg["x"].empty() && cfg["y"].empty())){
		if(!loc.matches_range(cfg["x"], cfg["y"])) {
			return false;
		}
	}

	//Allow filtering on unit
	if(cfg.has_child("filter")) {
		const vconfig& unit_filter = cfg.child("filter");
		const unit_map::const_iterator u = units.find(loc);
		if (u == units.end() || !u->second.matches_filter(unit_filter, loc, flat_tod))
			return false;
	}

	const t_string& t_tod_type = cfg["time_of_day"];
	const t_string& t_tod_id = cfg["time_of_day_id"];
	const std::string& tod_type = t_tod_type;
	const std::string& tod_id = t_tod_id;
	static config const dummy_cfg;
	time_of_day tod(dummy_cfg);
	if(!tod_type.empty() || !tod_id.empty()) {
		if(flat_tod) {
			tod = game_status.get_time_of_day(0,loc);
		} else {
			gamemap mapcopy = *mundi;
			tod = timeofday_at(game_status,units,loc, mapcopy);
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

	//allow filtering on owner (for villages)
	const t_string& t_owner_side = cfg["owner_side"];
	const std::string& owner_side = t_owner_side;
	if(!owner_side.empty()) {
		const int side_index = lexical_cast_default<int>(owner_side,0) - 1;
		wassert(game_status.teams != NULL);
		if(village_owner(loc, *(game_status.teams)) != side_index) {
			return false;
		}
	}
	return true; 
}

bool terrain_matches_filter(const gamemap *mundi, const gamemap::location& loc, const vconfig& cfg, 
		const gamestatus& game_status, const unit_map& units, const bool flat_tod,
		const size_t max_loop)
{
	//handle radius
	const size_t radius = minimum<size_t>(max_loop,
		lexical_cast_default<size_t>(cfg["radius"], 0));
	std::set<gamemap::location> hexes;
	std::vector<gamemap::location> loc_vec(1, loc);
	gamemap mapcopy = *mundi;
	get_tiles_radius(mapcopy, loc_vec, radius, hexes);

	size_t loop_count = 0;
	bool matches = false;
	std::set<gamemap::location>::const_iterator i;
	terrain_cache_manager tcm;
	for(i = hexes.begin(); i != hexes.end() && loop_count <= max_loop && !matches; ++i) {
		matches = terrain_matches_internal(mundi, *i, cfg, game_status, units, flat_tod, false, tcm.ptr);
		++loop_count;
	}

	//handle [and], [or], and [not] with in-order precedence
	config::all_children_iterator cond = cfg.get_config().ordered_begin();
	config::all_children_iterator cond_end = cfg.get_config().ordered_end();
	int ors_left = std::count_if(cond, cond_end, cfg_isor());
	while(cond != cond_end)
	{
		//if there are no matches or [or] conditions left, go ahead and return false
		if(!matches && ors_left <= 0) {
			return false;
		}

		const std::string& cond_name = *((*cond).first);
		const vconfig cond_filter(&(*((*cond).second)));

		//handle [and]
		if(cond_name == "and")
		{
			matches = matches &&
				terrain_matches_filter(mundi, loc, cond_filter, game_status, units, flat_tod, max_loop);
		}
		//handle [or]
		else if(cond_name == "or")
		{
			matches = matches ||
				terrain_matches_filter(mundi, loc, cond_filter, game_status, units, flat_tod, max_loop);
			--ors_left;
		}
		//handle [not]
		else if(cond_name == "not")
		{
			matches = matches &&
				!terrain_matches_filter(mundi, loc, cond_filter, game_status, units, flat_tod, max_loop);
		}

		++cond;
	}

	return matches;
}

void get_locations(const gamemap *mundi, std::set<gamemap::location>& locs, const vconfig& filter,
		const gamestatus& game_status, const unit_map& units, const bool flat_tod,
		const size_t max_loop)
{
	std::vector<gamemap::location> xy_locs = parse_location_range(filter["x"],filter["y"], mundi);
	if(xy_locs.empty()) {
		//consider all locations on the map
		for(int x=0; x < mundi->xsize(); x++) {
			for(int y=0; y < mundi->ysize(); y++) {
				xy_locs.push_back(gamemap::location(x,y));
			}
		}
	}

	//handle location filter
	terrain_cache_manager tcm;
	std::vector<gamemap::location>::iterator loc_itor = xy_locs.begin();
	while(loc_itor != xy_locs.end()) {
		if(terrain_matches_internal(mundi, *loc_itor, filter, game_status, units, flat_tod, true, tcm.ptr)) {
			++loc_itor;
		} else {
			loc_itor = xy_locs.erase(loc_itor);
		}
	}

	//handle radius
	const size_t radius = minimum<size_t>(max_loop,
		lexical_cast_default<size_t>(filter["radius"], 0));
	const gamemap mapcopy = *mundi; 
	get_tiles_radius(mapcopy, xy_locs, radius, locs);

	//handle [and], [or], and [not] with in-order precedence
	config::all_children_iterator cond = filter.get_config().ordered_begin();
	config::all_children_iterator cond_end = filter.get_config().ordered_end();
	int ors_left = std::count_if(cond, cond_end, cfg_isor());
	while(cond != cond_end)
	{
		//if there are no locations or [or] conditions left, go ahead and return empty
		if(locs.empty() && ors_left <= 0) {
			return;
		}

		const std::string& cond_name = *((*cond).first);
		const vconfig cond_filter(&(*((*cond).second)));

		//handle [and]
		if(cond_name == "and") {
			std::set<gamemap::location> intersect_hexes;
			get_locations(mundi, intersect_hexes, cond_filter, game_status, units, flat_tod, max_loop);
			std::set<gamemap::location>::iterator intersect_itor = locs.begin();
			while(intersect_itor != locs.end()) {
				if(intersect_hexes.find(*intersect_itor) == locs.end()) {
					locs.erase(*intersect_itor++);
				} else {
					++intersect_itor;
				}
			}
		}
		//handle [or]
		else if(cond_name == "or") {
			std::set<gamemap::location> union_hexes;
			get_locations(mundi, union_hexes, cond_filter, game_status, units, flat_tod, max_loop);
			//locs.insert(union_hexes.begin(), union_hexes.end()); //doesn't compile on MSVC
			std::set<gamemap::location>::iterator insert_itor = union_hexes.begin();
			while(insert_itor != union_hexes.end()) {
				locs.insert(*insert_itor++);
			}
			--ors_left;
		}
		//handle [not]
		else if(cond_name == "not") {
			std::set<gamemap::location> removal_hexes;
			get_locations(mundi, removal_hexes, cond_filter, game_status, units, flat_tod, max_loop);
			std::set<gamemap::location>::iterator erase_itor = removal_hexes.begin();
			while(erase_itor != removal_hexes.end()) {
				locs.erase(*erase_itor++);
			}
		}

		++cond;
	}

	//restrict the potential number of locations to be returned
	if(locs.size() > max_loop + 1) {
		std::set<gamemap::location>::iterator erase_itor = locs.begin();
		for(unsigned i=0; i < max_loop + 1; ++i) {
			++erase_itor;
		}
		locs.erase(erase_itor, locs.end());
	}
}


