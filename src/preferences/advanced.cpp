/*
	Copyright (C) 2020 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "preferences/advanced.hpp"

#include "game_config_view.hpp"
#include "game_version.hpp"
#include "gettext.hpp"
#include "log.hpp"

#include <cassert>

static lg::log_domain advanced_preferences("advanced_preferences");
#define ERR_ADV LOG_STREAM(err, advanced_preferences)

namespace preferences
{
static std::unique_ptr<preferences::advanced_manager> singleton = nullptr;

advanced_manager::advanced_manager(const game_config_view& gc)
	: prefs()
{
	for(const config& pref : gc.child_range("advanced_preference")) {
		try {
			prefs.emplace_back(pref);
		} catch(const std::invalid_argument& e) {
			ERR_ADV << e.what();
			continue;
		}
	}

	// show_deprecation has a different default on the dev branch
	if(game_config::wesnoth_version.is_dev_version()) {
		for(option& op : prefs) {
			if(op.field == "show_deprecation") {
				op.cfg["default"] = true;
			}
		}
	}

	std::sort(prefs.begin(), prefs.end(),
		[](const auto& lhs, const auto& rhs) { return translation::icompare(lhs.name, rhs.name) < 0; });
}

advanced_manager::~advanced_manager()
{
}

advanced_manager::option::option(const config& pref)
	: type()
	, name(pref["name"].t_str())
	, description(pref["description"].t_str())
	, field(pref["field"].str())
	, cfg(pref)
{
	const std::string p_type = cfg["type"];

	if(p_type == "boolean") {
		type = avd_type::TOGGLE;
	} else if(p_type == "int") {
		type = avd_type::SLIDER;
	} else if(p_type == "combo") {
		type = avd_type::COMBO;
	} else if(p_type == "custom") {
		type = avd_type::SPECIAL;
	} else {
		throw std::invalid_argument("Unknown type '" + p_type + "' for advanced preference " + name);
	}
}

void init_advanced_manager(const game_config_view& gc)
{
	singleton = std::make_unique<advanced_manager>(gc);
}

const advanced_pref_list& get_advanced_preferences()
{
	assert(singleton && "Advanced preference manager singleton is null!");
	return singleton->get_preferences();
}

} // namespace preferences
