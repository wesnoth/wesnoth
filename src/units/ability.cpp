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

#include "game_version.hpp" // for version_info in deprecated_message
#include "deprecation.hpp"

unit_ability_t::unit_ability_t(std::string tag, config cfg)
		: tag_(std::move(tag))
		, cfg_(std::move(cfg))
{
	//Add wml filter if "backstab" attribute used.
	if (!cfg_["backstab"].blank()) {
		deprecated_message("backstab= in weapon specials", DEP_LEVEL::INDEFINITE, "", "Use [filter_opponent] with a formula instead; the code can be found in data/core/macros/ in the WEAPON_SPECIAL_BACKSTAB macro.");
	}
	if(cfg_["backstab"].to_bool()){
		const std::string& backstab_formula = "enemy_of(self, flanker) and not flanker.petrified where flanker = unit_at(direction_from(loc, other.facing))";
		config& filter_opponent = cfg.child_or_add("filter_opponent");
		if(!filter_opponent.empty()) {
			filter_opponent = filter_opponent.add_child("and");
		}
		filter_opponent["formula"] = backstab_formula;
	}
	cfg_.remove_attribute("backstab");
}

void unit_ability_t::parse_vector(const config& abilities_cfg, std::vector<ability_ptr>& res)
{
	for(auto item : abilities_cfg.all_children_range()) {
		res.push_back(std::make_shared<unit_ability_t>(item.key, item.cfg));
	}
}

ability_vector unit_ability_t::cfg_to_vector(const config& abilities_cfg)
{
	ability_vector res;
	parse_vector(abilities_cfg, res);
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
