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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/achievements_dialog.hpp"

#include "game_config_manager.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(achievements_dialog)

achievements_dialog::achievements_dialog()
	: modal_dialog(window_id())
	, achieve_()
	, achievements_box_(nullptr)
	, content_names_(nullptr)
{
}

void achievements_dialog::pre_show(window& win)
{
	std::vector<config> content_list;
	content_names_ = &find_widget<menu_button>(&win, "selected_achievements_list", false);
	connect_signal_notify_modified(*content_names_, std::bind(&achievements_dialog::set_achievements_content, this));

	achievements_box_ = find_widget<listbox>(&win, "achievements_list", false, true);

auto* a = game_config_manager::get();
auto b = a->get_achievements();
	for(const auto& list : b) {
		// populate all possibilities into the dropdown
		content_list.emplace_back("label", list.display_name_);

		// only display the achievements for the first dropdown option on first showing the dialog
		if(content_list.size() == 1) {
			for(const auto& ach : list.achievements_) {
				widget_data row;
				widget_item item;

				item["label"] = !ach.achieved_ ? ach.icon_ : ach.icon_completed_;
				row.emplace("icon", item);

				if(ach.hidden_ && !ach.achieved_) {
					item["label"] = ach.hidden_name_;
				} else if(!ach.achieved_) {
					item["label"] = ach.name_;
				} else {
					item["label"] = "<span color='green'>"+ach.name_completed_+"</span>";
				}
				row.emplace("name", item);

				if(ach.hidden_ && !ach.achieved_) {
					item["label"] = ach.hidden_hint_;
				} else if(!ach.achieved_) {
					item["label"] = ach.description_;
				} else {
					item["label"] = "<span color='green'>"+ach.description_completed_+"</span>";
				}
				row.emplace("description", item);

				achievements_box_->add_row(row);
			}
		}
	}
	if(content_list.size() > 0) {
		content_names_->set_values(content_list);
	}
}

void achievements_dialog::post_show(window&)
{
}

void achievements_dialog::set_achievements_content()
{
	achievements_box_->clear();
	for(const auto& ach : game_config_manager::get()->get_achievements().at(content_names_->get_value()).achievements_) {
		widget_data row;
		widget_item item;

		item["label"] = !ach.achieved_ ? ach.icon_ : ach.icon_completed_;
		row.emplace("icon", item);

		if(ach.hidden_ && !ach.achieved_) {
			item["label"] = ach.hidden_name_;
		} else if(!ach.achieved_) {
			item["label"] = ach.name_;
		} else {
			item["label"] = "<span color='green'>"+ach.name_completed_+"</span>";
		}
		row.emplace("name", item);

		if(ach.hidden_ && !ach.achieved_) {
			item["label"] = ach.hidden_hint_;
		} else if(!ach.achieved_) {
			item["label"] = ach.description_;
		} else {
			item["label"] = "<span color='green'>"+ach.description_completed_+"</span>";
		}
		row.emplace("description", item);

		achievements_box_->add_row(row);
	}
}

} // namespace gui2::dialogs