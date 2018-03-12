/*
   Copyright (C) 2016 - 2018 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/dialogs/unit_recruit.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/unit_preview_pane.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "gettext.hpp"
#include "help/help.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "units/types.hpp"
#include "whiteboard/manager.hpp"

#include "utils/functional.hpp"

namespace gui2
{
namespace dialogs
{

REGISTER_DIALOG(unit_recruit)

unit_recruit::unit_recruit(std::vector<const unit_type*>& recruit_list, team& team)
	: recruit_list_(recruit_list)
	, team_(team)
	, selected_index_(0)
{
	// Ensure the recruit list is sorted by name
	std::sort(recruit_list_.begin(), recruit_list_.end(), [](const unit_type* t1, const unit_type* t2) {
		return t1->type_name().str() < t2->type_name().str();
	});
}

static std::string can_afford_unit(const std::string& text, const bool can_afford)
{
	return can_afford ? text : "<span color='#ff0000'>" + text + "</span>";
}

void unit_recruit::pre_show(window& window)
{
	listbox& list = find_widget<listbox>(&window, "recruit_list", false);

	connect_signal_notify_modified(list, std::bind(&unit_recruit::list_item_clicked, this, std::ref(window)));

	window.keyboard_capture(&list);

	connect_signal_mouse_left_click(
		find_widget<button>(&window, "show_help", false),
		std::bind(&unit_recruit::show_help, this));

	for(const auto& recruit : recruit_list_)
	{
		std::map<std::string, string_map> row_data;
		string_map column;

		std::string	image_string = recruit->image() + "~RC(" + recruit->flag_rgb() + ">"
			+ team_.color() + ")";

		int wb_gold = 0;
		if(resources::controller) {
			if(const std::shared_ptr<wb::manager>& whiteb = resources::controller->get_whiteboard()) {
				wb::future_map future; // So gold takes into account planned spending
				wb_gold = whiteb->get_spent_gold_for(team_.side());
			}
		}

		const bool can_afford = recruit->cost() <= team_.gold() - wb_gold;

		const std::string cost_string = std::to_string(recruit->cost());

		column["use_markup"] = "true";

		column["label"] = image_string;
		row_data.emplace("unit_image", column);

		column["label"] = can_afford_unit(recruit->type_name(), can_afford);
		row_data.emplace("unit_type", column);

		column["label"] = can_afford_unit(cost_string, can_afford);
		row_data.emplace("unit_cost", column);

		list.add_row(row_data);
	}

	list_item_clicked(window);
}

void unit_recruit::list_item_clicked(window& window)
{
	const int selected_row
		= find_widget<listbox>(&window, "recruit_list", false).get_selected_row();

	if(selected_row == -1) {
		return;
	}

	find_widget<unit_preview_pane>(&window, "recruit_details", false)
		.set_displayed_type(*recruit_list_[selected_row]);
}

void unit_recruit::show_help()
{
	help::show_help("recruit_and_recall");
}

void unit_recruit::post_show(window& window)
{
	if(get_retval() == retval::OK) {
		selected_index_ = find_widget<listbox>(&window, "recruit_list", false)
			.get_selected_row();
	}
}

} // namespace dialogs
} // namespace gui2
