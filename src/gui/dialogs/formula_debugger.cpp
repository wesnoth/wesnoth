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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/formula_debugger.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "formula/debugger.hpp"
#include "font/pango/escape.hpp"

#include "utils/functional.hpp"

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_formula_debugger
 *
 * == Formula debugger ==
 *
 * This shows the debugger for the formulas.
 *
 * @begin{table}{dialog_widgets}
 *
 * stack & & styled_widget & m &
 *         A stack. $
 *
 * execution & & styled_widget & m &
 *         Execution trace label. $
 *
 * state & & styled_widget & m &
 *         The state. $
 *
 * step & & button & m &
 *         Button to step into the execution. $
 *
 * stepout & & button & m &
 *         Button to step out of the execution. $
 *
 * next & & button & m &
 *         Button to execute the next statement. $
 *
 * continue & & button & m &
 *         Button to continue the execution. $
 *
 * @end{table}
 */

REGISTER_DIALOG(formula_debugger)

void formula_debugger::pre_show(window& window)
{
	// stack label
	scroll_label* stack_label
			= find_widget<scroll_label>(&window, "stack", false, true);

	std::stringstream stack_text;
	std::string indent = "  ";
	int c = 0;
	for(const auto & i : fdb_.get_call_stack())
	{
		for(int d = 0; d < c; ++d) {
			stack_text << indent;
		}
		stack_text << "#<span color=\"green\">" << i.counter()
				   << "</span>: \"<span color=\"green\">" << font::escape_text(i.name())
				   << "</span>\": (" << font::escape_text(i.str()) << ") " << std::endl;
		++c;
	}

	stack_label->set_use_markup(true);
	stack_label->set_label(stack_text.str());
	stack_label->scroll_vertical_scrollbar(scrollbar_base::END);
	window.keyboard_capture(stack_label);

	// execution trace label
	scroll_label* execution_label
			= find_widget<scroll_label>(&window, "execution", false, true);

	std::stringstream execution_text;
	for(const auto & i : fdb_.get_execution_trace())
	{
		for(int d = 0; d < i.level(); ++d) {
			execution_text << indent;
		}
		if(!i.evaluated()) {
			execution_text << "#<span color=\"green\">" << i.counter()
						   << "</span>: \"<span color=\"green\">" << font::escape_text(i.name())
						   << "</span>\": (" << font::escape_text(i.str()) << ") " << std::endl;
		} else {
			execution_text << "#<span color=\"yellow\">" << i.counter()
						   << "</span>: \"<span color=\"yellow\">" << font::escape_text(i.name())
						   << "</span>\": (" << font::escape_text(i.str()) << ") = "
						   << "<span color=\"orange\">"
						   << font::escape_text(i.value().to_debug_string())
						   << "</span>" << std::endl;
		}
	}

	execution_label->set_use_markup(true);
	execution_label->set_label(execution_text.str());
	execution_label->scroll_vertical_scrollbar(scrollbar_base::END);
	// state
	std::string state_str;
	bool is_end = false;
	if(!fdb_.get_current_breakpoint()) {
		state_str = "";
	} else {
		state_str = fdb_.get_current_breakpoint()->name();
		if(state_str == "End") {
			is_end = true;
		}
	}

	find_widget<styled_widget>(&window, "state", false).set_label(state_str);

	// callbacks
	button& step_button = find_widget<button>(&window, "step", false);
	connect_signal_mouse_left_click(
			step_button,
			std::bind(&formula_debugger::callback_step_button,
						this,
						std::ref(window)));

	button& stepout_button = find_widget<button>(&window, "stepout", false);
	connect_signal_mouse_left_click(
			stepout_button,
			std::bind(&formula_debugger::callback_stepout_button,
						this,
						std::ref(window)));

	button& next_button = find_widget<button>(&window, "next", false);
	connect_signal_mouse_left_click(
			next_button,
			std::bind(&formula_debugger::callback_next_button,
						this,
						std::ref(window)));

	button& continue_button = find_widget<button>(&window, "continue", false);
	connect_signal_mouse_left_click(
			continue_button,
			std::bind(&formula_debugger::callback_continue_button,
						this,
						std::ref(window)));

	if(is_end) {
		step_button.set_active(false);
		stepout_button.set_active(false);
		next_button.set_active(false);
		continue_button.set_active(false);
	}
}

void formula_debugger::callback_continue_button(window& window)
{
	fdb_.add_breakpoint_continue_to_end();
	window.set_retval(window::OK);
}

void formula_debugger::callback_next_button(window& window)
{
	fdb_.add_breakpoint_next();
	window.set_retval(window::OK);
}

void formula_debugger::callback_step_button(window& window)
{
	fdb_.add_breakpoint_step_into();
	window.set_retval(window::OK);
}

void formula_debugger::callback_stepout_button(window& window)
{
	fdb_.add_breakpoint_step_out();
	window.set_retval(window::OK);
}

} // namespace dialogs
} // namespace gui2
