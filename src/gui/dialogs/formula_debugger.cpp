/* $Id$ */
/*
   Copyright (C) 2009 - 2011 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/formula_debugger.hpp"

#include "gui/dialogs/helper.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "../../foreach.hpp"
#include "../../formula_debugger.hpp"

#include <boost/bind.hpp>

namespace gui2 {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_formula_debugger
 *
 * == Formula debugger ==
 *
 * This shows the debugger for the formulas.
 *
 * @start_table = grid
 *     (stack) (control) ()       A stack.
 *     (execution) (control) ()   Execution trace label.
 *     (state) (control) ()       The state.
 *
 *     (step) (button) ()         Button to step into the execution.
 *     (stepout) (button) ()      Button to step out of the execution.
 *     (next) (button) ()         Button to execute the next statement.
 *     (continue) (button) ()     Button to continue the execution.
 * @end_table
 */

twindow* tformula_debugger::build_window(CVideo& video)
{
	return build(video, get_id(FORMULA_DEBUGGER));
}

void tformula_debugger::pre_show(CVideo& /*video*/, twindow& window)
{
	// stack label
	tcontrol* stack_label = find_widget<tcontrol>(
			&window, "stack", false, true);

	std::stringstream stack_text;
	std::string indent = "  ";
	int c = 0;
	BOOST_FOREACH (const game_logic::debug_info &i, fdb_.get_call_stack()) {
		for (int d=0; d<c; d++) {
			stack_text << indent;
		}
		stack_text << "#<span color=\"green\">" << i.counter()
				<<"</span>: \"<span color=\"green\">"<< i.name()
				<< "</span>\": '" << i.str() << "' " << std::endl;
		c++;
	}

	stack_label->set_use_markup(true);
	stack_label->set_label(stack_text.str());
	window.keyboard_capture(stack_label);

	// execution trace label
	tcontrol* execution_label = find_widget<tcontrol>(
			&window, "execution", false, true);

	std::stringstream execution_text;
	BOOST_FOREACH (const game_logic::debug_info &i, fdb_.get_execution_trace()) {
		for (int d=0; d<i.level(); d++) {
			execution_text << indent;
		}
		if (!i.evaluated() ) {
			execution_text << "#<span color=\"green\">" << i.counter()
					<< "</span>: \"<span color=\"green\">" << i.name()
					<< "</span>\": '" << i.str() << "' " << std::endl;
		} else {
			execution_text << "#<span color=\"yellow\">" << i.counter()
					<< "</span>: \"<span color=\"yellow\">" << i.name()
					<< "</span>\": '" << i.str() << "' = "
					<< "<span color=\"red\">"
					<< i.value().to_debug_string(NULL,false)
					<<"</span>" << std::endl;
		}
	}

	execution_label->set_use_markup(true);
	execution_label->set_label(execution_text.str());

	// state
	std::string state_str;
	bool is_end = false;
	if (!fdb_.get_current_breakpoint()) {
		state_str = "";
	} else {
		state_str = fdb_.get_current_breakpoint()->name();
	        if (state_str=="End") {
			is_end = true;
		}
	}

	find_widget<tcontrol>(&window, "state", false).set_label(state_str);

	// callbacks
	tbutton& step_button = find_widget<tbutton>(&window, "step", false);

	step_button.set_callback_mouse_left_click(dialog_callback<
			  tformula_debugger
			, &tformula_debugger::callback_step_button>);


	tbutton& stepout_button = find_widget<tbutton>(&window, "stepout", false);

	stepout_button.set_callback_mouse_left_click(dialog_callback<
			  tformula_debugger
			, &tformula_debugger::callback_stepout_button>);


	tbutton& next_button = find_widget<tbutton>(&window, "next", false);

	next_button.set_callback_mouse_left_click(dialog_callback<
			  tformula_debugger
			, &tformula_debugger::callback_next_button>);


	tbutton& continue_button = find_widget<tbutton>(&window, "continue", false);

	continue_button.set_callback_mouse_left_click(dialog_callback<
			  tformula_debugger
			, &tformula_debugger::callback_continue_button>);


	if (is_end) {
		step_button.set_active(false);
		stepout_button.set_active(false);
		next_button.set_active(false);
		continue_button.set_active(false);
	}
}


void tformula_debugger::callback_continue_button(twindow& window)
{
	fdb_.add_breakpoint_continue_to_end();
	window.set_retval(twindow::OK);
}

void tformula_debugger::callback_next_button(twindow& window)
{
	fdb_.add_breakpoint_next();
	window.set_retval(twindow::OK);
}

void tformula_debugger::callback_step_button(twindow& window)
{
	fdb_.add_breakpoint_step_into();
	window.set_retval(twindow::OK);
}

void tformula_debugger::callback_stepout_button(twindow& window)
{
	fdb_.add_breakpoint_step_out();
	window.set_retval(twindow::OK);
}

} //end of namespace gui2
