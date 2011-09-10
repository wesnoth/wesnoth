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

#ifndef TERRAIN_FILTER_H_INCLUDED
#define TERRAIN_FILTER_H_INCLUDED

#include "map_location.hpp"
#include "pathutils.hpp"
#include "terrain_translation.hpp"
#include "variable.hpp"
#include "resources.hpp"
#include <boost/unordered_set.hpp>

class config;
class unit;
class unit_map;
class team;

//terrain_filter: a class that implements the Standard Location Filter
class terrain_filter : public xy_pred {
public:
	typedef map_location::t_maploc_set t_maploc_set;

#ifdef _MSC_VER
	// This constructor is required for MSVC 9 SP1 due to a bug there
	// see http://social.msdn.microsoft.com/forums/en-US/vcgeneral/thread/34473b8c-0184-4750-a290-08558e4eda4e
	// other compilers don't need it.
	 terrain_filter();
#endif

	terrain_filter(const vconfig& cfg,
		const unit_map& units, const bool flat_tod=false, const size_t max_loop=MAX_MAP_AREA);
	terrain_filter(const vconfig& cfg, const terrain_filter& original);
	/** Default implementation, but defined out-of-line for efficiency reasons. */
	~terrain_filter();

	terrain_filter(const terrain_filter &other);
	terrain_filter& operator=(const terrain_filter &other);

	//match: returns true if and only if the given location matches this filter
	bool match(const map_location& loc
			   , gamemap const & game_map = *resources::game_map
			   , t_teams const & teams = *resources::teams
			   , tod_manager const & tod_manager =  *resources::tod_manager) const;
	virtual bool operator()(const map_location& loc) { return this->match(loc); }

	//get_locations: gets all locations on the map that match this filter
	// @param locs - out parameter containing the results
	void get_locations(t_maploc_set& locs, bool with_border=false
					   , gamemap const & game_map = *resources::game_map
					   , t_teams const & teams = *resources::teams
					   , tod_manager const & tod_manager = *resources::tod_manager ) const;

	//restrict: limits the allowed radius size and also limits nesting
	// The purpose to limit the time spent for WML handling
	// Note: this feature is not fully implemented, e.g. SLF inside SUF inside SLF
	void restrict_size(const size_t max_loop) { max_loop_ = max_loop; }

	//flatten: use base time of day -- ignore illumination ability
	void flatten(const bool flat_tod=true) { flat_ = flat_tod; }

	config to_config() const;
private:
	bool match_internal(const map_location& loc, const bool ignore_xy
						, gamemap const & game_map = *resources::game_map
						, t_teams const & teams =  *resources::teams
						, tod_manager const & tod_manager=  *resources::tod_manager ) const ;

	const vconfig cfg_; //config contains WML for a Standard Location Filter
	const unit_map& units_;
	typedef std::vector< t_maploc_set > t_adjacent_matches;
	typedef boost::unordered_map<map_location,bool> t_adjacent_match_cache_hit;
	typedef std::pair<terrain_filter,  t_adjacent_match_cache_hit> t_adjacent_match_pair;
	typedef std::vector< t_adjacent_match_pair > t_adjacent_match_cache;

	struct terrain_filter_cache {
		terrain_filter_cache() :
			parsed_terrain(NULL),
			adjacent_matches(NULL),
			adjacent_match_cache()
		{
		}

		~terrain_filter_cache();

		//parsed_terrain: optimizes handling of terrain="..."
		t_translation::t_match *parsed_terrain;

		//adjacent_matches: optimize handling of [filter_adjacent_location] for get_locations()
		t_adjacent_matches *adjacent_matches;

		//adjacent_match_cache: optimize handling of [filter_adjacent_location] for match()
		t_adjacent_match_cache adjacent_match_cache;
	};

	mutable terrain_filter_cache cache_;
	size_t max_loop_;
	bool flat_;
};

#endif

