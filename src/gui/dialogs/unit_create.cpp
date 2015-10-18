/*
   Copyright (C) 2009 - 2015 by Ignacio R. Morelle <shadowm2006@gmail.com>
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

#include "gui/auxiliary/find_widget.tpp"
#include "gui/auxiliary/log.hpp"
#include "gui/dialogs/helper.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"
#include "unit_types.hpp"
#include "utils/foreach.tpp"

#include <boost/bind.hpp>

namespace
{
static std::string last_chosen_type_id = "";
static unit_race::GENDER last_gender = unit_race::MALE;

/**
 * Helper function for updating the male/female checkboxes.
 * It's not a private member of class gui2::tunit_create so
 * we don't have to expose a forward-declaration of ttoggle_button
 * in the interface.
 */
void update_male_female_toggles(gui2::ttoggle_button& male,
								gui2::ttoggle_button& female,
								unit_race::GENDER choice)
{
	male.set_value(choice == unit_race::MALE);
	female.set_value(choice == unit_race::FEMALE);
}
}

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
	: gender_(last_gender), choice_(last_chosen_type_id), type_info_()
{
}

void tunit_create::pre_show(CVideo& /*video*/, twindow& window)
{
	ttoggle_button& male_toggle
			= find_widget<ttoggle_button>(&window, "male_toggle", false);
	ttoggle_button& female_toggle
			= find_widget<ttoggle_button>(&window, "female_toggle", false);
	tlistbox& list = find_widget<tlistbox>(&window, "unit_type_list", false);

	male_toggle.set_callback_state_change(
			dialog_callback<tunit_create,
							&tunit_create::gender_toggle_callback>);
	female_toggle.set_callback_state_change(
			dialog_callback<tunit_create,
							&tunit_create::gender_toggle_callback>);
	update_male_female_toggles(male_toggle, female_toggle, gender_);
	list.clear();

	// We use this container to "map" unit_type ids to list subscripts
	// later, so it ought to be empty before proceeding.
	type_info_.clear();

	FOREACH(const AUTO & i, unit_types.types())
	{
		if(i.second.do_not_list())
			continue;

		// Make sure this unit type is built with the data we need.
		unit_types.build_unit_type(i.second, unit_type::HELP_INDEXED);

		// Create a race name/type id pair for each unit type
		type_info_.push_back(std::make_pair(i.second.race()->plural_name(), i.first));

		std::map<std::string, string_map> row_data;
		string_map column;

		column["label"] = type_info_.back().first;
		row_data.insert(std::make_pair("race", column));
		column["label"] = type_info_.back().second;
		row_data.insert(std::make_pair("unit_type", column));

		list.add_row(row_data);

		// Select the previous choice, if any.
		if(choice_.empty() != true && choice_ == i.first) {
			list.select_row(list.get_item_count() - 1);
		}
	}

	if(type_info_.empty()) {
		ERR_GUI_G << "no unit types found for unit create dialog; not good"
				  << std::endl;
	}

	std::vector<tgenerator_::torder_func> order_funcs(2);
	order_funcs[0] = boost::bind(&tunit_create::compare_race, this, _1, _2);
	order_funcs[1] = boost::bind(&tunit_create::compare_race_rev, this, _1, _2);
	list.set_column_order(0, order_funcs);
	order_funcs[0] = boost::bind(&tunit_create::compare_type, this, _1, _2);
	order_funcs[1] = boost::bind(&tunit_create::compare_type_rev, this, _1, _2);
	list.set_column_order(1, order_funcs);
}

bool tunit_create::compare_type(unsigned i1, unsigned i2) const
{
	return type_info_[i1].second < type_info_[i2].second;
}

bool tunit_create::compare_race(unsigned i1, unsigned i2) const
{
	return type_info_[i1].first < type_info_[i2].first;
}

bool tunit_create::compare_type_rev(unsigned i1, unsigned i2) const
{
	return type_info_[i1].second > type_info_[i2].second;
}

bool tunit_create::compare_race_rev(unsigned i1, unsigned i2) const
{
	return type_info_[i1].first > type_info_[i2].first;
}

void tunit_create::post_show(twindow& window)
{
	ttoggle_button& female_toggle
			= find_widget<ttoggle_button>(&window, "female_toggle", false);
	tlistbox& list = find_widget<tlistbox>(&window, "unit_type_list", false);

	choice_ = "";

	if(get_retval() != twindow::OK) {
		return;
	}

	const int selected_row = list.get_selected_row();
	if(selected_row < 0) {
		return;
	} else if(static_cast<size_t>(selected_row) >= type_info_.size()) {
		// FIXME: maybe assert?
		ERR_GUI_G << "unit create dialog has more list items than known unit "
					 "types; not good\n";
		return;
	}

	last_chosen_type_id = choice_
			= type_info_[static_cast<size_t>(selected_row)].second;
	last_gender = gender_ = female_toggle.get_value() ? unit_race::FEMALE
													  : unit_race::MALE;
}

void tunit_create::gender_toggle_callback(twindow& window)
{
	ttoggle_button& male_toggle
			= find_widget<ttoggle_button>(&window, "male_toggle", false);
	ttoggle_button& female_toggle
			= find_widget<ttoggle_button>(&window, "female_toggle", false);

	// TODO Ye olde ugly hack for the lack of radio buttons.

	if(gender_ == unit_race::MALE) {
		gender_ = female_toggle.get_value() ? unit_race::FEMALE
											: unit_race::MALE;
	} else {
		gender_ = male_toggle.get_value() ? unit_race::MALE : unit_race::FEMALE;
	}

	update_male_female_toggles(male_toggle, female_toggle, gender_);
}
}
