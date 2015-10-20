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

#ifndef GUI_DIALOGS_UNIT_CREATE_HPP_INCLUDED
#define GUI_DIALOGS_UNIT_CREATE_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "gui/widgets/text.hpp"
#include "race.hpp"
#include "unit_types.hpp"

#include <string>
#include <vector>

class display;

namespace gui2
{

class tunit_create : public tdialog
{
public:
	tunit_create(display* disp = NULL);

	/** Unit type choice from the user. */
	const std::string& choice() const
	{
		return choice_;
	}

	/** Whether the user actually chose a unit type or not. */
	bool no_choice() const
	{
		return choice_.empty();
	}

	/** Gender choice from the user. */
	unit_race::GENDER gender()
	{
		return gender_;
	}

private:
	std::vector<const unit_type*> units_;

	unit_race::GENDER gender_;

	std::string choice_;

	std::vector<std::string> last_words_;

	display* disp_;

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	bool compare_type(unsigned i1, unsigned i2) const;
	bool compare_race(unsigned i1, unsigned i2) const;
	bool compare_type_rev(unsigned i1, unsigned i2) const;
	bool compare_race_rev(unsigned i1, unsigned i2) const;

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	void print_stats(std::stringstream& str, const int row);

	/** Callbacks */
	void list_item_clicked(twindow& window);
	bool filter_text_changed(ttext_* textbox, const std::string& text);
	void profile_button_callback(twindow& window);
	void gender_toggle_callback(twindow& window);
};
}

#endif /* ! GUI_DIALOGS_UNIT_CREATE_HPP_INCLUDED */
