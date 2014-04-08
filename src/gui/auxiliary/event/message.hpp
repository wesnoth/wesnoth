/*
   Copyright (C) 2011 - 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * This file contains the defintions for the @ref gui2::event::tmessage class.
 *
 * The class is used in the @ref gui2::event::tsignal_message_function
 */

#ifndef GUI_WIDGETS_AUXILIARY_EVENT_MESSAGE_HPP_INCLUDED
#define GUI_WIDGETS_AUXILIARY_EVENT_MESSAGE_HPP_INCLUDED

#include "gui/widgets/helper.hpp"

namespace gui2
{

namespace event
{

/**
 * The message callbacks hold a reference to a message.
 *
 * The contents of the message differ per type. This class is a base with a
 * virtual destructor, which makes it possible to use a dynamic_cast on the
 * class received to make sure the proper message type is send.
 *
 * This means all messages used in the events need to be derived from this
 * class. When a message needs no `content' it can send this class as message.
 * This is done by:
 * * @ref REQUEST_PLACEMENT
 */
struct tmessage
{
	virtual ~tmessage()
	{
	}
};

/** The message for MESSAGE_SHOW_TOOLTIP. */
struct tmessage_show_tooltip : public tmessage
{
	tmessage_show_tooltip(const std::string& message_, const tpoint& location_)
		: message(message_), location(location_)
	{
	}

	/** The message to show on the tooltip. */
	const std::string message;

	/** The location where to show the tooltip. */
	const tpoint location;
};

/** The message for MESSAGE_SHOW_HELPTIP. */
struct tmessage_show_helptip : public tmessage
{
	tmessage_show_helptip(const std::string& message_, const tpoint& location_)
		: message(message_), location(location_)
	{
	}

	/** The message to show on the helptip. */
	const std::string message;

	/** The location where to show the helptip. */
	const tpoint location;
};

} // namespace event

} // namespace gui2

#endif
