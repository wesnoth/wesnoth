/*
   Copyright (C) 2010 - 2018 by Fabian Mueller <fabianmueller5@gmx.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "pathfind/teleport.hpp"

#include "display_context.hpp"
#include "filter_context.hpp"
#include "game_board.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "serialization/string_utils.hpp"
#include "team.hpp"
#include "terrain/filter.hpp"
#include "units/unit.hpp"
#include "units/filter.hpp"
#include "units/map.hpp"

static lg::log_domain log_engine("engine");
#define ERR_PF LOG_STREAM(err, log_engine)

namespace pathfind {


namespace {
	const std::string reversed_suffix = "-__REVERSED__";
}

// This constructor is *only* meant for loading from saves
teleport_group::teleport_group(const config& cfg) : cfg_(cfg), reversed_(cfg["reversed"].to_bool(false)), id_(cfg["id"])
{
	assert(cfg.has_attribute("id"));
	assert(cfg.has_attribute("reversed"));

	assert(cfg_.child_count("source") == 1);
	assert(cfg_.child_count("target") == 1);
	assert(cfg_.child_count("filter") == 1);
}

teleport_group::teleport_group(const vconfig& cfg, bool reversed) : cfg_(cfg.get_config()), reversed_(reversed), id_()
{
	assert(cfg_.child_count("source") == 1);
	assert(cfg_.child_count("target") == 1);
	assert(cfg_.child_count("filter") == 1);
	if (cfg["id"].empty()) {
		id_ = resources::tunnels->next_unique_id();
	} else {
		id_ = cfg["id"].str();
		if (reversed_) // Differentiate the reverse tunnel from the forward one
			id_ += reversed_suffix;
	}
}

class ignore_units_display_context : public display_context {
public:
	ignore_units_display_context(const display_context & dc)
		: um_()
		, gm_(&dc.map())
		, tm_(&dc.teams())
		, lbls_(&dc.hidden_label_categories())
	{
		static unit_map empty_unit_map;
		um_ = &empty_unit_map;
	}
	const unit_map & units() const override { return *um_; }
	const gamemap & map() const override { return *gm_; }
	const std::vector<team> & teams() const override { return *tm_; }
	const std::vector<std::string> & hidden_label_categories() const override { return *lbls_; }

private:
	const unit_map * um_;
	const gamemap * gm_;
	const std::vector<team> * tm_;
	const std::vector<std::string> * lbls_;
};

class ignore_units_filter_context : public filter_context {
public:
	ignore_units_filter_context(const filter_context & fc)
		: dc_(fc.get_disp_context())
		, tod_(&fc.get_tod_man())
		, gd_(fc.get_game_data())
		, lk_(fc.get_lua_kernel())
	{}

	const display_context & get_disp_context() const { return dc_; }
	const tod_manager & get_tod_man() const { return *tod_; }
	const game_data * get_game_data() const { return gd_; }
	game_lua_kernel * get_lua_kernel() const { return lk_; }

private:
	const ignore_units_display_context dc_;
	const tod_manager * tod_;
	const game_data * gd_;
	game_lua_kernel * lk_;
};

void teleport_group::get_teleport_pair(
		  teleport_pair& loc_pair
		, const unit& u
		, const bool ignore_units) const
{
	const filter_context * fc = resources::filter_con;
	assert(fc);

	if (ignore_units) {
		fc = new ignore_units_filter_context(*resources::filter_con);
	}

	vconfig filter(cfg_.child_or_empty("filter"), true);
	vconfig source(cfg_.child_or_empty("source"), true);
	vconfig target(cfg_.child_or_empty("target"), true);
	const unit_filter ufilt(filter, resources::filter_con); //Note: Don't use the ignore units filter context here, only for the terrain filters. (That's how it worked before the filter contexts were introduced)
	if (ufilt.matches(u)) {
		terrain_filter source_filter(source, fc);
		source_filter.get_locations(reversed_ ? loc_pair.second : loc_pair.first, u);

		terrain_filter target_filter(target, fc);
		target_filter.get_locations(reversed_ ? loc_pair.first : loc_pair.second, u);
	}

	if (ignore_units) {
		delete fc;
	}
}

const std::string& teleport_group::get_teleport_id() const {
	return id_;
}

bool teleport_group::always_visible() const {
	return cfg_["always_visible"].to_bool(false);
}

bool teleport_group::pass_allied_units() const {
	return cfg_["pass_allied_units"].to_bool(true);
}

bool teleport_group::allow_vision() const {
	return cfg_["allow_vision"].to_bool(true);
}

config teleport_group::to_config() const {
	config retval = cfg_;
	retval["saved"] = "yes";
	retval["reversed"] = reversed_ ? "yes" : "no";
	retval["id"] = id_;
	return retval;
}

teleport_map::teleport_map(
		  const std::vector<teleport_group>& groups
		, const unit& unit
		, const team &viewing_team
		, const bool see_all
		, const bool ignore_units
		, const bool check_vision)
	: teleport_map_()
	, sources_()
	, targets_()
{

	for (const teleport_group& group : groups) {

		teleport_pair locations;

		if (check_vision && !group.allow_vision()) {
			continue;
		}

		group.get_teleport_pair(locations, unit, ignore_units);
		if (!see_all && !group.always_visible() && viewing_team.is_enemy(unit.side())) {
			teleport_pair filter_locs;
			for (const map_location &loc : locations.first) {
				if(!viewing_team.fogged(loc))
					filter_locs.first.insert(loc);
			}
			for (const map_location &loc : locations.second) {
				if(!viewing_team.fogged(loc))
					filter_locs.second.insert(loc);
			}
			locations.first.swap(filter_locs.first);
			locations.second.swap(filter_locs.second);
		}

		if (!group.pass_allied_units() && !ignore_units && !check_vision) {
			std::set<map_location>::iterator loc = locations.second.begin();
			while(loc != locations.second.end()) {
				unit_map::iterator u;
				if (see_all) {
					u = resources::gameboard->units().find(*loc);
				} else {
					u = resources::gameboard->find_visible_unit(*loc, viewing_team);
				}
				if (u != resources::gameboard->units().end()) {
					loc = locations.second.erase(loc);
				} else {
					++loc;
				}
			}
		}

		std::string teleport_id = group.get_teleport_id();
		std::set<map_location>::iterator source_it = locations.first.begin();
		for (; source_it != locations.first.end(); ++source_it ) {
			if(teleport_map_.count(*source_it) == 0) {
				std::set<std::string> id_set;
				id_set.insert(teleport_id);
				teleport_map_.emplace(*source_it, id_set);
			} else {
				(teleport_map_.find(*source_it)->second).insert(teleport_id);
			}
		}
		sources_.emplace(teleport_id, locations.first);
		targets_.emplace(teleport_id, locations.second);
	}
}

void teleport_map::get_adjacents(std::set<map_location>& adjacents, map_location loc) const {

	if (teleport_map_.count(loc) == 0) {
		return;
	} else {
		const std::set<std::string>& keyset = (teleport_map_.find(loc)->second);
		for(std::set<std::string>::const_iterator it = keyset.begin(); it != keyset.end(); ++it) {

			const std::set<map_location>& target = targets_.find(*it)->second;
			adjacents.insert(target.begin(), target.end());
		}
	}
}

void teleport_map::get_sources(std::set<map_location>& sources) const {

	std::map<std::string, std::set<map_location> >::const_iterator it;
	for(it = sources_.begin(); it != sources_.end(); ++it) {
		sources.insert(it->second.begin(), it->second.end());
	}
}

void teleport_map::get_targets(std::set<map_location>& targets) const {

	std::map<std::string, std::set<map_location> >::const_iterator it;
	for(it = targets_.begin(); it != targets_.end(); ++it) {
		targets.insert(it->second.begin(), it->second.end());
	}
}


const teleport_map get_teleport_locations(const unit &u,
	const team &viewing_team,
	bool see_all, bool ignore_units, bool check_vision)
{
	std::vector<teleport_group> groups;

	if (u.get_ability_bool("teleport", *resources::gameboard)) {
		for (const unit_ability & teleport : u.get_abilities("teleport")) {
			const int tunnel_count = (teleport.first)->child_count("tunnel");
			for(int i = 0; i < tunnel_count; ++i) {
				config teleport_group_cfg = (teleport.first)->child("tunnel", i);
				groups.emplace_back(vconfig(teleport_group_cfg, true), false);
			}
		}
	}

	const std::vector<teleport_group>& global_groups = resources::tunnels->get();
	groups.insert(groups.end(), global_groups.begin(), global_groups.end());

	return teleport_map(groups, u, viewing_team, see_all, ignore_units, check_vision);
}

manager::manager(const config &cfg) : tunnels_(), id_(cfg["next_teleport_group_id"].to_int(0)) {
	const int tunnel_count = cfg.child_count("tunnel");
	for(int i = 0; i < tunnel_count; ++i) {
		const config& t = cfg.child("tunnel", i);
		if(!t["saved"].to_bool()) {
			lg::wml_error() << "Do not use [tunnel] directly in a [scenario]. Use it in an [event] or [abilities] tag.\n";
			continue;
		}
		const teleport_group tunnel(t);
		this->add(tunnel);
	}
}

void manager::add(const teleport_group &group) {
	tunnels_.push_back(group);
}

void manager::remove(const std::string &id) {
	std::vector<teleport_group>::iterator t = tunnels_.begin();
	for(;t != tunnels_.end();) {
		if (t->get_teleport_id() == id || t->get_teleport_id() == id + reversed_suffix) {
			t = tunnels_.erase(t);
		} else {
			++t;
		}
	}
}

const std::vector<teleport_group>& manager::get() const {
	return tunnels_;
}

config manager::to_config() const {
	config store;

	std::vector<teleport_group>::const_iterator tunnel = tunnels_.begin();
	for(; tunnel != tunnels_.end(); ++tunnel) {
		store.add_child("tunnel", tunnel->to_config());
	}
	store["next_teleport_group_id"] = std::to_string(id_);

	return store;
}

std::string manager::next_unique_id() {
	return std::to_string(++id_);
}


}//namespace pathfind
