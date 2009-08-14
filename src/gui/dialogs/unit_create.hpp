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

namespace gui2 {

class tunit_create : public tdialog
{
public:
	tunit_create(unit_race::GENDER default_gender = unit_race::MALE) :
		gender_(default_gender)
	{}

	unit_race::GENDER gender() const { return gender_; }
	void set_gender(unit_race::GENDER gender) { gender_ = gender; }

private:
	unit_race::GENDER gender_;

	/** Inherited from tdialog. */
	twindow* build_window(CVideo& video);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);
};

}

#endif /* ! GUI_DIALOGS_UNIT_CREATE_HPP_INCLUDED */
