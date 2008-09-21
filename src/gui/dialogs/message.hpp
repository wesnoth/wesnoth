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

#ifndef GUI_DIALOGS_MESSAGE_HPP_INCLUDED
#define GUI_DIALOGS_MESSAGE_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

namespace gui2 {

/**
 * Main class to show messages to the user.
 *
 * It can be used to show a message or ask a result from the user. For the most
 * common usage cases there are helper functions defined.
 */
class tmessage : public tdialog
{
public:
	tmessage(const std::string& title, const std::string& message)
		: title_(title)
		, message_(message)
	{}

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_title(const std::string& title) {  title_ = title; }

	void set_message(const std::string& message) {  message_ = message; }

private:
	/** The title for the dialog. */
	std::string title_;

	/** The message to show to the user. */
	std::string message_;

	/** Inherited from tdialog. */
	twindow build_window(CVideo& video);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);
};

/**
 * Shows a message to the user.
 *
 * Normally the dialog won't have a button only when the text doesn't fit in
 * the dialog and a scrollbar is used the button will be shown.
 *
 * @todo Since the click close function isn't implemented yet a button is
 * always shown.
 *
 * @param video               The video which contains the surface to draw upon.
 * @param title               The title of the dialog.
 * @param message             The message to show in the dialog.
 * @param button_caption      The caption of the close button.
 */
void show_message(CVideo& video, const std::string& title, 
	const std::string& message, const std::string& button_caption = "");

} // namespace gui2

#endif

