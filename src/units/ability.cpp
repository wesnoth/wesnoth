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
