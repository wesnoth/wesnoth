/*
   Copyright (C) 2009 - 2016 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_FORMULA_DEBUGGER_HPP_INCLUDED
#define GUI_DIALOGS_FORMULA_DEBUGGER_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

namespace game_logic
{
class formula_debugger;
}

namespace gui2
{

class tformula_debugger : public tdialog
{
public:
	explicit tformula_debugger(game_logic::formula_debugger& fdb) : fdb_(fdb)
	{
	}

private:
	/** Inherited from tdialog. */
	void pre_show(twindow& window);

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/***** ***** button callbacks ***** *****/
	void callback_continue_button(twindow& window);

	void callback_next_button(twindow& window);

	void callback_step_button(twindow& window);

	void callback_stepout_button(twindow& window);

	game_logic::formula_debugger& fdb_;
};

} // namespace gui2

#endif /* ! GUI_DIALOGS_FORMULA_DEBUGGER_HPP_INCLUDED */
