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

#ifndef GUI_DIALOGS_TRANSIENT_MESSAGE_HPP_INCLUDED
#define GUI_DIALOGS_TRANSIENT_MESSAGE_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "gui/widgets/control.hpp"

namespace gui2 {

/** Shows a transient message. */
class ttransient_message
	: public tdialog
{
public:
	ttransient_message(const std::string& title,
			bool title_use_markup,
			const std::string& message,
			bool message_use_markup)
		: title_(title)
		, title_use_markup_(title_use_markup)
		, message_(message)
		, message_use_markup_(message_use_markup)
	{}

protected:
	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

private:
	/** The title for the dialog. */
	std::string title_;

	/** Use  markup for the title. */
	bool title_use_markup_;

	/** The message to show to the user. */
	std::string message_;

	/** Use  markup for the message. */
	bool message_use_markup_;

	/** Inherited from tdialog. */
	twindow* build_window(CVideo& video);
};

/**
 * Shows a transient message to the user.
 *
 * This shows a dialog with a short message which can be dismissed with a
 * single click.
 *
 * @note The message _should_ be small enough to fit on the window, the test
 * can contain newlines and will wrap when needed.
 *
 * @param video               The video which contains the surface to draw
 *                            upon.
 * @param title               The title of the dialog.
 * @param message             The message to show in the dialog.
 * @param message_use_markup  Use markup for the message?
 * @param title_use_markup    Use markup for the title?
 */
void show_transient_message(CVideo& video, const std::string& title,
	const std::string& message,
	bool message_use_markup = false,
	bool title_use_markup = false);

/**
 * Shows a transient error message to the user.
 *
 * This shows a dialog with a short message which can be dismissed with a
 * single click.
 *
 * @param video               The video which contains the surface to draw
 *                            upon.
 * @param message             The message to show in the dialog.
 * @param message_use_markup  Use markup for the message?
 */
void show_transient_error_message(CVideo& video
		, const std::string& message
		, bool message_use_markup = false);

} // namespace gui2

#endif

