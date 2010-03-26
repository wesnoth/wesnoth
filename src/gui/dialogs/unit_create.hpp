/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Ignacio R. Morelle <shadowm2006@gmail.com>
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
	tunit_create();

	/** Unit type choice from the user. */
	const std::string& choice() const { return choice_; }

	/** Whether the user actually chose a unit type or not. */
	bool no_choice() const { return choice_.empty(); }

	/** User's choice whether to create a unit with a random name. */
	bool generate_name() const { return generate_name_; }

	/** Gender choice from the user. */
	unit_race::GENDER gender() { return gender_; }

private:
	unit_race::GENDER        gender_;
	bool                     generate_name_;

	std::string              choice_;
	std::vector<std::string> type_ids_;

	/** Inherited from tdialog, implemented by REGISTER_WINDOW. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	void gender_toggle_callback(twindow& window);
};

}

#endif /* ! GUI_DIALOGS_UNIT_CREATE_HPP_INCLUDED */
