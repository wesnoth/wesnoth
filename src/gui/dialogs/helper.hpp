/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_HELPER_HPP_INCLUDED
#define GUI_DIALOGS_HELPER_HPP_INCLUDED

#include "gui/widgets/window.hpp"

namespace gui2 {

/**
 * Template for dialog callbacks. Example usage:
 * widget->set_callback(dialog_callback<my_dialog_class, &my_dialog_class::member_function>);
 */
template <class D, void (D::*fptr)(twindow&)>
void dialog_callback(twidget* caller)
{
	D* dialog = dynamic_cast<D*>(caller->dialog());
	assert(dialog);
	twindow* window = dynamic_cast<twindow*>(caller->get_window());
	assert(window);
	(dialog->*fptr)(*window);
}

/**
 * Macro (ugh) for setting standardised dialog callbacks, a button with id "foo"
 * will have it's callback registered to MY_TYPE::foo_button_callback. Designed
 * to be used within a dialog's pre_show() (requires a twindow& window in the
 * current scope).
 */
#define GUI2_EASY_BUTTON_CALLBACK(ID, MY_TYPE) \
	tbutton* ID##_btn = dynamic_cast<tbutton*>(window.find(#ID, false)); \
	VALIDATE(ID##_btn, missing_widget(#ID)); \
	ID##_btn->set_callback_mouse_left_click(dialog_callback<MY_TYPE, \
		&MY_TYPE::ID##_button_callback>);

} // namespace gui2

#endif

