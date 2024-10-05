/*
	Copyright (C) 2009 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "gui/dialogs/editor/choose_addon.hpp"

#include "filesystem.hpp"
#include "gettext.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "preferences/preferences.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(editor_choose_addon)

editor_choose_addon::editor_choose_addon(std::string& addon_id)
	: modal_dialog(window_id())
	, addon_id_(addon_id)
{
	connect_signal_mouse_left_click(
		find_widget<toggle_button>("show_all"),
		std::bind(&editor_choose_addon::toggle_installed, this));

	populate_list(false);
}

void editor_choose_addon::post_show()
{
	listbox& existing_addons = find_widget<listbox>("existing_addons");
	int selected_row = existing_addons.get_selected_row();

	if(selected_row == 0) {
		addon_id_ = "///newaddon///";
		prefs::get().set_editor_chosen_addon("");
	} else if(selected_row == 1 && find_widget<toggle_button>("show_all").get_value_bool()) {
		addon_id_ = "mainline";
		prefs::get().set_editor_chosen_addon("");
	} else {
		grid* row = existing_addons.get_row_grid(selected_row);
		addon_id_ = dynamic_cast<label*>(row->find("existing_addon_id", false))->get_label();
		prefs::get().set_editor_chosen_addon(addon_id_);
	}
}

void editor_choose_addon::toggle_installed()
{
	toggle_button& show_all = find_widget<toggle_button>("show_all");
	populate_list(show_all.get_value_bool());
}

void editor_choose_addon::populate_list(bool show_all)
{
	listbox& existing_addons = find_widget<listbox>("existing_addons");
	existing_addons.clear();

	std::vector<std::string> dirs;
	filesystem::get_files_in_dir(filesystem::get_addons_dir(), nullptr, &dirs, filesystem::name_mode::FILE_NAME_ONLY);

	const widget_data& new_addon{
		{"existing_addon_id", widget_item{{"label", _("New Add-on")}, {"tooltip", _("Create a new add-on")}}},
	};
	existing_addons.add_row(new_addon);

	if(show_all) {
		const widget_data& mainline{
			{"existing_addon_id",
				widget_item{{"label", _("Mainline")}, {"tooltip", _("Mainline multiplayer scenarios")}}},
		};
		existing_addons.add_row(mainline);
	}

	int selected_row = 0;
	for(const std::string& dir : dirs) {
		if((show_all || filesystem::file_exists(filesystem::get_addons_dir() + "/" + dir + "/_server.pbl"))
			&& filesystem::file_exists(filesystem::get_addons_dir() + "/" + dir + "/_main.cfg")) {
			const widget_data& entry{
				{"existing_addon_id", widget_item{{"label", dir}}},
			};
			existing_addons.add_row(entry);
			if(dir == prefs::get().editor_chosen_addon()) {
				selected_row = existing_addons.get_item_count()-1;
			}
		}
	}

	existing_addons.select_row(selected_row);
}

} // namespace gui2::dialogs
