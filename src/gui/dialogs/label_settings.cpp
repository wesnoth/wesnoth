/*
	Copyright (C) 2017 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "serialization/markup.hpp"
#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"
#include "map/label.hpp"
#include "team.hpp"

#include <vector>

namespace gui2::dialogs
{
REGISTER_DIALOG(label_settings)

label_settings::label_settings(display_context& dc)
	: modal_dialog(window_id())
	, viewer_(dc)
{
	const std::vector<std::string>& all_categories = display::get_singleton()->labels().all_categories();
	const std::vector<std::string>& hidden_categories = viewer_.hidden_label_categories();

	for(const std::string& cat : all_categories) {
		all_labels_[cat] = true;

		// TODO: Translatable names for categories?
		if(cat.substr(0, 4) == "cat:") {
			labels_display_[cat] = cat.substr(4);
		} else if(cat == "team") {
			labels_display_[cat] = _("Team Labels");
		}
	}

	for(const std::string& hidden_cat : hidden_categories) {
		all_labels_[hidden_cat] = false;
	}

	for(const team& team : viewer_.teams()) {
		const std::string label_cat_key = "side:" + std::to_string(team.side());

		if(team.hidden()) {
			labels_display_[label_cat_key] = "";
			continue;
		}

		std::string team_name = team.side_name();
		if(team_name.empty()) {
			team_name = team.user_team_name();
		}

		if(team_name.empty()) {
			team_name = _("Unknown");
		}

		utils::string_map subst;
		subst["side_number"] = std::to_string(team.side());
		subst["name"] = team_name;
		labels_display_[label_cat_key] = VGETTEXT("Side $side_number ($name)", subst);
	}
}

void label_settings::pre_show()
{
	listbox& cats_listbox = find_widget<listbox>("label_types");
	widget_data list_data;

	for(const auto& label_entry : all_labels_) {
		const std::string& category = label_entry.first;
		const bool visible = label_entry.second;

		std::string name = labels_display_[category];
		if(category.substr(0, 5) == "side:") {
			// This means it's a hidden side, so don't show it.
			if(name.empty()) {
				continue;
			}

			const int team = std::stoi(category.substr(5)) - 1;
			const color_t tc = game_config::tc_info(viewer_.teams()[team].color())[0];

			name = markup::span_color(tc, name);
		}

		list_data["cat_name"]["label"] = name;
		grid* grid = &cats_listbox.add_row(list_data);

		toggle_button& status = grid->find_widget<toggle_button>("cat_status");
		status.set_value(visible);

		connect_signal_notify_modified(status, std::bind(&label_settings::toggle_category, this, std::placeholders::_1, category));

		if(category.substr(0, 5) == "side:") {
			label& cat_name = grid->find_widget<label>("cat_name");
			cat_name.set_use_markup(true);
		}
	}
}

void label_settings::post_show()
{
	if(get_retval() == retval::OK) {
		std::vector<std::string> hidden_categories;

		for(const auto& lbl : all_labels_) {
			if(lbl.second == false) {
				hidden_categories.push_back(lbl.first);
			}
		}

		viewer_.hidden_label_categories().swap(hidden_categories);
	}
}

void label_settings::toggle_category(widget& box, const std::string& category)
{
	all_labels_[category] = static_cast<toggle_button&>(box).get_value_bool();
}

} // namespace dialogs
