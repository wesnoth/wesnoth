/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "game_config.hpp"
#include "pathutils.hpp"
#include "terrain/translation.hpp"
#include "variable.hpp"

class config;
class filter_context;
class unit;
class unit_filter;
class unit_map;
class team;

//terrain_filter: a class that implements the Standard Location Filter
class terrain_filter : public xy_pred {
public:

	terrain_filter(const vconfig& cfg,
		const filter_context * fc, const bool flat_tod=false, const size_t max_loop=game_config::max_loop);
	terrain_filter(const vconfig& cfg, const terrain_filter& original);
	/** Default implementation, but defined out-of-line for efficiency reasons. */
	~terrain_filter();

	terrain_filter(const terrain_filter &other);
	terrain_filter& operator=(const terrain_filter &other);

	/// @param loc The location to test
	/// @returns true if and only if the given location matches this filter
	bool match(const map_location& loc) const {
		return match_impl(loc, nullptr);
	}

	/// @param loc The location to test
	/// @param ref_unit A reference unit for the $teleport_unit auto-stored variable
	/// @returns true if and only if the given location matches this filter
	bool match(const map_location& loc, const unit& ref_unit) const {
		return match_impl(loc, &ref_unit);
	}

	virtual bool operator()(const map_location& loc) const { return this->match(loc); }

	/// gets all locations on the map that match this filter
	/// @param[out] locs set to store the results in
	/// @param[in] with_border whether to include the borders
	void get_locations(std::set<map_location>& locs, bool with_border=false) const {
		return get_locs_impl(locs, nullptr, with_border);
	}

	/// gets all locations on the map that match this filter
	/// @param[out] locs set to store the results in
	/// @param[in] with_border whether to include the borders
	/// @param[in] ref_unit A reference unit for the $teleport_unit auto-stored variable
	void get_locations(std::set<map_location>& locs, const unit& ref_unit, bool with_border=false) const {
		return get_locs_impl(locs, &ref_unit, with_border);
	}

	//restrict: limits the allowed radius size and also limits nesting
	// The purpose to limit the time spent for WML handling
	// Note: this feature is not fully implemented, e.g. SLF inside SUF inside SLF
	void restrict_size(const size_t max_loop) { max_loop_ = max_loop; }

	//flatten: use base time of day -- ignore illumination ability
	void flatten(const bool flat_tod=true) { flat_ = flat_tod; }

	config to_config() const;
	friend class terrain_filterimpl;
private:
	bool match_impl(const map_location& loc, const unit* ref_unit) const;
	void get_locs_impl(std::set<map_location>& locs, const unit* ref_unit, bool with_border) const;
	bool match_internal(const map_location& loc, const unit* ref_unit, const bool ignore_xy) const;

	const vconfig cfg_; //config contains WML for a Standard Location Filter
	const filter_context * fc_;

	struct terrain_filter_cache {
		terrain_filter_cache();

		~terrain_filter_cache();

		//parsed_terrain: optimizes handling of terrain="..."
		t_translation::ter_match *parsed_terrain;

		//adjacent_matches: optimize handling of [filter_adjacent_location] for get_locations()
		std::vector< std::set<map_location> > *adjacent_matches;

		//adjacent_match_cache: optimize handling of [filter_adjacent_location] for match()
		std::vector< std::pair<terrain_filter, std::map<map_location,bool> > > adjacent_match_cache;

		std::unique_ptr<unit_filter> ufilter_;
	};

	mutable terrain_filter_cache cache_;
	size_t max_loop_;
	bool flat_;
};
