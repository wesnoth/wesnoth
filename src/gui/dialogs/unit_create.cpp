/* $Id$ */
/*
   Copyright (C) 2009 by Ignacio R. Morelle <shadowm2006@gmail.com>
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
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"

namespace gui2 {

/* TODO: wiki-doc me! */

size_t tunit_create::no_choice() const
{
	return std::max(races_.size(), types_.size());
}

std::vector<std::string>::size_type tunit_create::list_size() const
{
	return std::min(races_.size(), types_.size());
}

void tunit_create::set_list_choice(size_t choice)
{
	choice_ = std::min(choice, list_size() - 1);
}

twindow* tunit_create::build_window(CVideo& video)
{
	return build(video, get_id(UNIT_CREATE));
}

void tunit_create::pre_show(CVideo& /*video*/, twindow& window)
{
	ttoggle_button& male_toggle = NEW_find_widget<ttoggle_button>(
			&window, "male_toggle", false);
	ttoggle_button& female_toggle = NEW_find_widget<ttoggle_button>(
			&window, "female_toggle", false);
	ttoggle_button& namegen_toggle = NEW_find_widget<ttoggle_button>(
			&window, "namegen_toggle", false);
	tlistbox& list = NEW_find_widget<tlistbox>(
			&window, "unit_type_list", false);

	if(gender_ == unit_race::FEMALE) {
		female_toggle.set_value(true);
		male_toggle.set_value(false);
	}
	else {
		female_toggle.set_value(false);
		male_toggle.set_value(true);
	}

	namegen_toggle.set_value(generate_name_);

	if(types_.empty() != true) {
		// TODO: check at setter time instead/merge setters in one?
		if(races_.size() != types_.size()) {
			WRN_GUI_G << "tunit_create::pre_show(): more unit "
					"races than types, using minimum set\n";
		}

		for(std::vector<std::string>::size_type k = 0; k < list_size(); ++k) {
			std::map<std::string, string_map> data;
			string_map item;

			item["label"] = types_[k];
			data.insert(std::make_pair("unit_type", item));
			item["label"] = races_[k];
			data.insert(std::make_pair("race", item));

			list.add_row(data);
		}
	}
}

void tunit_create::post_show(twindow& window)
{
	if(get_retval() != twindow::OK) {
		choice_ = no_choice();
		return;
	}

	tlistbox& list = NEW_find_widget<tlistbox>(
			&window, "unit_type_list", false);

	if(list.get_selected_row() < 0) {
		choice_ = no_choice();
		return;
	}

	ttoggle_button& female_toggle = NEW_find_widget<ttoggle_button>(
			&window, "female_toggle", false);
	ttoggle_button& namegen_toggle = NEW_find_widget<ttoggle_button>(
			&window, "namegen_toggle", false);

	choice_ = static_cast<size_t>(list.get_selected_row());
	gender_ = female_toggle.get_value() ? unit_race::FEMALE : unit_race::MALE;
	generate_name_ = namegen_toggle.get_value();
}

}
