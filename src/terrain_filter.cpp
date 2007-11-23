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
#include "map.hpp"
#include "pathfind.hpp"
#include "terrain_filter.hpp"
#include "util.hpp"
#include "wassert.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>

#define ERR_CF LOG_STREAM(err, config)
#define LOG_G LOG_STREAM(info, general)
#define ERR_NG LOG_STREAM(err, engine)

namespace {
	struct terrain_filter_cache {
		terrain_filter_cache() : parsed_terrain(NULL), adjacent_matches(NULL) {}
		~terrain_filter_cache() { 
			delete parsed_terrain;
			delete adjacent_matches;
		}
		t_translation::t_match *parsed_terrain;
		std::vector< std::set<gamemap::location> > *adjacent_matches;
		std::vector< std::map<gamemap::location,bool> > adjacent_match_cache;
	};

	struct cfg_isor {
		bool operator() (std::pair<const std::string*,const config*> val) {
			return *(val.first) == "or";
		}
	};
} //end anonymous namespace

static bool terrain_matches_internal(const gamemap& map, const gamemap::location& loc, const vconfig& cfg,
		const gamestatus& game_status, const unit_map& units, const bool flat_tod,
		const bool ignore_xy, terrain_filter_cache& cache)
{

	if(cfg.has_attribute("terrain")) {
		if(cache.parsed_terrain == NULL) {
			cache.parsed_terrain = new t_translation::t_match(cfg["terrain"]);
		}
		if(!cache.parsed_terrain->is_empty) {
			const t_translation::t_letter letter = map.get_terrain_info(loc).number();
			if(!t_translation::terrain_matches(letter, *cache.parsed_terrain)) {
				return false;
			}
		}
	}

	//Allow filtering on location ranges
	if(!ignore_xy) {
		if(!loc.matches_range(cfg["x"], cfg["y"])) {
			return false;
		}
		//allow filtering by searching a stored variable of locations
		if(cfg.has_attribute("find_in")) {
			variable_info vi(cfg["find_in"], false, variable_info::TYPE_CONTAINER);
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
	if(cfg.has_child("filter")) {
		const vconfig& unit_filter = cfg.child("filter");
		const unit_map::const_iterator u = units.find(loc);
		if (u == units.end() || !u->second.matches_filter(unit_filter, loc, flat_tod))
			return false;
	}

	//Allow filtering on adjacent locations
	if(cfg.has_child("filter_adjacent_location")) {
		gamemap::location adjacent[6];
		get_adjacent_tiles(loc, adjacent);
		const vconfig::child_list& adj_filt = cfg.get_children("filter_adjacent_location");
		vconfig::child_list::const_iterator i, i_end, i_begin = adj_filt.begin();
		for (i = i_begin, i_end = adj_filt.end(); i != i_end; ++i) {
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
				if(map.on_board(adj)) {
					if(cache.adjacent_matches == NULL) {
						while(index >= std::distance(cache.adjacent_match_cache.begin(), cache.adjacent_match_cache.end())) {
							cache.adjacent_match_cache.push_back(std::map<gamemap::location,bool>());
						}
						std::map<gamemap::location,bool> &amc = cache.adjacent_match_cache[index];
						std::map<gamemap::location,bool>::iterator lookup = amc.find(adj);
						if(lookup == amc.end()) {
							if(terrain_matches_filter(map,adj,*i,game_status,units,flat_tod)) {
								amc[adj] = true;
								++match_count;
							} else {
								amc[adj] = false;
							}
						} else if(lookup->second) {
							++match_count;
						}
					} else if(index < std::distance(cache.adjacent_matches->begin(), cache.adjacent_matches->end())) {
						std::set<gamemap::location> &amc = (*cache.adjacent_matches)[index];
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
			tod = timeofday_at(game_status,units,loc, map);
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

namespace {
	//terrain predicate, returns true if a terrain matches the predicate
	class terrain_pred : public xy_pred, protected terrain_filter_cache {
	public:
		terrain_pred(const gamemap& map, const vconfig& cfg, const gamestatus& game_status,
			const unit_map& units, const bool flat_tod) : map_(map), cfg_(cfg),
			status_(game_status), units_(units), flat_(flat_tod) {}

		virtual bool operator()(const gamemap::location& loc) {
			return terrain_matches_internal(map_, loc, cfg_, status_, units_, flat_, false, *this);
		}
	private:
		const gamemap& map_;
		const vconfig& cfg_;
		const gamestatus& status_;
		const unit_map& units_;
		const bool flat_;
	};

} //end anonymous namespace

bool terrain_matches_filter(const gamemap& map, const gamemap::location& loc, const vconfig& cfg,
		const gamestatus& game_status, const unit_map& units, const bool flat_tod,
		const size_t max_loop)
{
	if(cfg["x"] == "recall" && cfg["y"] == "recall") {
		return !map.on_board(loc);
	}
	std::set<gamemap::location> hexes;
	std::vector<gamemap::location> loc_vec(1, loc);

	//handle radius
	size_t radius = lexical_cast_default<size_t>(cfg["radius"], 0);
	if(radius > max_loop) {
		ERR_NG << "terrain_matches_filter: radius greater than " << max_loop
		<< ", restricting\n";
		radius = max_loop;
	}
	if(cfg.has_child("filter_radius")) {
		terrain_pred tp(map, cfg.child("filter_radius"), game_status, units, flat_tod);
		get_tiles_radius(map, loc_vec, radius, hexes, &tp);
	} else {
		get_tiles_radius(map, loc_vec, radius, hexes);
	}

	size_t loop_count = 0;
	std::set<gamemap::location>::const_iterator i;
	terrain_filter_cache tfc;
	for(i = hexes.begin(); i != hexes.end(); ++i) {
		bool matches = terrain_matches_internal(map, *i, cfg, game_status, units, flat_tod, false, tfc);

		//handle [and], [or], and [not] with in-order precedence
		config::all_children_iterator cond = cfg.get_config().ordered_begin();
		config::all_children_iterator cond_end = cfg.get_config().ordered_end();
		while(cond != cond_end)
		{
			const std::string& cond_name = *((*cond).first);
			const vconfig cond_filter(&(*((*cond).second)));

			//handle [and]
			if(cond_name == "and")
			{
				matches = matches &&
					terrain_matches_filter(map, *i, cond_filter, game_status, units, flat_tod, max_loop);
			}
			//handle [or]
			else if(cond_name == "or")
			{
				matches = matches ||
					terrain_matches_filter(map, *i, cond_filter, game_status, units, flat_tod, max_loop);
			}
			//handle [not]
			else if(cond_name == "not")
			{
				matches = matches &&
					!terrain_matches_filter(map, *i, cond_filter, game_status, units, flat_tod, max_loop);
			}

			++cond;
		}
		if(matches) {
			return true;
		}
		if(++loop_count > max_loop) {
			std::set<gamemap::location>::const_iterator temp = i;
			if(++temp != hexes.end()) {
				ERR_NG << "terrain_matches_filter: loop count greater than " << max_loop
				<< ", aborting\n";
				break;
			}
		}
	}
	return false;
}

void get_locations(const gamemap& map, std::set<gamemap::location>& locs, const vconfig& filter,
		const gamestatus& game_status, const unit_map& units, const bool flat_tod,
		const size_t max_loop)
{
	std::vector<gamemap::location> xy_vector = parse_location_range(filter["x"],filter["y"], &map);
	std::set<gamemap::location> xy_set(xy_vector.begin(), xy_vector.end());
	if(xy_set.empty()) {
		//consider all locations on the map
		for(int x=0; x < map.w(); x++) {
			for(int y=0; y < map.h(); y++) {
				xy_set.insert(gamemap::location(x,y));
			}
		}
	}
	if(filter.has_attribute("find_in")) {
		//remove any locations not found in the specified variable
		variable_info vi(filter["find_in"], false, variable_info::TYPE_CONTAINER);
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
	terrain_filter_cache tfc;
	if(filter.has_child("filter_adjacent_location")) {
		tfc.adjacent_matches = new std::vector<std::set<gamemap::location> >();
		const vconfig::child_list& adj_filt = filter.get_children("filter_adjacent_location");
		for (unsigned i = 0; i < adj_filt.size(); ++i) {
			std::set<gamemap::location> adj_set;
			get_locations(map, adj_set, adj_filt[i], game_status, units, flat_tod, max_loop);
			tfc.adjacent_matches->push_back(adj_set);
			if(i >= max_loop && i+1 < adj_filt.size()) {
				ERR_NG << "get_locations: loop count greater than " << max_loop
				<< ", aborting\n";
				break;
			}
		}
	}
	std::set<gamemap::location>::iterator loc_itor = xy_set.begin();
	while(loc_itor != xy_set.end()) {
		if(terrain_matches_internal(map, *loc_itor, filter, game_status, units, flat_tod, true, tfc)) {
			++loc_itor;
		} else {
			xy_set.erase(loc_itor++);
		}
	}

	//handle [and], [or], and [not] with in-order precedence
	config::all_children_iterator cond = filter.get_config().ordered_begin();
	config::all_children_iterator cond_end = filter.get_config().ordered_end();
	int ors_left = std::count_if(cond, cond_end, cfg_isor());
	while(cond != cond_end)
	{
		//if there are no locations or [or] conditions left, go ahead and return empty
		if(xy_set.empty() && ors_left <= 0) {
			return;
		}

		const std::string& cond_name = *((*cond).first);
		const vconfig cond_filter(&(*((*cond).second)));

		//handle [and]
		if(cond_name == "and") {
			std::set<gamemap::location> intersect_hexes;
			get_locations(map, intersect_hexes, cond_filter, game_status, units, flat_tod, max_loop);
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
			get_locations(map, union_hexes, cond_filter, game_status, units, flat_tod, max_loop);
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
			get_locations(map, removal_hexes, cond_filter, game_status, units, flat_tod, max_loop);
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
	size_t radius = lexical_cast_default<size_t>(filter["radius"], 0);
	if(radius > max_loop) {
		ERR_NG << "get_locations: radius greater than " << max_loop
		<< ", restricting\n";
		radius = max_loop;
	}
	if(radius > 0) {
		xy_vector.clear();
		std::copy(xy_set.begin(),xy_set.end(),std::inserter(xy_vector,xy_vector.end()));
		if(filter.has_child("filter_radius")) {
			terrain_pred tp(map, filter.child("filter_radius"), game_status, units, flat_tod);
			get_tiles_radius(map, xy_vector, radius, locs, &tp);
		} else {
			get_tiles_radius(map, xy_vector, radius, locs);
		}
	} else {
		std::copy(xy_set.begin(),xy_set.end(),std::inserter(locs,locs.end()));
	}
}


