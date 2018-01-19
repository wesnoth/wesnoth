/*
   Copyright (C) 2009 - 2018 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gui/dialogs/modal_dialog.hpp"

namespace wfl
{
class formula_debugger;
}

namespace gui2
{
namespace dialogs
{

class formula_debugger : public modal_dialog
{
public:
	explicit formula_debugger(wfl::formula_debugger& fdb) : fdb_(fdb)
	{
	}

private:
	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/***** ***** button callbacks ***** *****/
	void callback_continue_button(window& window);

	void callback_next_button(window& window);

	void callback_step_button(window& window);

	void callback_stepout_button(window& window);

	wfl::formula_debugger& fdb_;
};

} // namespace dialogs
} // namespace gui2
