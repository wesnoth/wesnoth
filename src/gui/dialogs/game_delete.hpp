/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_DELETE_GAME_HPP_INCLUDED
#define GUI_DIALOGS_DELETE_GAME_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

namespace gui2 {

class tgame_delete
	: public tdialog
{
public:
	tgame_delete();

	bool dont_ask_again() const { return dont_ask_again_; }

private:

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	/** Inherited from tdialog, implemented by REGISTER_WINDOW. */
	virtual const std::string& window_id() const;

	tfield_bool* chk_dont_ask_again_;
	bool dont_ask_again_;
};

}

#endif

