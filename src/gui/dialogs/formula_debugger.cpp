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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/formula_debugger.hpp"

#include "gui/widgets/button.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/window.hpp"
#include "formula/debugger.hpp"
#include "font/pango/escape.hpp"
#include "serialization/markup.hpp"

#include <functional>

namespace gui2::dialogs
{

REGISTER_DIALOG(formula_debugger)

void formula_debugger::pre_show()
{
	// stack label
	scroll_label* stack_label
			= find_widget<scroll_label>("stack", false, true);

	std::stringstream stack_text;
	std::string indent = "  ";
	int c = 0;
	for(const auto & i : fdb_.get_call_stack())
	{
		for(int d = 0; d < c; ++d) {
			stack_text << indent;
		}
		stack_text << "#" << markup::span_color("#00ff00", i.counter())
				   << ": \"" << markup::span_color("#00ff00", font::escape_text(i.name()))
				   << "\": (" << font::escape_text(i.str()) << ") " << std::endl;
		++c;
	}

	stack_label->set_use_markup(true);
	stack_label->set_label(stack_text.str());
	stack_label->scroll_vertical_scrollbar(scrollbar_base::END);
	keyboard_capture(stack_label);

	// execution trace label
	scroll_label* execution_label = find_widget<scroll_label>("execution", false, true);

	std::stringstream execution_text;
	for(const auto & i : fdb_.get_execution_trace())
	{
		for(int d = 0; d < i.level(); ++d) {
			execution_text << indent;
		}
		if(!i.evaluated()) {
			execution_text << "#" << markup::span_color("#00ff00", i.counter())
				   		   << ": \"" << markup::span_color("#00ff00", font::escape_text(i.name()))
				   		   << "\": (" << font::escape_text(i.str()) << ") " << std::endl;
		} else {
			execution_text << "#" << markup::span_color("#ffff00", i.counter())
				   		   << ": \"" << markup::span_color("#ffff00", font::escape_text(i.name()))
				   		   << "\": (" << font::escape_text(i.str()) << ") ="
						   << markup::span_color("#ffa500", font::escape_text(i.value().to_debug_string()))
						   << std::endl;
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

	find_widget<styled_widget>("state").set_label(state_str);

	// callbacks
	button& step_button = find_widget<button>("step");
	connect_signal_mouse_left_click(
			step_button,
			std::bind(&formula_debugger::callback_step_button, this));

	button& stepout_button = find_widget<button>("stepout");
	connect_signal_mouse_left_click(
			stepout_button,
			std::bind(&formula_debugger::callback_stepout_button, this));

	button& next_button = find_widget<button>("next");
	connect_signal_mouse_left_click(
			next_button,
			std::bind(&formula_debugger::callback_next_button, this));

	button& continue_button = find_widget<button>("continue");
	connect_signal_mouse_left_click(
			continue_button,
			std::bind(&formula_debugger::callback_continue_button, this));

	if(is_end) {
		step_button.set_active(false);
		stepout_button.set_active(false);
		next_button.set_active(false);
		continue_button.set_active(false);
	}
}

void formula_debugger::callback_continue_button()
{
	fdb_.add_breakpoint_continue_to_end();
	set_retval(retval::OK);
}

void formula_debugger::callback_next_button()
{
	fdb_.add_breakpoint_next();
	set_retval(retval::OK);
}

void formula_debugger::callback_step_button()
{
	fdb_.add_breakpoint_step_into();
	set_retval(retval::OK);
}

void formula_debugger::callback_stepout_button()
{
	fdb_.add_breakpoint_step_out();
	set_retval(retval::OK);
}

} // namespace dialogs
