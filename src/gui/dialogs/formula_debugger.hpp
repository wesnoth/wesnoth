/*
	Copyright (C) 2009 - 2024
	by Yurii Chernyi <terraninfo@terraninfo.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

namespace gui2::dialogs
{

/**
 * @ingroup GUIWindowDefinitionWML
 *
 * This shows the debugger for the formulas.
 * Key               |Type          |Mandatory|Description
 * ------------------|--------------|---------|-----------
 * stack             | control      |yes      |A stack.
 * execution         | control      |yes      |Execution trace label.
 * state             | control      |yes      |The state.
 * step              | @ref button  |yes      |Button to step into the execution.
 * stepout           | @ref button  |yes      |Button to step out of the execution.
 * next              | @ref button  |yes      |Button to execute the next statement.
 * continue          | @ref button  |yes      |Button to continue the execution.
 */
class formula_debugger : public modal_dialog
{
public:
	explicit formula_debugger(wfl::formula_debugger& fdb)
		: modal_dialog(window_id()) , fdb_(fdb)
	{
	}

	DEFINE_SIMPLE_DISPLAY_WRAPPER(formula_debugger)

private:
	virtual void pre_show(window& window) override;

	virtual const std::string& window_id() const override;

	/***** ***** button callbacks ***** *****/
	void callback_continue_button();

	void callback_next_button();

	void callback_step_button();

	void callback_stepout_button();

	wfl::formula_debugger& fdb_;
};

} // namespace dialogs
