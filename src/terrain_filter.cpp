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

#include "global.hpp"

#include "actions.hpp"
#include "config.hpp"
#include "gamestatus.hpp"
#include "log.hpp"
#include "terrain_filter.hpp"
#include "util.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>

#define ERR_CF LOG_STREAM(err, config)
#define LOG_G LOG_STREAM(info, general)
#define ERR_NG LOG_STREAM(err, engine)


terrain_filter::terrain_filter(const vconfig& cfg, const gamemap& map, 
	const gamestatus& game_status, const unit_map& units, const bool flat_tod,
	const size_t max_loop) : cfg_(cfg), map_(map), status_(game_status), units_(units)
{
	restrict(max_loop);
	flatten(flat_tod);
}

terrain_filter::terrain_filter(const vconfig& cfg, const terrain_filter& original)
	: cfg_(cfg), map_(original.map_), status_(original.status_), units_(original.units_)
{
	restrict(original.max_loop_);
	flatten(original.flat_);
}

terrain_filter::terrain_filter(const terrain_filter& other) : cfg_(other.cfg_),
	map_(other.map_), status_(other.status_), units_(other.units_)
{
	restrict(other.max_loop_);
	flatten(other.flat_);
}

terrain_filter& terrain_filter::operator=(const terrain_filter& other)
{
	// Use copy constructor to make sure we are coherant
	if (this != &other) {
		this->~terrain_filter();
		new (this) terrain_filter(other) ;
	}
	return *this ;
}

namespace {
	struct cfg_isor {
		bool operator() (std::pair<const std::string*,const config*> val) {
			return *(val.first) == "or";
		}
	};
} //end anonymous namespace

