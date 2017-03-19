/*
   Copyright (C) 2007 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_SELECTABLE_HPP_INCLUDED
#define GUI_WIDGETS_SELECTABLE_HPP_INCLUDED

#include "utils/functional.hpp"
#include <cassert>

namespace gui2
{

class widget;

/**
 * Small abstract helper class.
 *
 * Parts of the engine inherit this class so we can have generic
 * selectable items.
 */
class selectable_item
{
public:
	virtual ~selectable_item()
	{
	}

	/** Is the styled_widget selected? */
	virtual unsigned get_value() const = 0;

	/** Select the styled_widget. */
	virtual void set_value(const unsigned) = 0;

	/** The number of states, that is 2 for normal buttons, 3 for tristate buttons. */
	virtual unsigned num_states() const = 0;

	bool get_value_bool() const
	{
		assert(num_states() == 2);
		return get_value() != 0;
	}

	void set_value_bool(const bool value)
	{
		assert(num_states() == 2);
		return set_value(value);
	}
	/**
	 * When the user does something to change the widget state this event is
	 * fired. Most of the time it will be a left click on the widget.
	 */
	virtual void
	set_callback_state_change(std::function<void(widget&)> callback) = 0;
};

} // namespace gui2

#endif
