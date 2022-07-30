/*
	Copyright (C) 2011 - 2022
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
 * This file contains the definitions for the @ref gui2::event::message class.
 *
 * The class is used in the @ref gui2::event::signal_message
 */

#pragma once

#include "gui/widgets/helper.hpp"
#include "sdl/point.hpp"

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
struct message
{
	message() = default;

	// Disallow copying because constructing a copy loses the exact type.
	message(const message&) = delete;

	virtual ~message()
	{
	}
};

/** The message for MESSAGE_SHOW_TOOLTIP. */
struct message_show_tooltip : public message
{
	message_show_tooltip(const std::string& message_, const point& location_, const SDL_Rect& source_rect_)
		: message(message_), location(location_), source_rect(source_rect_)
	{
	}

	/** The message to show on the tooltip. */
	const std::string message;

	/** The location where to show the tooltip. */
	const point location;

	/** The size of the entity requesting to show a tooltip. */
	const SDL_Rect source_rect;
};

/** The message for MESSAGE_SHOW_HELPTIP. */
struct message_show_helptip : public message
{
	message_show_helptip(const std::string& message_, const point& location_, const SDL_Rect& source_rect_)
		: message(message_), location(location_), source_rect(source_rect_)
	{
	}

	/** The message to show on the helptip. */
	const std::string message;

	/** The location where to show the helptip. */
	const point location;

	/** The size of the entity requesting to show a helptip. */
	const SDL_Rect source_rect;
};

} // namespace event

} // namespace gui2
