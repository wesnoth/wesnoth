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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/formula_debugger.hpp"

#include "gui/widgets/button.hpp"
#include "gui/widgets/window.hpp"
#include "../../foreach.hpp"
#include "../../formula_debugger.hpp"

#include <boost/bind.hpp>

namespace gui2 {

//ugly hack, should be removed as soon as boost::bind works with tbutton callbacks
static tformula_debugger *td;



namespace {

void formula_debugger_callback_continue_function(gui2::twidget *caller)
{
	if (td==NULL) {
		return;
	}
	td->callback_continue_button(caller);
}

void formula_debugger_callback_next_function(gui2::twidget *caller)
{
	if (td==NULL) {
		return;
	}
	td->callback_next_button(caller);
}

void formula_debugger_callback_step_function(gui2::twidget *caller)
{
	if (td==NULL) {
		return;
	}
	td->callback_step_button(caller);
}

void formula_debugger_callback_stepout_function(gui2::twidget *caller)
{
	if (td==NULL) {
		return;
	}
	td->callback_stepout_button(caller);
}

} //of namespace {


twindow* tformula_debugger::build_window(CVideo& video)
{
	return build(video, get_id(FORMULA_DEBUGGER));
}

void tformula_debugger::pre_show(CVideo& /*video*/, twindow& window)
{
	//hack
	td = this;

	// stack label
	tcontrol* stack_label =
                dynamic_cast<tcontrol*>(window.find_widget("stack", false));
        VALIDATE(stack_label, missing_widget("stack"));

	std::stringstream stack_text;
	std::string indent = "  ";
	int c = 0;
	foreach (const game_logic::debug_info &i, fdb_.get_call_stack()) {
		for (int d=0; d<c; d++) {
			stack_text << indent;
		}
		stack_text << "#<span color=\"green\">" << i.counter() <<"</span>: \"<span color=\"green\">"<< i.name() << "</span>\": '" << i.str() << "' " << std::endl;
		c++;
	}

	stack_label->set_markup_mode(tcontrol::PANGO_MARKUP);
       	stack_label->set_label(stack_text.str());
        window.keyboard_capture(stack_label);

	// execution trace label
	
	tcontrol* execution_label =
                dynamic_cast<tcontrol*>(window.find_widget("execution", false));
        VALIDATE(execution_label, missing_widget("execution"));

	std::stringstream execution_text;
	foreach (const game_logic::debug_info &i, fdb_.get_execution_trace()) {
		for (int d=0; d<i.level(); d++) {
			execution_text << indent;
		}
		if (!i.evaluated() ) {
			execution_text << "#<span color=\"green\">" << i.counter() <<"</span>: \"<span color=\"green\">"<< i.name() << "</span>\": '" << i.str() << "' " << std::endl;
		} else {
			execution_text << "#<span color=\"yellow\">" << i.counter() <<"</span>: \"<span color=\"yellow\">"<< i.name() << "</span>\": '" << i.str() << "' = " << "<span color=\"red\">"<< i.value().to_debug_string(NULL,true) <<"</span>" << std::endl;
		}
	}

	execution_label->set_markup_mode(tcontrol::PANGO_MARKUP);
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

	tcontrol* state_label =
                dynamic_cast<tcontrol*>(window.find_widget("state", false));
        VALIDATE(state_label, missing_widget("state"));
       	state_label->set_label(state_str);

	// callbacks

	tbutton* step_button =
                dynamic_cast<tbutton*>(window.find_widget("step", false));
        VALIDATE(step_button, missing_widget("step"));

        step_button->set_callback_mouse_left_click(formula_debugger_callback_step_function);
	
	tbutton* stepout_button =
                dynamic_cast<tbutton*>(window.find_widget("stepout", false));
        VALIDATE(stepout_button, missing_widget("stepout"));
        stepout_button->set_callback_mouse_left_click(formula_debugger_callback_stepout_function);

	tbutton* next_button =
                dynamic_cast<tbutton*>(window.find_widget("next", false));
        VALIDATE(next_button, missing_widget("next"));
        next_button->set_callback_mouse_left_click(formula_debugger_callback_next_function);

	tbutton* continue_button =
                dynamic_cast<tbutton*>(window.find_widget("continue", false));
        VALIDATE(continue_button, missing_widget("continue"));
        continue_button->set_callback_mouse_left_click(formula_debugger_callback_continue_function);


	if (is_end) {
		step_button->set_active(false);
		stepout_button->set_active(false);
		next_button->set_active(false);
		continue_button->set_active(false);
	}
	
}


void tformula_debugger::callback_continue_button(gui2::twidget* caller)
{
	fdb_.add_breakpoint_continue_to_end();
	caller->get_window()->set_retval(twindow::OK);
}

void tformula_debugger::callback_next_button(gui2::twidget* caller)
{
	fdb_.add_breakpoint_next();
	caller->get_window()->set_retval(twindow::OK);
}

void tformula_debugger::callback_step_button(gui2::twidget* caller)
{
	fdb_.add_breakpoint_step_into();
	caller->get_window()->set_retval(twindow::OK);
}

void tformula_debugger::callback_stepout_button(gui2::twidget* caller)
{
	fdb_.add_breakpoint_step_out();
	caller->get_window()->set_retval(twindow::OK);
}

} //end of namespace gui2
