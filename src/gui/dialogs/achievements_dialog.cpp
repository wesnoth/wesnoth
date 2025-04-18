/*
	Copyright (C) 2003 - 2025
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

#include "formula/string_utils.hpp"
#include "game_config_manager.hpp"
#include "gettext.hpp"
#include "gui/widgets/drawing.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/progress_bar.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "preferences/preferences.hpp"
#include "serialization/markup.hpp"

static lg::log_domain log_config("config");
#define ERR_CONFIG LOG_STREAM(err, log_config)

constexpr int sub_achievements_limit = 28;

namespace gui2::dialogs
{

REGISTER_DIALOG(achievements_dialog)

achievements_dialog::achievements_dialog()
	: modal_dialog(window_id())
	, last_selected_(prefs::get().selected_achievement_group())
	, achievements_box_(nullptr)
	, content_names_(nullptr)
{
}

void achievements_dialog::pre_show()
{
	std::vector<config> content_list;
	content_names_ = &find_widget<menu_button>("selected_achievements_list");
	connect_signal_notify_modified(*content_names_, std::bind(&achievements_dialog::set_achievements_row, this));

	achievements_box_ = find_widget<listbox>("achievements_list", false, true);
	int selected = 0;

	for(const auto& list : game_config_manager::get()->get_achievements()) {
		// only display the achievements for the first dropdown option on first showing the dialog
		if(list.content_for_ == last_selected_ || last_selected_ == "") {
			selected = content_list.size();
		}

		// populate all possibilities into the dropdown
		content_list.emplace_back("label", list.display_name_);
	}

	if(content_list.size() > 0) {
		content_names_->set_values(content_list);
		content_names_->set_selected(selected, false);
		set_achievements_row();
	}
}

void achievements_dialog::post_show()
{
	prefs::get().set_selected_achievement_group(last_selected_);
}

void achievements_dialog::set_achievements_row()
{
	const achievement_group& list = game_config_manager::get()->get_achievements().at(content_names_->get_value());
	achievements_box_->clear();
	last_selected_ = list.content_for_;
	int achieved_count = 0;

	for(const auto& ach : list.achievements_) {
		if(ach.achieved_) {
			achieved_count++;
		} else if(ach.hidden_ && !ach.achieved_) {
			continue;
		}

		const bool in_progress = ach.max_progress_ != 0 && ach.current_progress_ != -1;
		const auto in_progress_name = !in_progress
			? ach.name_
			: t_string(VGETTEXT("$title ($count/$total)", {{
				{"title", ach.name_},
				{"count", std::to_string(ach.current_progress_)},
				{"total", std::to_string(ach.max_progress_)}
			}}));

		grid& newrow = achievements_box_->add_row(widget_data{
			{ "icon", {
				{ "label", ach.achieved_
					? ach.icon_completed_
					: ach.icon_
				}
			}},
			{ "name", {
				{ "label", ach.achieved_
					? ach.name_completed_
					: in_progress_name
				}
			}},
			{ "description", {
				{ "label", ach.achieved_
					? t_string(markup::span_color("green", ach.description_completed_))
					: ach.description_
				}
			}}
		});

		auto achievement_progress = static_cast<progress_bar*>(newrow.find("achievement_progress", false));
		if(in_progress) {
			achievement_progress->set_percentage((ach.current_progress_ / double(ach.max_progress_)) * 100);
		} else {
			achievement_progress->set_visible(gui2::widget::visibility::invisible);
		}

		auto name = static_cast<styled_widget*>(newrow.find("name", false));
		name->get_canvas(0).set_variable("achieved", wfl::variant(ach.achieved_));

		set_sub_achievements(newrow, ach);
	}

	auto& achieved_label = find_widget<label>("achievement_count");
	achieved_label.set_label(VGETTEXT("Completed $count/$total", {
		{"count", std::to_string(achieved_count)} ,
		{"total", std::to_string(list.achievements_.size())}
	}));
}

void achievements_dialog::set_sub_achievements(grid& newrow, const achievement& ach)
{
	int i = 0;

	// set any sub achievements
	for(const sub_achievement& sub_ach : ach.sub_achievements_)
	{
		if(i == sub_achievements_limit) {
			ERR_CONFIG << "Too many sub achievements";
			break;
		} else {
			drawing* img = static_cast<drawing*>(newrow.find("sub_icon" + std::to_string(i), false));
			img->set_label(sub_ach.achieved_ ? sub_ach.icon_completed_ : sub_ach.icon_);
			img->set_tooltip(sub_ach.description_);
		}
		i++;
	}

	// if an achievement hasn't defined the maximum possible sub-achievements, hide the [image]s for the rest
	for(; i < sub_achievements_limit; i++)
	{
		newrow.find("sub_icon" + std::to_string(i), false)->set_visible(visibility::invisible);
	}
}

} // namespace gui2::dialogs
