/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-help"

#include "help/section_generators.hpp"

#include "formatter.hpp"
#include "game_config_manager.hpp"
#include "gettext.hpp"
#include "help/manager.hpp"
#include "help/constants.hpp"
#include "help/section.hpp"
#include "help/utils.hpp"
#include "log.hpp"
#include "preferences/game.hpp"
#include "terrain/terrain.hpp"
#include "terrain/translation.hpp"
#include "terrain/type_data.hpp"
#include "units/race.hpp"
#include "units/types.hpp"

#include <set>

static lg::log_domain log_help("help");
#define WRN_HP LOG_STREAM(warn, log_help)
#define DBG_HP LOG_STREAM(debug, log_help)

namespace help
{
void generate_races_sections(section& sec)
{
	std::set<std::string, string_less> races;
	std::set<std::string, string_less> visible_races;

	for(const auto& i : unit_types.types()) {
		const unit_type& type = i.second;

		if(description_type(type) == FULL_DESCRIPTION) {
			races.insert(type.race_id());

			if(!type.hide_help()) {
				visible_races.insert(type.race_id());
			}
		}
	}

	for(const auto& race : races) {
		config section_cfg;
		const bool hidden = (visible_races.count(race) == 0);

		std::string title;
		if(const unit_race* r = unit_types.find_race(race)) {
			title = r->plural_name();
		} else {
			title = _("race^Miscellaneous");
		}

		section_cfg["id"] = hidden_symbol(hidden) + race_prefix + race;
		section_cfg["title"] = title;
		section_cfg["sections_generator"] = "units:" + race;
		section_cfg["generator"] = "units:" + race;

		sec.add_section(section_cfg);
	}
}

void generate_terrain_sections(section& sec)
{
	ter_data_cache tdata = load_terrain_types_data();
	if(!tdata) {
		WRN_HP << "When building terrain help sections, couldn't acquire terrain types data, aborting.\n";
		return;
	}

	std::map<std::string, section*> base_map;

	for(const t_translation::terrain_code& t : tdata->list()) {
		const terrain_type& info = tdata->get_terrain_info(t);

		bool hidden = info.is_combined() || info.hide_help();
		if(preferences::encountered_terrains().find(t) == preferences::encountered_terrains().end()
			&& !info.is_overlay()
		) {
			hidden = true;
		}

		std::string topic_id = hidden_symbol(hidden) + terrain_prefix + info.id();

		for(const t_translation::terrain_code& base : tdata->underlying_union_terrain(t)) {
			const terrain_type& base_info = tdata->get_terrain_info(base);
			const std::string& base_id = base_info.id();

			if(!base_info.is_nonnull() || base_info.hide_help()) {
				continue;
			}

			if(base_id == info.id()) {
				topic_id = ".." + terrain_prefix + info.id();
			}

			section* base_section = nullptr;

			try {
				base_section = base_map.at(base_id);
			} catch(const std::out_of_range&) {
				config base_section_config;
				base_section_config["id"] = terrain_prefix + base_id;
				base_section_config["title"] = base_info.editor_name();

				base_section = base_map[base_id] = sec.add_section(base_section_config);
			}

			// May be null if a parse/recursion depth error ocurred during construction.
			if(base_section) {
				base_section->add_topic<terrain_topic_generator>(topic_id, info.editor_name(), info);
			}
		}
	}

	for(auto& t_sec : base_map) {
		if(section* s = t_sec.second) {
			s->sort_topics();
		}
	}
}

void generate_era_sections(section& sec)
{
	// TODO: should we be using the gcm here?
	for(const config& era : game_config_manager::get()->game_config().child_range("era")) {
		if(era["hide_help"].to_bool()) {
			continue;
		}

		DBG_HP << "Adding help section: " << era["id"].str() << "\n";

		config section_cfg;
		section_cfg["id"] = era_prefix + era["id"].str();
		section_cfg["title"] = era["name"];
		section_cfg["generator"] = "era:" + era["id"].str();

		DBG_HP << section_cfg.debug() << "\n";

		sec.add_section(section_cfg);
	}
}

void generate_unit_sections(section& sec, const std::string& race)
{
	for(const auto& i : unit_types.types()) {
		const unit_type& type = i.second;

		if(type.race_id() != race || !type.show_variations_in_help()) {
			continue;
		}

		config base_unit_config;
		base_unit_config["id"] = hidden_symbol(type.hide_help()) + unit_prefix + type.id();
		base_unit_config["title"] = type.type_name();

		section* base_unit = sec.add_section(base_unit_config);

		// May be null if a parse/recursion depth error ocurred during construction.
		// In that case, no need to parse variations since we have no section to which to
		// append them.
		if(!base_unit) {
			continue;
		}

		// Add topics for each of the unit's variations.
		for(const std::string& variation_id : type.variations()) {
			// TODO: Do we apply encountered stuff to variations?
			const unit_type& var_type = type.get_variation(variation_id);

			const std::string topic_name = var_type.type_name() + "\n" + var_type.variation_name();
			const std::string topic_id = formatter()
				<< hidden_symbol(var_type.hide_help()) << variation_prefix << var_type.id() << "_" << variation_id;

			base_unit->add_topic<unit_topic_generator>(topic_id, topic_name, var_type, variation_id);
		}
	}
}

} // namespace help
