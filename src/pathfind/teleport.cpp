/* $Id: teleport.cpp $ */
/*
   Copyright (C) 2010 by Fabian Mueller <fabianmueller5@gmx.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "pathfind/teleport.hpp"

#include "serialization/string_utils.hpp"
#include "unit.hpp"
#include "log.hpp"
#include "resources.hpp"

#include "foreach.hpp"

static lg::log_domain log_engine("engine");
#define ERR_PF LOG_STREAM(err, log_engine)

namespace {
	const std::string reversed_suffix = "-__REVERSED__";
}

// This constructor is *only* meant for loading from saves
pathfind::teleport_group::teleport_group(const config cfg) : cfg_(cfg, true), reversed_(utils::string_bool(cfg["reversed"], false)), id_(cfg["id"])
{
	assert(cfg.has_attribute("id"));
	assert(cfg.has_attribute("reversed"));
	assert(!cfg_.get_children("source").empty());
	assert(!cfg_.get_children("target").empty());
	assert(!cfg_.get_children("filter").empty());
}

pathfind::teleport_group::teleport_group(vconfig cfg, bool reversed) : cfg_(cfg), reversed_(reversed), id_()
{
	assert(!cfg_.get_children("source").empty());
	assert(!cfg_.get_children("target").empty());
	assert(!cfg_.get_children("filter").empty());
	if (cfg["id"].empty()) {
		id_ = resources::tunnels->next_unique_id();
	} else {
		id_ = cfg["id"];
		if (reversed_) // Differentiate the reverse tunnel from the forward one
			id_ += reversed_suffix;
	}
}

void pathfind::teleport_group::get_teleport_pair(teleport_pair& loc_pair, const unit& u, bool ignore_units)
{
	const map_location &loc = u.get_location();
	static unit_map empty_unit_map;
	unit_map *units;
	if (ignore_units) {
		units = &empty_unit_map;
	} else {
		units = resources::units;
	}
	if (u.matches_filter(cfg_.child("filter"), loc)) {

		scoped_xy_unit teleport_unit("teleport_unit", loc.x, loc.y, *resources::units);

		terrain_filter source_filter(cfg_.child("source"), *units);
		source_filter.get_locations(reversed_ ? loc_pair.second : loc_pair.first);

		terrain_filter target_filter(cfg_.child("target"), *units);
		target_filter.get_locations(reversed_ ? loc_pair.first : loc_pair.second);
	}
}

const std::string& pathfind::teleport_group::get_teleport_id() const {
	return id_;
}

bool pathfind::teleport_group::always_visible() const {
	return utils::string_bool(cfg_["always_visible"], false);
}

config pathfind::teleport_group::to_config() const {
	config retval = cfg_.get_config();
	retval["reversed"] = reversed_ ? "yes" : "no";
	retval["id"] = id_;
	return retval;
}

pathfind::teleport_map::teleport_map(std::vector<teleport_group> groups, const unit& u, const unit_map &/*units*/, const team &viewing_team, bool see_all, bool ignore_units)
	: teleport_map_(), sources_(), targets_() {

	for (std::vector<teleport_group>::iterator it = groups.begin(); it != groups.end(); ++it) {

		teleport_pair locations;
		it->get_teleport_pair(locations, u, ignore_units);
		if (!see_all && !it->always_visible() && viewing_team.is_enemy(u.side())) {
			teleport_pair filter_locs;
			foreach(const map_location &loc, locations.first)
				if(!viewing_team.fogged(loc))
					filter_locs.first.insert(loc);
			foreach(const map_location &loc, locations.second)
				if(!viewing_team.fogged(loc))
					filter_locs.second.insert(loc);
			locations.first.swap(filter_locs.first);
			locations.second.swap(filter_locs.second);
		}
		std::string teleport_id = it->get_teleport_id();

		std::set<map_location>::iterator source_it = locations.first.begin();
		for (; source_it != locations.first.end(); ++source_it ) {
			if(teleport_map_.count(*source_it) == 0) {
				std::set<std::string> id_set;
				id_set.insert(teleport_id);
				teleport_map_.insert(std::make_pair(*source_it, id_set));
			} else {
				(teleport_map_.find(*source_it)->second).insert(teleport_id);
			}
		}
		sources_.insert(std::make_pair(teleport_id, locations.first));
		targets_.insert(std::make_pair(teleport_id, locations.second));
	}
}

void pathfind::teleport_map::get_adjacents(std::set<map_location>& adjacents, map_location loc) const {

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

void pathfind::teleport_map::get_sources(std::set<map_location>& sources) const {

	std::map<std::string, std::set<map_location> >::const_iterator it;
	for(it = sources_.begin(); it != sources_.end(); ++it) {
		sources.insert(it->second.begin(), it->second.end());
	}
}

void pathfind::teleport_map::get_targets(std::set<map_location>& targets) const {

	std::map<std::string, std::set<map_location> >::const_iterator it;
	for(it = targets_.begin(); it != targets_.end(); ++it) {
		targets.insert(it->second.begin(), it->second.end());
	}
}


const pathfind::teleport_map pathfind::get_teleport_locations(const unit &u,
	const unit_map &units, const team &viewing_team,
	bool see_all, bool ignore_units)
{
	std::vector<teleport_group> groups;

	if (u.get_ability_bool("teleport")) {

		unit_ability_list teleport_list = u.get_abilities("teleport");

		std::vector<std::pair<const config *, map_location> > teleports = teleport_list.cfgs;
		std::vector<std::pair<const config *, map_location> >::const_iterator it = teleports.begin();

		for(; it != teleports.end(); ++it) {
			const int tunnel_count = (it->first)->child_count("tunnel");
			for(int i = 0; i < tunnel_count; ++i) {
				config teleport_group_cfg = (it->first)->child("tunnel", i);
				teleport_group group = teleport_group(vconfig(teleport_group_cfg, true), false);
				groups.push_back(group);
			}
		}
	}

	const std::vector<teleport_group>& global_groups = resources::tunnels->get();
	groups.insert(groups.end(), global_groups.begin(), global_groups.end());

	return teleport_map(groups, u, units, viewing_team, see_all, ignore_units);
}

pathfind::manager::manager(const config &cfg) : tunnels_(), id_(lexical_cast_default<int>(cfg["next_teleport_group_id"], 0)) {
	const int tunnel_count = cfg.child_count("tunnel");
	for(int i = 0; i < tunnel_count; ++i) {
		const config& t = cfg.child("tunnel", i);
		const pathfind::teleport_group tunnel(t);
		this->add(tunnel);
	}
}

void pathfind::manager::add(const teleport_group &group) {
	tunnels_.push_back(group);
}

void pathfind::manager::remove(const std::string &id) {
	std::vector<pathfind::teleport_group>::iterator t = tunnels_.begin();
	for(;t != tunnels_.end();) {
		if (t->get_teleport_id() == id || t->get_teleport_id() == id + reversed_suffix) {
			t = tunnels_.erase(t);
		} else {
			++t;
		}
	}
}

const std::vector<pathfind::teleport_group>& pathfind::manager::get() const {
	return tunnels_;
}

config pathfind::manager::to_config() const {
	config store;

	std::vector<pathfind::teleport_group>::const_iterator tunnel = tunnels_.begin();
	for(; tunnel != tunnels_.end(); ++tunnel) {
		store.add_child("tunnel", tunnel->to_config());
	}
	store["next_teleport_group_id"] = str_cast(id_);

	return store;
}

std::string pathfind::manager::next_unique_id() {
	return str_cast(++id_);
}

