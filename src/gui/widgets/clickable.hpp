/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_CLICKABLE_HPP_INCLUDED
#define GUI_WIDGETS_CLICKABLE_HPP_INCLUDED

namespace gui2 {

/**
 * Small abstract helper class.
 *
 * Parts of the engine inherit this class so we can have generic
 * clickable items. This is mainly for the button and the repeating button
 * classes.
 */
class tclickable_
{
public:
	virtual ~tclickable_() {}

	/**
	 * Connects a signal handler for a 'click' event.
	 *
	 * What the click is depends on the subclass.
	 *
	 * @param signal              The signal to connect.
	 */
	virtual void connect_click_handler(const event::tsignal_function& signal) = 0;

	/**
	 * Disconnects a signal handler for a 'click' event.
	 *
	 * What the click is depends on the subclass.
	 *
	 * @param signal              The signal to disconnect (should be the same
	 *                            as send to the connect call.
	 */
	virtual void disconnect_click_handler(const event::tsignal_function& signal) = 0;
};

} // namespace gui2

#endif
