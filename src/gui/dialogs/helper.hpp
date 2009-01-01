/* $Id$ */
/*
   copyright (c) 2008 - 2009 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
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

} // namespace gui2

#endif

