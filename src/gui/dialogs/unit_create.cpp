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

#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"

namespace gui2 {

/* TODO: wiki-doc me! */

twindow* tunit_create::build_window(CVideo& video)
{
	return build(video, get_id(UNIT_CREATE));
}

void tunit_create::pre_show(CVideo& /*video*/, twindow& window)
{
	ttoggle_button* male_toggle = dynamic_cast<ttoggle_button*>(window.find_widget("male_toggle", false));
	ttoggle_button* female_toggle = dynamic_cast<ttoggle_button*>(window.find_widget("female_toggle", false));

	VALIDATE(male_toggle, missing_widget("male_toggle"));
	VALIDATE(female_toggle, missing_widget("female_toggle"));

	if(gender_ == unit_race::FEMALE) {
		female_toggle->set_value(true);
		male_toggle->set_value(false);
	}
	else {
		female_toggle->set_value(false);
		male_toggle->set_value(true);
	}
}

}
