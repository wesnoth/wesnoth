/*
   Copyright (C) 2016 - 2018 by the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "gui/dialogs/unit_recruit.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/unit_preview_pane.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/text_box_base.hpp"
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

namespace
{

bool ci_search(const std::string& a, const std::string& b)
{
	return std::search(a.begin(), a.end(),
	                   b.begin(), b.end(),
	                   chars_equal_insensitive) != a.end();
}

} // end unnamed namespace

// Compare unit_create::filter_text_change
void unit_recruit::filter_text_changed(text_box_base* textbox, const std::string& text)
{
	window& window = *textbox->get_window();

	listbox& list = find_widget<listbox>(&window, "recruit_list", false);

	const std::vector<std::string> words = utils::split(text, ' ');

	if(words == last_words_)
		return;
	last_words_ = words;

	boost::dynamic_bitset<> show_items;
	show_items.resize(list.get_item_count(), true);

	if(!text.empty()) {
		for(unsigned int i = 0; i < list.get_item_count(); i++) {
			assert(i < recruit_list_.size());
			const unit_type* type = recruit_list_[i];
			if(!type) continue;

			bool found = false;
			for(const auto & word : words)
			{
				// Search for the name in the local language.
				// In debug mode, also search for the type id.
				found =
				        (game_config::debug && ci_search(type->id(), word)) ||
					ci_search(type->type_name(), word);

				if(!found) {
					// one word doesn't match, we don't reach words.end()
					break;
				}
			}

			show_items[i] = found;
		}
	}

	list.set_row_shown(show_items);
}

void unit_recruit::pre_show(window& window)
{
	text_box* filter = find_widget<text_box>(&window, "filter_box", false, true);
	filter->set_text_changed_callback(
			std::bind(&unit_recruit::filter_text_changed, this, _1, _2));

	listbox& list = find_widget<listbox>(&window, "recruit_list", false);

	connect_signal_notify_modified(list, std::bind(&unit_recruit::list_item_clicked, this, std::ref(window)));

	window.keyboard_capture(filter);
	window.add_to_keyboard_chain(&list);

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
