/*
   Copyright (C) 2017-2018 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"


#include "gui/dialogs/label_settings.hpp"

#include "display.hpp"
#include "font/text_formatting.hpp"
#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"
#include "map/label.hpp"
#include "team.hpp"
#include "utils/functional.hpp"

#include <vector>

namespace gui2
{
namespace dialogs
{
REGISTER_DIALOG(label_settings)

label_settings::label_settings(display_context& dc)
	: viewer(dc)
{
	const std::vector<std::string>& all_categories = display::get_singleton()->labels().all_categories();
	const std::vector<std::string>& hidden_categories = viewer.hidden_label_categories();

	for(const std::string& cat : all_categories) {
		all_labels[cat] = true;

		if(cat.substr(0, 4) == "cat:") {
			labels_display[cat] = cat.substr(4);
		} else if(cat == "team") {
			labels_display[cat] = _("Team Labels");
		}

		// TODO: Translatable names for categories?
	}

	for(const std::string& hidden_cat : hidden_categories) {
		all_labels[hidden_cat] = false;
	}

	for(const team& team : dc.teams()) {
		const std::string label_cat_key = "side:" + std::to_string(team.side());

		if(team.hidden()) {
			labels_display[label_cat_key] = "";
			continue;
		}

		std::string team_name = team.side_name();
		if(team_name.empty()) {
			team_name = team.user_team_name();
		}

		if(team_name.empty()) {
			team_name = _("Unknown");
		}

		widget_item subst;
		subst["side_number"] = std::to_string(team.side());
		subst["name"] = team_name;

		labels_display[label_cat_key] = VGETTEXT("Side $side_number ($name)", subst);
	}
}

void label_settings::pre_show(window& window)
{
	widget_data list_data;
	listbox& cats_listbox = find_widget<listbox>(&window, "label_types", false);

	for(const auto& label_entry : all_labels) {
		const std::string& category = label_entry.first;
		const bool& visible = label_entry.second;

		std::string name = labels_display[category];
		if(category.substr(0, 5) == "side:") {
			// This means it's a hidden side, so don't show it.
			if(name.empty()) {
				continue;
			}

			int team = std::stoi(category.substr(5)) - 1;
			color_t which_color = game_config::tc_info(viewer.teams()[team].color())[0];

			name = formatter() << font::span_color(which_color) << name << "</span>";
		}

		list_data["cat_name"]["label"] = name;
		grid* grid = &cats_listbox.add_row(list_data);

		toggle_button& status = find_widget<toggle_button>(grid, "cat_status", false);
		status.set_value(visible);

		connect_signal_notify_modified(status, std::bind(&label_settings::toggle_category, this, _1, category));

		if(category.substr(0, 5) == "side:") {
			label& cat_name = find_widget<label>(grid, "cat_name", false);
			cat_name.set_use_markup(true);
		}
	}
}

bool label_settings::execute(display_context& dc)
{
	label_settings window(dc);
	if(!window.show()) {
		return false;
	}

	std::vector<std::string> hidden_categories;
	for(auto lbl : window.all_labels) {
		if(lbl.second == false) {
			hidden_categories.push_back(lbl.first);
		}
	}

	dc.hidden_label_categories_ref().swap(hidden_categories);
	return true;
}

void label_settings::toggle_category(widget& box, std::string category)
{
	all_labels[category] = static_cast<toggle_button&>(box).get_value_bool();
}
} // namespace dialogs
} // namespace gui2
