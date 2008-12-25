/* $Id$ */
/*
   copyright (c) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#ifndef GUI_DIALOGS_WML_MESSAGE_HPP_INCLUDED
#define GUI_DIALOGS_WML_MESSAGE_HPP_INCLUDED

#include "gui/dialogs/message.hpp"

namespace gui2 {

/**
 * Base class for the wml generated messages.
 *
 * We have a separate sub class for left and right images.
 */
class twml_message_ : public tmessage
{
public:
	twml_message_(const std::string& title, const std::string& message,
			const std::string portrait, const bool mirror)
		: tmessage(title, message, true)
		, portrait_(portrait)
		, mirror_(mirror)
	{
	}

private:

	/** Filename of the portrait. */
	std::string portrait_;

	/** Mirror the portrait? */
	bool mirror_;

	/**
	 * Inherited from tmessage.
	 *
	 * The subclasses need to implement the left or right definition.
	 */
	twindow* build_window(CVideo& /*video*/) = 0;

	/** Inherited from tmessage. */
	void pre_show(CVideo& video, twindow& window);
};

/** Shows a dialog with the portrait on the left side. */
class twml_message_left : public twml_message_
{
public:
	twml_message_left(const std::string& title, const std::string& message,
			const std::string portrait, const bool mirror)
		: twml_message_(title, message, portrait, mirror)
	{
	}
private:
	/** Inherited from twml_message_. */
	twindow* build_window(CVideo& video);
};

/** Shows a dialog with the portrait on the right side. */
class twml_message_right : public twml_message_
{
public:
	twml_message_right(const std::string& title, const std::string& message,
			const std::string portrait, const bool mirror)
		: twml_message_(title, message, portrait, mirror)
	{
	}
private:
	/** Inherited from twml_message_. */
	twindow* build_window(CVideo& video);
};

} // namespace gui2

#endif

