/* $Id$ */
/*
   Copyright (C) 2009 - 2011 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/unit_create.hpp"

#include "gui/auxiliary/log.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"
#include "foreach.hpp"
#include "unit_types.hpp"

namespace {
	static std::string last_chosen_type_id = "";
	static bool last_generate_names_value = true;
	static unit_race::GENDER last_gender = unit_race::MALE;

	/**
	 * Helper function for updating the male/female checkboxes.
	 * It's not a private member of class gui2::tunit_create so
	 * we don't have to expose a forward-declaration of ttoggle_button
	 * in the interface.
	 */
	void update_male_female_toggles(gui2::ttoggle_button& male, gui2::ttoggle_button& female, unit_race::GENDER choice)
	{
		male.set_value(choice == unit_race::MALE);
		female.set_value(choice == unit_race::FEMALE);
	}
}

namespace gui2 {

/* TODO: wiki-doc me! */

tunit_create::tunit_create()
	: gender_(last_gender)
	, generate_name_(last_generate_names_value)
	, choice_(last_chosen_type_id)
	, type_ids_()
{
}

twindow* tunit_create::build_window(CVideo& video)
{
	return build(video, get_id(UNIT_CREATE));
}

void tunit_create::pre_show(CVideo& /*video*/, twindow& window)
{
	ttoggle_button& male_toggle = find_widget<ttoggle_button>(
			&window, "male_toggle", false);
	ttoggle_button& female_toggle = find_widget<ttoggle_button>(
			&window, "female_toggle", false);
	ttoggle_button& namegen_toggle = find_widget<ttoggle_button>(
			&window, "namegen_toggle", false);
	tlistbox& list = find_widget<tlistbox>(&window, "unit_type_list", false);

	male_toggle.set_callback_state_change(
		dialog_callback<tunit_create, &tunit_create::gender_toggle_callback>
	);
	female_toggle.set_callback_state_change(
		dialog_callback<tunit_create, &tunit_create::gender_toggle_callback>
	);
	update_male_female_toggles(male_toggle, female_toggle, gender_);
	namegen_toggle.set_value(generate_name_);
	list.clear();

	// We use this container to "map" unit_type ids to list subscripts
	// later, so it ought to be empty before proceeding.
	type_ids_.clear();

	std::vector< std::string > type_labels, race_labels;

	BOOST_FOREACH (const unit_type_data::unit_type_map::value_type &i, unit_types.types())
	{
		unit_types.find(i.first, unit_type::HELP_INDEX);

		// And so we map an unit_type id to a list subscript. Ugh.
		type_ids_.push_back(i.first);

		std::string race_label;

		if (const unit_race *r = unit_types.find_race(i.second.race())) {
			race_label = r->plural_name();
		}

		std::map< std::string, string_map > row_data;
		string_map column;

		column["label"] = i.second.type_name();
		row_data.insert(std::make_pair("unit_type", column));
		column["label"] = race_label;
		row_data.insert(std::make_pair("race", column));

		list.add_row(row_data);

		// Select the previous choice, if any.
		if(choice_.empty() != true && choice_ == i.first) {
			list.select_row(list.get_item_count() - 1);
		}
	}

	if(type_ids_.empty()) {
		ERR_GUI_G << "no unit types found for unit create dialog; not good\n";
	}
}

void tunit_create::post_show(twindow& window)
{
	ttoggle_button& female_toggle = find_widget<ttoggle_button>(
			&window, "female_toggle", false);
	ttoggle_button& namegen_toggle = find_widget<ttoggle_button>(
			&window, "namegen_toggle", false);
	tlistbox& list = find_widget<tlistbox>(&window, "unit_type_list", false);

	choice_ = "";

	if(get_retval() != twindow::OK) {
		return;
	}

	const int selected_row = list.get_selected_row();
	if(selected_row < 0) {
		return;
	}
	else if(static_cast<size_t>(selected_row) >= type_ids_.size()) {
		// FIXME: maybe assert?
		ERR_GUI_G << "unit create dialog has more list items than known unit types; not good\n";
		return;
	}

	last_chosen_type_id = choice_ =
		type_ids_[static_cast<size_t>(selected_row)];
	last_gender = gender_ =
		female_toggle.get_value() ? unit_race::FEMALE : unit_race::MALE;
	last_generate_names_value = generate_name_ =
		namegen_toggle.get_value();
}

void tunit_create::gender_toggle_callback(twindow& window)
{
	ttoggle_button& male_toggle = find_widget<ttoggle_button>(
			&window, "male_toggle", false);
	ttoggle_button& female_toggle = find_widget<ttoggle_button>(
			&window, "female_toggle", false);

	// Ye olde ugly hack for the lack of radio buttons.

	if(gender_ == unit_race::MALE) {
		gender_ = female_toggle.get_value() ? unit_race::FEMALE : unit_race::MALE;
	}
	else {
		gender_ = male_toggle.get_value() ? unit_race::MALE : unit_race::FEMALE;
	}

	update_male_female_toggles(male_toggle, female_toggle, gender_);
}

}
