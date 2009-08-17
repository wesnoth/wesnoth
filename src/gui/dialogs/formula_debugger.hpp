/* $Id$ */
/*
   Copyright (C) 2009 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_FORMULA_DEBUGGER_HPP_INCLUDED
#define GUI_DIALOGS_FORMULA_DEBUGGER_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

namespace game_logic {
	class formula_debugger;
}

namespace gui2 {

class tformula_debugger : public tdialog
{
public:
	tformula_debugger(game_logic::formula_debugger &fdb) :
		fdb_(fdb)
	{}


	/** Inherited from tdialog. */
	twindow* build_window(CVideo& video);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);


	void callback_continue_button(gui2::twidget *caller);


	void callback_next_button(gui2::twidget *caller);


	void callback_step_button(gui2::twidget *caller);


	void callback_stepout_button(gui2::twidget *caller);

private:
	game_logic::formula_debugger &fdb_;
};

}

#endif /* ! GUI_DIALOGS_FORMULA_DEBUGGER_HPP_INCLUDED */
