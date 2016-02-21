/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_HELPER_HPP_INCLUDED
#define GUI_DIALOGS_HELPER_HPP_INCLUDED

#include "gui/widgets/window.hpp"

#include <boost/bind.hpp>

namespace gui2
{

/**
 * Template for dialog callbacks. Example usage:
 * widget->set_callback(dialog_callback<my_dialog_class,
 * &my_dialog_class::member_function>);
 */
template <class D, void (D::*fptr)(twindow&)>
void dialog_callback(twidget& caller)
{
	D* dialog = dynamic_cast<D*>(caller.dialog());
	assert(dialog);
	twindow* window = caller.get_window();
	assert(window);
	(dialog->*fptr)(*window);
}

typedef boost::function<void(twindow &)> dialog_member_func_type;

inline void make_dialog_callback_helper(const dialog_member_func_type & t, 
		twidget & caller)
{
	twindow * window = caller.get_window();
	assert(window);
	t(*window);
}

inline boost::function<void(twidget &)> make_dialog_callback(
		dialog_member_func_type func)
{
	return boost::bind(make_dialog_callback_helper, func, _1);
}

} // namespace gui2

#endif