bool terrain_filter::match_internal(const gamemap::location& loc, const bool ignore_xy)
{
	if(cfg_.has_attribute("terrain")) {
		if(cache_.parsed_terrain == NULL) {
			cache_.parsed_terrain = new t_translation::t_match(cfg_["terrain"]);
		}
		if(!cache_.parsed_terrain->is_empty) {
			const t_translation::t_letter letter = map_.get_terrain_info(loc).number();
			if(!t_translation::terrain_matches(letter, *cache_.parsed_terrain)) {
				return false;
			}
		}
	}

	//Allow filtering on location ranges
	if(!ignore_xy) {
		if(!loc.matches_range(cfg_["x"], cfg_["y"])) {
			return false;
		}
		//allow filtering by searching a stored variable of locations
		if(cfg_.has_attribute("find_in")) {
			variable_info vi(cfg_["find_in"], false, variable_info::TYPE_CONTAINER);
			if(!vi.is_valid) return false;
			if(vi.explicit_index) {
				if(gamemap::location(vi.as_container(),NULL) != loc) {
					return false;
				}
			} else {
				variable_info::array_range a_range;
				for(a_range = vi.as_array(); a_range.first != a_range.second; ++a_range.first) {
					if(gamemap::location(**a_range.first,NULL) == loc) {
						break;
					}
				}
				if(a_range.first == a_range.second) {
					return false;
				}
			}
		}
	}

	//Allow filtering on unit
	if(cfg_.has_child("filter")) {
		const vconfig& unit_filter = cfg_.child("filter");
		const unit_map::const_iterator u = units_.find(loc);
		if (u == units_.end() || !u->second.matches_filter(unit_filter, loc, flat_))
			return false;
	}

	//Allow filtering on adjacent locations
	if(cfg_.has_child("filter_adjacent_location")) {
		gamemap::location adjacent[6];
		get_adjacent_tiles(loc, adjacent);
		const vconfig::child_list& adj_cfgs = cfg_.get_children("filter_adjacent_location");
		vconfig::child_list::const_iterator i, i_end, i_begin = adj_cfgs.begin();
		for (i = i_begin, i_end = adj_cfgs.end(); i != i_end; ++i) {
			int match_count = 0;
			vconfig::child_list::difference_type index = i - i_begin;
			std::string adj_dirs = (*i).has_attribute("adjacent") ? (*i)["adjacent"]
				: "n,ne,se,s,sw,nw";
			static std::vector<gamemap::location::DIRECTION> default_dirs
				= gamemap::location::parse_directions("n,ne,se,s,sw,nw");
			std::vector<gamemap::location::DIRECTION> dirs = (*i).has_attribute("adjacent")
				? gamemap::location::parse_directions((*i)["adjacent"]) : default_dirs;
			std::vector<gamemap::location::DIRECTION>::const_iterator j, j_end = dirs.end();
			for (j = dirs.begin(); j != j_end; ++j) {
				gamemap::location &adj = adjacent[*j];
				if(map_.on_board(adj)) {
					if(cache_.adjacent_matches == NULL) {
						while(index >= std::distance(cache_.adjacent_match_cache.begin(), cache_.adjacent_match_cache.end())) {
							const vconfig& adj_cfg = adj_cfgs[cache_.adjacent_match_cache.size()];
							std::pair<terrain_filter, std::map<gamemap::location,bool> > amc_pair(
								terrain_filter(adj_cfg, *this),
								std::map<gamemap::location,bool>());
							cache_.adjacent_match_cache.push_back(amc_pair);
						}
						terrain_filter &amc_filter = cache_.adjacent_match_cache[index].first;
						std::map<gamemap::location,bool> &amc = cache_.adjacent_match_cache[index].second;
						std::map<gamemap::location,bool>::iterator lookup = amc.find(adj);
						if(lookup == amc.end()) {
							if(amc_filter(adj)) {
								amc[adj] = true;
								++match_count;
							} else {
								amc[adj] = false;
							}
						} else if(lookup->second) {
							++match_count;
						}
					} else {
						assert(index < std::distance(cache_.adjacent_matches->begin(), cache_.adjacent_matches->end()));
						std::set<gamemap::location> &amc = (*cache_.adjacent_matches)[index];
						if(amc.find(adj) != amc.end()) {
							++match_count;
						}
					}
				}
			}
			static std::vector<std::pair<int,int> > default_counts = utils::parse_ranges("1-6");
			std::vector<std::pair<int,int> > counts = (*i).has_attribute("count") 
				? utils::parse_ranges((*i)["count"]) : default_counts;
			std::vector<std::pair<int,int> >::const_iterator count, count_end = counts.end();
			bool count_matches = false;
			for (count = counts.begin(); count != count_end && !count_matches; ++count) {
				if(count->first <= match_count && match_count <= count->second) {
					count_matches = true;
				}
			}
			if(!count_matches) {
				return false;
			}
		}
	}

	const t_string& t_tod_type = cfg_["time_of_day"];
	const t_string& t_tod_id = cfg_["time_of_day_id"];
	const std::string& tod_type = t_tod_type;
	const std::string& tod_id = t_tod_id;
	static config const dummy_cfg;
	time_of_day tod(dummy_cfg);
	if(!tod_type.empty() || !tod_id.empty()) {
		if(flat_) {
			tod = status_.get_time_of_day(0,loc);
		} else {
			tod = timeofday_at(status_,units_,loc, map_);
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
	const t_string& t_owner_side = cfg_["owner_side"];
	const std::string& owner_side = t_owner_side;
	if(!owner_side.empty()) {
		const int side_index = lexical_cast_default<int>(owner_side,0) - 1;
		assert(status_.teams != NULL);
		if(village_owner(loc, *(status_.teams)) != side_index) {
			return false;
		}
	}

	return true;
}

bool terrain_filter::match(const gamemap::location& loc)
{
	if(cfg_["x"] == "recall" && cfg_["y"] == "recall") {
		return !map_.on_board(loc);
	}
	std::set<gamemap::location> hexes;
	std::vector<gamemap::location> loc_vec(1, loc);

	//handle radius
	size_t radius = lexical_cast_default<size_t>(cfg_["radius"], 0);
	if(radius > max_loop_) {
		ERR_NG << "terrain_filter: radius greater than " << max_loop_
		<< ", restricting\n";
		radius = max_loop_;
	}
	if(cfg_.has_child("filter_radius")) {
		terrain_filter r_filter(cfg_.child("filter_radius"), *this);
		get_tiles_radius(map_, loc_vec, radius, hexes, &r_filter);
	} else {
		get_tiles_radius(map_, loc_vec, radius, hexes);
	}

	size_t loop_count = 0;
	std::set<gamemap::location>::const_iterator i;
	for(i = hexes.begin(); i != hexes.end(); ++i) {
		bool matches = match_internal(*i, false);

		//handle [and], [or], and [not] with in-order precedence
		config::all_children_iterator cond = cfg_.get_config().ordered_begin();
		config::all_children_iterator cond_end = cfg_.get_config().ordered_end();
		while(cond != cond_end)
		{
			const std::string& cond_name = *((*cond).first);
			const vconfig cond_cfg(&(*((*cond).second)));

			//handle [and]
			if(cond_name == "and")
			{
				matches = matches && terrain_filter(cond_cfg, *this)(*i);
			}
			//handle [or]
			else if(cond_name == "or")
			{
				matches = matches || terrain_filter(cond_cfg, *this)(*i);
			}
			//handle [not]
			else if(cond_name == "not")
			{
				matches = matches && !terrain_filter(cond_cfg, *this)(*i);
			}
			++cond;
		}
		if(matches) {
			return true;
		}
		if(++loop_count > max_loop_) {
			std::set<gamemap::location>::const_iterator temp = i;
			if(++temp != hexes.end()) {
				ERR_NG << "terrain_filter: loop count greater than " << max_loop_
				<< ", aborting\n";
				break;
			}
		}
	}
	return false;
}

void terrain_filter::get_locations(std::set<gamemap::location>& locs)
{
	std::vector<gamemap::location> xy_vector = parse_location_range(cfg_["x"],cfg_["y"], &map_);
	std::set<gamemap::location> xy_set(xy_vector.begin(), xy_vector.end());
	if(xy_set.empty()) {
		//consider all locations on the map
		for(int x=0; x < map_.w(); x++) {
			for(int y=0; y < map_.h(); y++) {
				xy_set.insert(gamemap::location(x,y));
			}
		}
	}
	if(cfg_.has_attribute("find_in")) {
		//remove any locations not found in the specified variable
		variable_info vi(cfg_["find_in"], false, variable_info::TYPE_CONTAINER);
		if(!vi.is_valid) {
			xy_set.clear();
		} else if(vi.explicit_index) {
			gamemap::location test_loc(vi.as_container(),NULL);
			if(xy_set.count(test_loc)) {
				xy_set.clear();
				xy_set.insert(test_loc);
			} else {
				xy_set.clear();
			}
		} else {
			std::set<gamemap::location> findin_locs;
			variable_info::array_range a_range;
			for(a_range = vi.as_array(); a_range.first != a_range.second; ++a_range.first) {
				gamemap::location test_loc(**a_range.first,NULL);
				if(xy_set.count(test_loc)) {
					findin_locs.insert(test_loc);
				}
			}
			xy_set.swap(findin_locs);
		}
	}

	//handle location filter
	if(cfg_.has_child("filter_adjacent_location")) {
		if(cache_.adjacent_matches == NULL) {
			cache_.adjacent_matches = new std::vector<std::set<gamemap::location> >();
		}
		const vconfig::child_list& adj_cfgs = cfg_.get_children("filter_adjacent_location");
		for (unsigned i = 0; i < adj_cfgs.size(); ++i) {
			std::set<gamemap::location> adj_set;
			/* GCC-3.3 doesn't like operator[] so use at which has the same result */
			terrain_filter(adj_cfgs.at(i), *this).get_locations(adj_set);
			cache_.adjacent_matches->push_back(adj_set);
			if(i >= max_loop_ && i+1 < adj_cfgs.size()) {
				ERR_NG << "terrain_filter: loop count greater than " << max_loop_
				<< ", aborting\n";
				break;
			}
		}
	}
	std::set<gamemap::location>::iterator loc_itor = xy_set.begin();
	while(loc_itor != xy_set.end()) {
		if(match_internal(*loc_itor, true)) {
			++loc_itor;
		} else {
			xy_set.erase(loc_itor++);
		}
	}

	//handle [and], [or], and [not] with in-order precedence
	config::all_children_iterator cond = cfg_.get_config().ordered_begin();
	config::all_children_iterator cond_end = cfg_.get_config().ordered_end();
	int ors_left = std::count_if(cond, cond_end, cfg_isor());
	while(cond != cond_end)
	{
		//if there are no locations or [or] conditions left, go ahead and return empty
		if(xy_set.empty() && ors_left <= 0) {
			return;
		}

		const std::string& cond_name = *((*cond).first);
		const vconfig cond_cfg(&(*((*cond).second)));

		//handle [and]
		if(cond_name == "and") {
			std::set<gamemap::location> intersect_hexes;
			terrain_filter(cond_cfg, *this).get_locations(intersect_hexes);
			std::set<gamemap::location>::iterator intersect_itor = xy_set.begin();
			while(intersect_itor != xy_set.end()) {
				if(intersect_hexes.find(*intersect_itor) == intersect_hexes.end()) {
					xy_set.erase(*intersect_itor++);
				} else {
					++intersect_itor;
				}
			}
		}
		//handle [or]
		else if(cond_name == "or") {
			std::set<gamemap::location> union_hexes;
			terrain_filter(cond_cfg, *this).get_locations(union_hexes);
			//xy_set.insert(union_hexes.begin(), union_hexes.end()); //doesn't compile on MSVC
			std::set<gamemap::location>::iterator insert_itor = union_hexes.begin();
			while(insert_itor != union_hexes.end()) {
				xy_set.insert(*insert_itor++);
			}
			--ors_left;
		}
		//handle [not]
		else if(cond_name == "not") {
			std::set<gamemap::location> removal_hexes;
			terrain_filter(cond_cfg, *this).get_locations(removal_hexes);
			std::set<gamemap::location>::iterator erase_itor = removal_hexes.begin();
			while(erase_itor != removal_hexes.end()) {
				xy_set.erase(*erase_itor++);
			}
		}
		++cond;
	}
	if(xy_set.empty()) {
		return;
	}

	//handle radius
	size_t radius = lexical_cast_default<size_t>(cfg_["radius"], 0);
	if(radius > max_loop_) {
		ERR_NG << "terrain_filter: radius greater than " << max_loop_
		<< ", restricting\n";
		radius = max_loop_;
	}
	if(radius > 0) {
		xy_vector.clear();
		std::copy(xy_set.begin(),xy_set.end(),std::inserter(xy_vector,xy_vector.end()));
		if(cfg_.has_child("filter_radius")) {
			terrain_filter r_filter(cfg_.child("filter_radius"), *this);
			get_tiles_radius(map_, xy_vector, radius, locs, &r_filter);
		} else {
			get_tiles_radius(map_, xy_vector, radius, locs);
		}
	} else {
		std::copy(xy_set.begin(),xy_set.end(),std::inserter(locs,locs.end()));
	}
}


