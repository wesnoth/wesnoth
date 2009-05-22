/* $Id$ */
/*
   Copyright (C) 2009 by Mark de Wever <koraq@xs4all.nl>
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
			const tcontrol::tmarkup_mode title_markup_mode,
			const std::string& message,
			const tcontrol::tmarkup_mode message_markup_mode)
		: title_(title)
		, title_markup_mode_(title_markup_mode)
		, message_(message)
		, message_markup_mode_(message_markup_mode)
	{}

protected:
	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

private:
	/** The title for the dialog. */
	std::string title_;

	/** The markup mode for the title. */
	tcontrol::tmarkup_mode title_markup_mode_;

	/** The message to show to the user. */
	std::string message_;

	/** The markup mode for the message. */
	tcontrol::tmarkup_mode message_markup_mode_;

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
 * @param message_markup_mode The markup mode used for the title.
 * @param title_markup_mod    The markup mode used for the title.
 */
void show_transient_message(CVideo& video, const std::string& title,
	const std::string& message,
	const tcontrol::tmarkup_mode message_markup_mode = tcontrol::NO_MARKUP,
	const tcontrol::tmarkup_mode title_markup_mode = tcontrol::NO_MARKUP);

} // namespace gui2

#endif

