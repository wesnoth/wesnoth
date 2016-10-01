/*
   Copyright (C) 2009 - 2016 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/unit_create.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/log.hpp"
#include "gui/dialogs/helper.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/unit_preview_pane.hpp"
#include "gui/widgets/window.hpp"
#include "marked-up_text.hpp"
#include "help/help.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "units/types.hpp"

#include "utils/functional.hpp"
#include <boost/dynamic_bitset.hpp>

static std::string last_chosen_type_id = "";
static unit_race::GENDER last_gender = unit_race::MALE;

namespace gui2
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_unit_create
 *
 * == Unit create ==
 *
 * This shows the debug-mode dialog to create new units on the map.
 *
 * @begin{table}{dialog_widgets}
 *
 * male_toggle & & toggle_button & m &
 *         Option button to select the "male" gender for created units. $
 *
 * female_toggle & & toggle_button & m &
 *         Option button to select the "female" gender for created units. $
 *
 * unit_type_list & & listbox & m &
 *         Listbox displaying existing unit types sorted by name and race. $
 *
 * -unit_type & & control & m &
 *         Widget which shows the unit type name label. $
 *
 * -race & & control & m &
 *         Widget which shows the unit race name label. $
 *
 * @end{table}
 */

REGISTER_DIALOG(unit_create)

tunit_create::tunit_create()
	: gender_(last_gender)
	, choice_(last_chosen_type_id)
	, last_words_()
{
}

void tunit_create::pre_show(twindow& window)
{
	ttoggle_button& male_toggle
			= find_widget<ttoggle_button>(&window, "male_toggle", false);
	ttoggle_button& female_toggle
			= find_widget<ttoggle_button>(&window, "female_toggle", false);

	gender_toggle.add_member(&male_toggle, unit_race::MALE);
	gender_toggle.add_member(&female_toggle, unit_race::FEMALE);

	gender_toggle.set_member_states(last_gender);

	gender_toggle.set_callback_on_value_change(
			dialog_callback<tunit_create, &tunit_create::gender_toggle_callback>);

	tlistbox& list = find_widget<tlistbox>(&window, "unit_type_list", false);

	ttext_box* filter
			= find_widget<ttext_box>(&window, "filter_box", false, true);

	filter->set_text_changed_callback(
			std::bind(&tunit_create::filter_text_changed, this, _1, _2));

	window.keyboard_capture(filter);
	window.add_to_keyboard_chain(&list);

#ifdef GUI2_EXPERIMENTAL_LISTBOX
	connect_signal_notify_modified(*list,
								   std::bind(&tunit_create::list_item_clicked,
											   *this,
											   std::ref(window)));
#else
	list.set_callback_value_change(
			dialog_callback<tunit_create, &tunit_create::list_item_clicked>);
#endif

	list.clear();

	for(const auto & i : unit_types.types())
	{
		if(i.second.do_not_list())
			continue;

		// Make sure this unit type is built with the data we need.
		unit_types.build_unit_type(i.second, unit_type::FULL);

		units_.push_back(&i.second);

		std::map<std::string, string_map> row_data;
		string_map column;

		column["label"] = units_.back()->race()->plural_name();
		row_data.emplace("race", column);

		column["label"] = units_.back()->type_name();
		row_data.emplace("unit_type", column);

		list.add_row(row_data);

		// Select the previous choice, if any.
		if(choice_.empty() != true && choice_ == i.first) {
			list.select_row(list.get_item_count() - 1);
		}
	}

	if(units_.empty()) {
		ERR_GUI_G << "no unit types found for unit create dialog; not good"
				  << std::endl;
	}

	list.register_sorting_option(0, [this](const int i) { return (*units_[i]).race()->plural_name().str(); });
	list.register_sorting_option(1, [this](const int i) { return (*units_[i]).type_name().str(); });

	list_item_clicked(window);
}

void tunit_create::post_show(twindow& window)
{
	tlistbox& list = find_widget<tlistbox>(&window, "unit_type_list", false);

	choice_ = "";

	if(get_retval() != twindow::OK) {
		return;
	}

	const int selected_row = list.get_selected_row();
	if(selected_row < 0) {
		return;
	} else if(static_cast<size_t>(selected_row) >= units_.size()) {
		// FIXME: maybe assert?
		ERR_GUI_G << "unit create dialog has more list items than known unit "
					 "types; not good\n";
		return;
	}

	last_chosen_type_id = choice_ = units_[selected_row]->id();
	last_gender = gender_;
}

void tunit_create::list_item_clicked(twindow& window)
{
	const int selected_row
		= find_widget<tlistbox>(&window, "unit_type_list", false).get_selected_row();

	if(selected_row == -1) {
		return;
	}

	find_widget<tunit_preview_pane>(&window, "unit_details", false)
		.set_displayed_type(*units_[selected_row]);

	gender_toggle.set_member_active(unit_race::GENDER::MALE,
		units_[selected_row]->has_gender_variation(unit_race::GENDER::MALE));
	gender_toggle.set_member_active(unit_race::GENDER::FEMALE,
		units_[selected_row]->has_gender_variation(unit_race::GENDER::FEMALE));
}

void tunit_create::filter_text_changed(ttext_* textbox, const std::string& text)
{
	twindow& window = *textbox->get_window();

	tlistbox& list = find_widget<tlistbox>(&window, "unit_type_list", false);

	const std::vector<std::string> words = utils::split(text, ' ');

	if(words == last_words_)
		return;
	last_words_ = words;

	boost::dynamic_bitset<> show_items;
	show_items.resize(list.get_item_count(), true);

	if(!text.empty()) {
		for(unsigned int i = 0; i < list.get_item_count(); i++) {
			tgrid* row = list.get_row_grid(i);

			tgrid::iterator it = row->begin();
			tlabel& type_label
					= find_widget<tlabel>(*it, "unit_type", false);

			bool found = false;
			for(const auto & word : words)
			{
				found = std::search(type_label.label().str().begin(),
									type_label.label().str().end(),
									word.begin(),
									word.end(),
									chars_equal_insensitive)
						!= type_label.label().str().end();

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

void tunit_create::gender_toggle_callback(twindow&)
{
	gender_ = gender_toggle.get_active_member_value();
}
}
