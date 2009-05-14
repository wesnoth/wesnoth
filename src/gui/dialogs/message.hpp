/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_MESSAGE_HPP_INCLUDED
#define GUI_DIALOGS_MESSAGE_HPP_INCLUDED

#include "gui/widgets/widget.hpp"
#include "gui/dialogs/dialog.hpp"

namespace gui2 {

class tbutton;

/**
 * Main class to show messages to the user.
 *
 * It can be used to show a message or ask a result from the user. For the
 * most common usage cases there are helper functions defined.
 */
class tmessage : public tdialog
{
	friend struct tmessage_implementation;
public:
	tmessage(const std::string& title, const std::string& message,
			const bool auto_close)
		: title_(title)
		, image_()
		, message_(message)
		, auto_close_(auto_close)
		, buttons_(count)
	{}

	enum tbutton_id {
		  left_1 = 0
		, cancel
		, ok
		, right_1
		, count
	};

	void set_button_caption(const tbutton_id button,
			const std::string& caption);

	void set_button_visible(const tbutton_id button,
			const twidget::tvisible visible);

	void set_button_retval(const tbutton_id button,
			const int retval);

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_title(const std::string& title) {  title_ = title; }

	void set_image(const std::string& image) {  image_ = image; }

	void set_message(const std::string& message) {  message_ = message; }

	void set_auto_close(const bool auto_close) { auto_close_ = auto_close; }

protected:
	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

private:
	/** The title for the dialog. */
	std::string title_;

	/**
	 * The image which is shown in the dialog.
	 *
	 * This image can be an icon or portrait or any other image.
	 */
	std::string image_;

	/** The message to show to the user. */
	std::string message_;

	/**
	 * Does the window need to use easy_close when the dialog doesn't need a
	 * scrollbar.
	 */
	bool auto_close_;

	struct tbutton_status
	{
		tbutton_status();

		tbutton* button;
		std::string caption;
		twidget::tvisible visible;
		int retval;
	};

	/** Holds a pointer to the buttons. */
	std::vector<tbutton_status> buttons_;

	/** Inherited from tdialog. */
	twindow* build_window(CVideo& video);
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
 * @param auto_close          When true the window will hide the ok button
 *                            when the message doesn't need a scrollbar to
 *                            show itself.
 */
void show_message(CVideo& video, const std::string& title,
	const std::string& message, const std::string& button_caption = "",
	const bool auto_close = true);

} // namespace gui2

#endif

