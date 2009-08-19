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

#ifndef GUI_DIALOGS_UNIT_CREATE_HPP_INCLUDED
#define GUI_DIALOGS_UNIT_CREATE_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "race.hpp"

#include <string>
#include <vector>

namespace gui2 {

class tunit_create : public tdialog
{
public:
	tunit_create() :
		races_(),
		types_(),
		gender_(unit_race::MALE),
		generate_name_(false),
		choice_(0)
	{}

	unit_race::GENDER gender() const { return gender_; }
	void set_gender(unit_race::GENDER gender) { gender_ = gender; }

	bool generate_name() const { return generate_name_; }
	void set_generate_name(bool generate_name) { generate_name_ = generate_name; }

	/** Inserts a new race/unit-type pair into the list. */
	void add_race_type_pair(const std::string& race, const std::string& type) {
		races_.push_back(race);
		types_.push_back(type);
	}

	/** Unit type choice from the user. */
	size_t list_choice() const { return choice_; }
	void set_list_choice(size_t choice);

	/** Value used to indicate that the user did not choice an unit type. */
	size_t no_choice() const;

private:
	std::vector<std::string> races_;
	std::vector<std::string> types_;
	unit_race::GENDER gender_;
	bool              generate_name_;
	size_t            choice_;

	/** Inherited from tdialog. */
	twindow* build_window(CVideo& video);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	std::vector<std::string>::size_type list_size() const;
};

}

#endif /* ! GUI_DIALOGS_UNIT_CREATE_HPP_INCLUDED */
