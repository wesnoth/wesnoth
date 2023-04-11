/*
	Copyright (C) 2003 - 2022
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "units/ability.hpp"

#include "deprecation.hpp"
#include "game_version.hpp" // for version_info in deprecated_message
#include "map/map.hpp"

namespace {
	void do_ability_compat_fixes(config& cfg, bool is_attack_ability = false)
	{
		//Add wml filter if "backstab" attribute used.
		if (!cfg["backstab"].blank()) {
			deprecated_message("backstab= in weapon specials", DEP_LEVEL::INDEFINITE, "", "Use [filter_opponent] with a formula instead; the code can be found in data/core/macros/ in the WEAPON_SPECIAL_BACKSTAB macro.");
		}
		if(cfg["backstab"].to_bool()){
			const std::string& backstab_formula = "enemy_of(self, flanker) and not flanker.petrified where flanker = unit_at(direction_from(loc, other.facing))";
			config& filter_opponent = cfg.child_or_add("filter_opponent");
			if(!filter_opponent.empty()) {
				filter_opponent = filter_opponent.add_child("and");
			}
			filter_opponent["formula"] = backstab_formula;
		}
		cfg.remove_attribute("backstab");

		std::string filter_teacher = is_attack_ability ? "filter_self" : "filter";
		for (config &filter_adjacent : cfg.child_range("filter_adjacent")) {
			if(filter_adjacent["count"].empty()) {
				deprecated_message("omitting count= in [filter_adjacent] in abilities", DEP_LEVEL::FOR_REMOVAL, version_info("1.19"), "specify count explicitly");
				filter_adjacent["count"] = map_location::parse_directions(filter_adjacent["adjacent"]).size();
			}
			cfg.child_or_add(filter_teacher).add_child("filter_adjacent", filter_adjacent);
		}
		for (config &filter_adjacent : cfg.child_range("filter_adjacent_location")) {
			if(filter_adjacent["count"].empty()) {
				deprecated_message("omitting count= in [filter_adjacent_location] in abilities", DEP_LEVEL::FOR_REMOVAL, version_info("1.19"), "specify count explicitly");
				filter_adjacent["count"] = map_location::parse_directions(filter_adjacent["adjacent"]).size();
			}
			cfg.child_or_add(filter_teacher).add_child("filter_location").add_child("filter_adjacent_location", filter_adjacent);
		}
		cfg.clear_children("filter_adjacent", "filter_adjacent_location");
	}
}

unit_ability_t::unit_ability_t(std::string tag, config cfg, bool is_attack_ability)
	: tag_(std::move(tag))
	, cfg_(std::move(cfg))
{
	do_ability_compat_fixes(cfg_, is_attack_ability);
}

void unit_ability_t::parse_vector(const config& abilities_cfg, std::vector<ability_ptr>& res, bool is_attack_ability)
{
	for(auto item : abilities_cfg.all_children_range()) {
		res.push_back(std::make_shared<unit_ability_t>(item.key, item.cfg, is_attack_ability));
	}
}

ability_vector unit_ability_t::cfg_to_vector(const config& abilities_cfg, bool is_attack_ability)
{
	ability_vector res;
	parse_vector(abilities_cfg, res, is_attack_ability);
	return res;
}

config unit_ability_t::vector_to_cfg(const ability_vector& abilities)
{
	config abilities_cfg;
	for(const auto& item : abilities) {
		item->write(abilities_cfg);
	}
	return abilities_cfg;
}


void unit_ability_t::write(config& abilities_cfg)
{
	abilities_cfg.add_child(tag(), cfg());
}
