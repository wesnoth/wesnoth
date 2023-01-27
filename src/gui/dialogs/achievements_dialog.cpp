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
#include "gui/widgets/grid.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/progress_bar.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(achievements_dialog)

unsigned int achievements_dialog::selected_index_ = 0;

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

	std::vector<achievement_group> groups = game_config_manager::get()->get_achievements();
	// reset the selected achievement group in case add-ons with achievements are uninstalled between closing and re-opening the dialog
	if(selected_index_ > groups.size()) {
		selected_index_ = 0;
	}

	for(const auto& list : groups) {
		// only display the achievements for the first dropdown option on first showing the dialog
		if(content_list.size() == selected_index_) {
			int achieved_count = 0;

			for(const auto& ach : list.achievements_) {
				widget_data row;
				widget_item item;

				if(ach.achieved_) {
					achieved_count++;
				}

				item["label"] = !ach.achieved_ ? ach.icon_ : ach.icon_completed_;
				row.emplace("icon", item);

				if(ach.hidden_ && !ach.achieved_) {
					item["label"] = ach.hidden_name_;
				} else if(!ach.achieved_) {
					std::string name = ach.name_;
					if(ach.max_progress_ != 0 && ach.current_progress_ != -1) {
						name += " ("+std::to_string(ach.current_progress_)+"/"+std::to_string(ach.max_progress_)+")";
					}
					item["label"] = name;
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

				grid& newrow = achievements_box_->add_row(row);
				progress_bar* achievement_progress = static_cast<progress_bar*>(newrow.find("achievement_progress", false));
				if(ach.max_progress_ != 0 && ach.current_progress_ != -1) {
					achievement_progress->set_percentage((ach.current_progress_/double(ach.max_progress_))*100);
				} else {
					achievement_progress->set_visible(gui2::widget::visibility::invisible);
				}
			}

			label* achieved_label = find_widget<label>(&win, "achievement_count", false, true);
			achieved_label->set_label(_("Completed")+" "+std::to_string(achieved_count)+"/"+std::to_string(list.achievements_.size()));
		}

		// populate all possibilities into the dropdown
		content_list.emplace_back("label", list.display_name_);
	}
	if(content_list.size() > 0) {
		content_names_->set_values(content_list);
		content_names_->set_selected(selected_index_, false);
	}
}

void achievements_dialog::post_show(window&)
{
}

void achievements_dialog::set_achievements_content()
{
	achievements_box_->clear();
	int achieved_count = 0;
	selected_index_ = content_names_->get_value();

	achievement_group list = game_config_manager::get()->get_achievements().at(selected_index_);
	for(const auto& ach : list.achievements_) {
		widget_data row;
		widget_item item;

		if(ach.achieved_) {
			achieved_count++;
		}

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

		grid& newrow = achievements_box_->add_row(row);
		progress_bar* achievement_progress = static_cast<progress_bar*>(newrow.find("achievement_progress", false));
		if(ach.max_progress_ != 0 && ach.current_progress_ != -1) {
			achievement_progress->set_percentage((ach.current_progress_/double(ach.max_progress_))*100);
		} else {
			achievement_progress->set_visible(gui2::widget::visibility::invisible);
		}
	}

	label* achieved_label = find_widget<label>(get_window(), "achievement_count", false, true);
	achieved_label->set_label(_("Completed")+" "+std::to_string(achieved_count)+"/"+std::to_string(list.achievements_.size()));
}

} // namespace gui2::dialogs