/* $Id$ */
/*
   Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "global.hpp"

#include "actions.hpp"
#include "config.hpp"
#include "foreach.hpp"
#include "log.hpp"
#include "map.hpp"
#include "resources.hpp"
#include "terrain_filter.hpp"
#include "tod_manager.hpp"
#include "variable.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)

namespace {
//Constant literal tokens
static const config::t_token z_x("x", false);
static const config::t_token z_y("y", false);
static const config::t_token z_or("or", false);
static const config::t_token z_and("and", false);
static const config::t_token z_not("not", false);
static const config::t_token z_terrain("terrain", false);
static const config::t_token z_filter("filter", false);
static const config::t_token z_filter_adjacent_location("filter_adjacent_location", false);
static const config::t_token z_adjacent("adjacent", false);
static const config::t_token z_count("count", false);
static const config::t_token z_time_of_day("time_of_day", false);
static const config::t_token z_time_of_day_id("time_of_day_id", false);
static const config::t_token z_chaotic("chaotic", false);
static const config::t_token z_lawful("lawful", false);
static const config::t_token z_neutral("neutral", false);
static const config::t_token z_liminal("liminal", false);
static const config::t_token z_owner_side("owner_side", false);
static const config::t_token z_recall("recall", false);
static const config::t_token z_radius("radius", false);
static const config::t_token z_filter_radius("filter_radius", false);
static const config::t_token z_find_in("find_in", false);
//static const config::t_token z_("", false);


}
terrain_filter::~terrain_filter()
{
}

#ifdef _MSC_VER
// This is a workaround for a VC bug; this constructor is never called
// and so we don't care about the warnings this quick fix generates
#pragma warning(push)
#pragma warning(disable:4413)
terrain_filter::terrain_filter():
	cfg_(vconfig::unconstructed_vconfig()),
	units_(unit_map()),
	cache_(),
	max_loop_(),
	flat_()
{
	assert(false);
}
#pragma warning(pop)
#endif


terrain_filter::terrain_filter(const vconfig& cfg, const unit_map& units,
		const bool flat_tod, const size_t max_loop) :
	cfg_(cfg),
	units_(units),
	cache_(),
	max_loop_(max_loop),
	flat_(flat_tod)
{
}

terrain_filter::terrain_filter(const vconfig& cfg, const terrain_filter& original) :
	cfg_(cfg),
	units_(original.units_),
	cache_(),
	max_loop_(original.max_loop_),
	flat_(original.flat_)
{
}

terrain_filter::terrain_filter(const terrain_filter& other) :
	xy_pred(), // We should construct this too, since it has no datamembers
	           // use the default constructor.
	cfg_(other.cfg_),
	units_(other.units_),
	cache_(),
	max_loop_(other.max_loop_),
	flat_(other.flat_)
{
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
		bool operator() (std::pair<const config::t_token, const vconfig> val) {
			return val.first == z_or;
		}
	};
} //end anonymous namespace

bool terrain_filter::match_internal(const map_location& loc, const bool ignore_xy,gamemap const & game_map, t_teams const & teams, tod_manager const & tod_manager) const
{
	if(cfg_.has_attribute(z_terrain)) {
		if(cache_.parsed_terrain == NULL) {
			cache_.parsed_terrain = new t_translation::t_match(cfg_[z_terrain]);
		}
		if(!cache_.parsed_terrain->is_empty) {
			const t_translation::t_terrain letter = game_map.get_terrain_info(loc).number();
			if(!t_translation::terrain_matches(letter, *cache_.parsed_terrain)) {
				return false;
			}
		}
	}

	//Allow filtering on location ranges
	if(!ignore_xy) {
		if(!loc.matches_range(cfg_[z_x], cfg_[z_y])) {
			return false;
		}
		//allow filtering by searching a stored variable of locations
		if(cfg_.has_attribute(z_find_in)) {
			variable_info vi(cfg_[z_find_in].token(), false, variable_info::TYPE_CONTAINER);
			if(!vi.is_valid()) return false;
			if(vi.is_explicit_index()) {
				if(map_location(vi.as_container(),NULL) != loc) {
					return false;
				}
			} else {
				bool found = false;
				foreach (const config &cfg, vi.as_array()) {
					if (map_location(cfg, NULL) == loc) {
						found = true;
						break;
					}
				}
				if (!found) return false;
			}
		}
	}

	//Allow filtering on unit
	const vconfig& unit_filter = cfg_.child(z_filter);

	if(! unit_filter.null() ){
		const unit_map::const_iterator u = units_.find(loc);
		if (u == units_.end() || ! u->matches_filter(unit_filter, loc, flat_) )
			return false;
	}

	//Allow filtering on adjacent locations
	if(cfg_.has_child(z_filter_adjacent_location)) {
		map_location adjacent[6];
		get_adjacent_tiles(loc, adjacent);
		const vconfig::child_list& adj_cfgs = cfg_.get_children(z_filter_adjacent_location);
		vconfig::child_list::const_iterator i, i_end, i_begin = adj_cfgs.begin();
		for (i = i_begin, i_end = adj_cfgs.end(); i != i_end; ++i) {
			int match_count = 0;
			vconfig::child_list::difference_type index = i - i_begin;
			static std::vector<map_location::DIRECTION> default_dirs
				= map_location::parse_directions("n,ne,se,s,sw,nw");
			std::vector<map_location::DIRECTION> dirs = (*i).has_attribute(z_adjacent)
				? map_location::parse_directions((*i)[z_adjacent]) : default_dirs;
			std::vector<map_location::DIRECTION>::const_iterator j, j_end = dirs.end();
			for (j = dirs.begin(); j != j_end; ++j) {
				map_location &adj = adjacent[*j];
				if (game_map.on_board(adj)) {
					if(cache_.adjacent_matches == NULL) {
						while(index >= std::distance(cache_.adjacent_match_cache.begin(), cache_.adjacent_match_cache.end())) {
							const vconfig& adj_cfg = adj_cfgs[cache_.adjacent_match_cache.size()];
							t_adjacent_match_pair amc_pair(
								terrain_filter(adj_cfg, *this),
								t_adjacent_match_cache_hit());
							cache_.adjacent_match_cache.push_back(amc_pair);
						}
						terrain_filter &amc_filter = cache_.adjacent_match_cache[index].first;
						t_adjacent_match_cache_hit &amc = cache_.adjacent_match_cache[index].second;
						t_adjacent_match_cache_hit::iterator lookup = amc.find(adj);
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
						t_maploc_set &amc = (*cache_.adjacent_matches)[index];
						if(amc.find(adj) != amc.end()) {
							++match_count;
						}
					}
				}
			}
			static std::vector<std::pair<int,int> > default_counts = utils::parse_ranges("1-6");
			std::vector<std::pair<int,int> > counts = (*i).has_attribute(z_count)
				? utils::parse_ranges((*i)[z_count]) : default_counts;
			if(!in_ranges(match_count, counts)) {
				return false;
			}
		}
	}

	// const t_string& t_tod_type = cfg_[z_time_of_day];
	// const t_string& t_tod_id = cfg_[z_time_of_day_id];
	const config::t_token& tod_type = cfg_[z_time_of_day];
	const config::t_token& tod_id = cfg_[z_time_of_day_id];
	static config const dummy_cfg;
	time_of_day tod(dummy_cfg);
	if(!tod_type.empty() || !tod_id.empty()) {
		if(flat_) {
			tod = tod_manager.get_time_of_day_with_areas(loc);
		} else {
			tod = tod_manager.get_time_of_day(0, loc, true);
		}
	}
	if(!tod_type.empty()) {
		const std::vector<config::t_token>& vals = utils::split_token(tod_type);
		if(tod.lawful_bonus<0) {
			if(std::find(vals.begin(),vals.end(), z_chaotic) == vals.end()) {
				return false;
			}
		} else if(tod.lawful_bonus>0) {
			if(std::find(vals.begin(),vals.end(),z_lawful) == vals.end()) {
				return false;
			}
		} else if(std::find(vals.begin(),vals.end(),z_neutral) == vals.end()
			  && std::find(vals.begin(),vals.end(),z_liminal) == vals.end() ) {
			return false;
		}
	}

	if(!tod_id.empty()) {
		if(tod_id != config::t_token(tod.id)) {
			const std::vector<config::t_token>& vals = utils::split_token(tod_id);
			if(std::find(vals.begin(),vals.end(),tod.id) == vals.end()) {
				return false;
			}
		}
	}

	//allow filtering on owner (for villages)
	const config::attribute_value& owner_side = cfg_[z_owner_side];
	if(!owner_side.empty()) {
		const int side_index = owner_side.to_int(0) - 1;
		if(village_owner(loc, teams) != side_index) {
			return false;
		}
	}

	return true;
}

bool terrain_filter::match(const map_location& loc, gamemap const & game_map, t_teams const & teams, tod_manager const & tod_manager) const
{
	if(cfg_[z_x] == z_recall && cfg_[z_y] == z_recall) {
		return !game_map.on_board(loc);
	}
	t_maploc_set hexes;
	std::vector<map_location> loc_vec(1, loc);

	//handle radius
	size_t radius = lexical_cast_default<size_t>(cfg_[z_radius], 0);
	if(radius > max_loop_) {
		ERR_NG << "terrain_filter: radius greater than " << max_loop_
		<< ", restricting\n";
		radius = max_loop_;
	}
	const vconfig& filter_radius = cfg_.child(z_filter_radius);

	if(! filter_radius.null() ) {
		terrain_filter r_filter(filter_radius , *this);
		get_tiles_radius(game_map, loc_vec, radius, hexes, &r_filter);
	} else {
		get_tiles_radius(game_map, loc_vec, radius, hexes);
	}

	size_t loop_count = 0;
	t_maploc_set::const_iterator i;
	for(i = hexes.begin(); i != hexes.end(); ++i) {
		bool matches = match_internal(*i, false, game_map, teams, tod_manager);

		//handle [and], [or], and [not] with in-order precedence
		vconfig::all_children_iterator cond = cfg_.ordered_begin();
		vconfig::all_children_iterator cond_end = cfg_.ordered_end();
		while(cond != cond_end)
		{
			const config::t_token& cond_name = cond.get_key();
			const vconfig& cond_cfg = cond.get_child();

			//handle [and]
			if(cond_name == z_and)
			{
				matches = matches && terrain_filter(cond_cfg, *this)(*i);
			}
			//handle [or]
			else if(cond_name == z_or)
			{
				matches = matches || terrain_filter(cond_cfg, *this)(*i);
			}
			//handle [not]
			else if(cond_name == z_not)
			{
				matches = matches && !terrain_filter(cond_cfg, *this)(*i);
			}
			++cond;
		}
		if(matches) {
			return true;
		}
		if(++loop_count > max_loop_) {
			t_maploc_set::const_iterator temp = i;
			if(++temp != hexes.end()) {
				ERR_NG << "terrain_filter: loop count greater than " << max_loop_
				<< ", aborting\n";
				break;
			}
		}
	}
	return false;
}

void terrain_filter::get_locations(t_maploc_set& locs, bool with_border, gamemap const & game_map, t_teams const & teams, tod_manager const & tod_manager) const
{
	std::vector<map_location> xy_vector =
		parse_location_range(cfg_[z_x], cfg_[z_y], with_border);
	t_maploc_set xy_set(xy_vector.begin(), xy_vector.end());
	if (!cfg_.has_attribute(z_x) && !cfg_.has_attribute(z_y)) {
		//consider all locations on the map
		int bs = game_map.border_size();
		int w = with_border ? game_map.w() + bs : game_map.w();
		int h = with_border ? game_map.h() + bs : game_map.h();
		for (int x = with_border ? 0 - bs : 0; x < w; ++x) {
			for (int y = with_border ? 0 - bs : 0; y < h; ++y) {
				xy_set.insert(map_location(x,y));
			}
		}
	}
	if(cfg_.has_attribute(z_find_in)) {
		//remove any locations not found in the specified variable
		variable_info vi(cfg_[z_find_in].token(), false, variable_info::TYPE_CONTAINER);
		if(!vi.is_valid()) {
			xy_set.clear();
		} else if(vi.is_explicit_index()) {
			map_location test_loc(vi.as_container(),NULL);
			if(xy_set.count(test_loc)) {
				xy_set.clear();
				xy_set.insert(test_loc);
			} else {
				xy_set.clear();
			}
		} else {
			t_maploc_set findin_locs;
			foreach (const config &cfg, vi.as_array()) {
				map_location test_loc(cfg, NULL);
				if (xy_set.count(test_loc)) {
					findin_locs.insert(test_loc);
				}
			}
			xy_set.swap(findin_locs);
		}
	}

	//handle location filter
	if(cfg_.has_child(z_filter_adjacent_location)) {
		if(cache_.adjacent_matches == NULL) {
			cache_.adjacent_matches = new std::vector<t_maploc_set >();
		}
		const vconfig::child_list& adj_cfgs = cfg_.get_children(z_filter_adjacent_location);
		for (unsigned i = 0; i < adj_cfgs.size(); ++i) {
			t_maploc_set adj_set;
			/* GCC-3.3 doesn't like operator[] so use at which has the same result */
			terrain_filter(adj_cfgs.at(i), *this).get_locations(adj_set, with_border);
			cache_.adjacent_matches->push_back(adj_set);
			if(i >= max_loop_ && i+1 < adj_cfgs.size()) {
				ERR_NG << "terrain_filter: loop count greater than " << max_loop_
				<< ", aborting\n";
				break;
			}
		}
	}
	t_maploc_set::iterator loc_itor = xy_set.begin();
	while(loc_itor != xy_set.end()) {
		bool test(match_internal(*loc_itor, true, game_map, teams, tod_manager));

		if(test) {
			++loc_itor;
		} else {
			loc_itor = xy_set.erase(loc_itor);
		}
	}

	//handle [and], [or], and [not] with in-order precedence
	vconfig::all_children_iterator cond = cfg_.ordered_begin();
	vconfig::all_children_iterator cond_end = cfg_.ordered_end();
	int ors_left = std::count_if(cond, cond_end, cfg_isor());
	while(cond != cond_end)
	{
		//if there are no locations or [or] conditions left, go ahead and return empty
		if(xy_set.empty() && ors_left <= 0) {
			return;
		}

		const config::t_token& cond_name = cond.get_key();
		const vconfig& cond_cfg = cond.get_child();

		//handle [and]
		if(cond_name == z_and) {
			t_maploc_set intersect_hexes;
			terrain_filter(cond_cfg, *this).get_locations(intersect_hexes, with_border);
			t_maploc_set::iterator intersect_itor = xy_set.begin();
			while(intersect_itor != xy_set.end()) {
				if(intersect_hexes.find(*intersect_itor) == intersect_hexes.end()) {
					intersect_itor = xy_set.erase(intersect_itor);
				} else {
					++intersect_itor;
				}
			}
		}
		//handle [or]
		else if(cond_name == z_or) {
			t_maploc_set union_hexes;
			terrain_filter(cond_cfg, *this).get_locations(union_hexes, with_border);
			//doesn't compile on MSVC
			//xy_set.insert(union_hexes.begin(), union_hexes.end());
			t_maploc_set::iterator insert_itor = union_hexes.begin();
			while(insert_itor != union_hexes.end()) {
				xy_set.insert(*insert_itor++);
			}
			--ors_left;
		}
		//handle [not]
		else if(cond_name == z_not) {
			t_maploc_set removal_hexes;
			terrain_filter(cond_cfg, *this).get_locations(removal_hexes, with_border);
			t_maploc_set::iterator erase_itor = removal_hexes.begin();
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
	size_t radius = lexical_cast_default<size_t>(cfg_[z_radius], 0);
	if(radius > max_loop_) {
		ERR_NG << "terrain_filter: radius greater than " << max_loop_
		<< ", restricting\n";
		radius = max_loop_;
	}
	if(radius > 0) {
		xy_vector.clear();
		std::copy(xy_set.begin(),xy_set.end(),std::inserter(xy_vector,xy_vector.end()));
		const vconfig& filter_radius = cfg_.child(z_filter_radius);
		if(! filter_radius.null()) {
			terrain_filter r_filter(filter_radius, *this);
			get_tiles_radius(game_map, xy_vector, radius, locs, &r_filter);
		} else {
			get_tiles_radius(game_map, xy_vector, radius, locs);
		}
	} else {
		std::copy(xy_set.begin(),xy_set.end(),std::inserter(locs,locs.end()));
	}
}

config terrain_filter::to_config() const
{
	return cfg_.get_config();
}

terrain_filter::terrain_filter_cache::~terrain_filter_cache() {
	delete parsed_terrain;
	delete adjacent_matches;
}
