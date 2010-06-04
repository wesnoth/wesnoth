/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "reports.hpp"


namespace {
	const std::string report_names[] = {
		"unit_name", "unit_type",
		"unit_race", "unit_level", "unit_side", "unit_amla",
		"unit_traits", "unit_status", "unit_alignment", "unit_abilities",
		"unit_hp", "unit_xp", "unit_advancement_options", "unit_defense", "unit_moves",
		"unit_weapons", "unit_image", "unit_profile", "time_of_day",
		"turn", "gold", "villages", "num_units", "upkeep", "expenses",
		"income", "terrain", "position", "side_playing", "observers",
		"report_countdown", "report_clock",
		"selected_terrain", "edit_left_button_function", "editor_tool_hint"
	};
}

namespace reports {

const std::string& report_name(TYPE type)
{
	assert(sizeof(report_names)/sizeof(*report_names) == NUM_REPORTS);
	assert(type < NUM_REPORTS);

	return report_names[type];
}

void report::add_text(const std::string& text,
		const std::string& tooltip, const std::string& action) {
	this->push_back(element(text,"",tooltip,action));
}

void report::add_image(const std::string& image, const std::string& tooltip,
		const std::string& action) {
	this->push_back(element("",image,tooltip,action));
}

}
