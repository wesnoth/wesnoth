/*
	Copyright (C) 2016 - 2024
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

#include "gui/dialogs/unit_advance.hpp"

#include "gui/widgets/button.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/unit_preview_pane.hpp"
#include "gui/widgets/window.hpp"
#include "units/unit.hpp"
#include "units/types.hpp"
#include "help/help.hpp"

#include <functional>

namespace gui2::dialogs
{

REGISTER_DIALOG(unit_advance)

unit_advance::unit_advance(const std::vector<unit_const_ptr>& samples, std::size_t real)
	: modal_dialog(window_id())
	, previews_(samples)
	, selected_index_(0)
	, last_real_advancement_(real)
{
}

void unit_advance::pre_show()
{
	listbox& list = find_widget<listbox>("advance_choice");

	connect_signal_notify_modified(list, std::bind(&unit_advance::list_item_clicked, this));

	keyboard_capture(&list);

	connect_signal_mouse_left_click(
		find_widget<button>("show_help"),
		std::bind(&unit_advance::show_help, this));

	for(std::size_t i = 0; i < previews_.size(); i++) {
		const unit& sample = *previews_[i];

		widget_data row_data;
		widget_item column;

		std::string image_string, name = sample.type_name();

		// This checks if we've finished iterating over the last unit type advancements
		// and are into the modification-based advancements.
		if(i >= last_real_advancement_) {
			const auto range = sample.get_modifications().child_range("advancement");
			const auto& back = range.back();

			if(back.has_attribute("image")) {
				image_string = back["image"].str();
			}

			name = back["description"].str();
		}

		if(image_string.empty()) {
			image_string = sample.type().image() + sample.image_mods();
		}

		column["label"] = image_string;
		row_data.emplace("advancement_image", column);

		column["label"] = name;
		row_data.emplace("advancement_name", column);

		list.add_row(row_data);
	}

	list_item_clicked();

	// Disable ESC existing
	set_escape_disabled(true);
}

void unit_advance::list_item_clicked()
{
	const int selected_row
		= find_widget<listbox>("advance_choice").get_selected_row();

	if(selected_row == -1) {
		return;
	}

	find_widget<unit_preview_pane>("advancement_details")
		.set_display_data(*previews_[selected_row]);
}

void unit_advance::show_help()
{
	help::show_help("advancement");
}

void unit_advance::post_show()
{
	if(get_retval() == retval::OK) {
		selected_index_ = find_widget<listbox>("advance_choice")
			.get_selected_row();
	}
}

} // namespace dialogs
