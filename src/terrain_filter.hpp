/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef TERRAIN_FILTER_H_INCLUDED
#define TERRAIN_FILTER_H_INCLUDED

#include "map.hpp"
#include "pathfind.hpp"

class config;
class gamestatus;
class unit;
class vconfig;
class unit_map;

//terrain_filter: a class that implements the Standard Location Filter
class terrain_filter : public xy_pred {
public:
	terrain_filter(const vconfig& cfg, const gamemap& map, const gamestatus& game_status, 
		const unit_map& units, const bool flat_tod=false, const size_t max_loop=MAX_MAP_AREA);
	terrain_filter(const vconfig& cfg, const terrain_filter& original);
	~terrain_filter() {};

	terrain_filter(const terrain_filter &other);
	terrain_filter& operator=(const terrain_filter &other);

	//match: returns true if and only if the given location matches this filter
	bool match(const gamemap::location& loc);
	virtual bool operator()(const gamemap::location& loc) { return this->match(loc); }

	//get_locations: gets all locations on the map that match this filter
	// @param locs - out parameter containing the results
	void get_locations(std::set<gamemap::location>& locs);

	//restrict: limits the allowed radius size and also limits nesting
	// The purpose to limit the time spent for WML handling
	// Note: this feature is not fully implemented, e.g. SLF inside SUF inside SLF
	void restrict(const size_t max_loop) { max_loop_ = max_loop; }

	//flatten: use base time of day -- ignore illumination ability
	void flatten(const bool flat_tod=true) { flat_ = flat_tod; }

private:
	bool match_internal(const gamemap::location& loc, const bool ignore_xy);

	const vconfig& cfg_; //config contains WML for a Standard Location Filter
	const gamemap& map_;
	const gamestatus& status_;
	const unit_map& units_;

	struct terrain_filter_cache {
		terrain_filter_cache() : parsed_terrain(NULL), adjacent_matches(NULL) {}
		~terrain_filter_cache() { 
			delete parsed_terrain;
			delete adjacent_matches;
		}	

		//parsed_terrain: optimizes handling of terrain="..."
		t_translation::t_match *parsed_terrain; 

		//adjacent_matches: optimize handling of [filter_adjacent_location] for get_locations()
		std::vector< std::set<gamemap::location> > *adjacent_matches; 

		//adjacent_match_cache: optimize handling of [filter_adjacent_location] for match()
		std::vector< std::pair<terrain_filter, std::map<gamemap::location,bool> > > adjacent_match_cache;
	};

	terrain_filter_cache cache_;
	size_t max_loop_;
	bool flat_;
};

#endif

