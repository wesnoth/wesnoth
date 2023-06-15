/*
	Copyright (C) 2009 - 2023
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
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/text_box.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(editor_choose_addon)

editor_choose_addon::editor_choose_addon(std::string& addon_id)
    : modal_dialog(window_id())
    , addon_id_(addon_id)
{
	listbox& existing_addons = find_widget<listbox>(get_window(), "existing_addons", false);
	std::vector<std::string> dirs;
	filesystem::get_files_in_dir(filesystem::get_addons_dir(), nullptr, &dirs, filesystem::name_mode::FILE_NAME_ONLY);

	const widget_data& mainline{
		{ "existing_addon_id",    widget_item{{"label", "mainline"}} },
	};
	existing_addons.add_row(mainline);
	for(const std::string& dir : dirs) {
		const widget_data& entry{
			{ "existing_addon_id",    widget_item{{"label", dir}} },
		};
		existing_addons.add_row(entry);
	}
}

void editor_choose_addon::pre_show(window& win)
{
	text_box* id = find_widget<text_box>(&win, "addon_id", false, true);
	win.keyboard_capture(id);
}

void editor_choose_addon::post_show(window& win)
{
	// use the textbox value if not empty
	// else use the selected addon from the list
	if(std::string value = find_widget<text_box>(&win, "addon_id", false, true)->get_value(); !value.empty()) {
    	addon_id_ = value;
	} else {
		listbox& existing_addons = find_widget<listbox>(get_window(), "existing_addons", false);
		grid* row = existing_addons.get_row_grid(existing_addons.get_selected_row());
		addon_id_ = dynamic_cast<label*>(row->find("existing_addon_id", false))->get_label();
	}
}

}