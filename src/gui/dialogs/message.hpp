/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_MESSAGE_HPP_INCLUDED
#define GUI_DIALOGS_MESSAGE_HPP_INCLUDED

#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/styled_widget.hpp"

namespace gui2
{

class button;

namespace dialogs
{

/**
 * Main class to show messages to the user.
 *
 * It can be used to show a message or ask a result from the user. For the
 * most common usage cases there are helper functions defined.
 */
class message : public modal_dialog
{
	friend struct message_implementation;

public:
	message(const std::string& title,
			 const std::string& message,
			 const bool auto_close,
			 const bool message_use_markup)
		: title_(title)
		, image_()
		, message_(message)
		, auto_close_(auto_close)
		, message_use_markup_(message_use_markup)
		, buttons_(count)
	{
	}

	enum button_id {
		left_1 = 0,
		cancel,
		ok,
		right_1,
		count
	};

	/**
	 * Selects the style of the buttons to be shown.
	 *
	 * These values are not directly implemented in this class but are used
	 * by our helper functions.
	 */
	enum button_style {
		auto_close /**< Enables auto close. */
		,
		ok_button /**< Shows an ok button. */
		,
		close_button /**< Shows a close button. */
		,
		ok_cancel_buttons /**< Shows an ok and cancel button. */
		,
		cancel_button /**< Shows a cancel button. */
		,
		yes_no_buttons /**< Shows a yes and no button. */
	};

	void set_button_caption(const button_id button,
							const std::string& caption);

	void set_button_visible(const button_id button,
							const widget::visibility visible);

	void set_button_retval(const button_id button, const int retval);

	/***** ***** ***** setters / getters for members ***** ****** *****/

	void set_title(const std::string& title)
	{
		title_ = title;
	}

	void set_image(const std::string& image)
	{
		image_ = image;
	}

	void set_message(const std::string& message)
	{
		message_ = message;
	}

	void set_auto_close(const bool auto_close)
	{
		auto_close_ = auto_close;
	}

protected:
	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Inherited from modal_dialog. */
	virtual void post_show(window& window) override;

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
	 * Does the window need to use click_dismiss when the dialog doesn't need a
	 * scrollbar.
	 */
	bool auto_close_;

	/** Whether to enable formatting markup for the dialog message. */
	bool message_use_markup_;

	struct button_status
	{
		button_status();

		button* ptr;
		std::string caption;
		widget::visibility visible;
		int retval;
	};

	/** Holds a pointer to the buttons. */
	std::vector<button_status> buttons_;

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;
};
} // namespace dialogs

/**
 * Shows a message to the user.
 *
 * Normally the dialog won't have a button only when the text doesn't fit in
 * the dialog and a scrollbar is used the button will be shown.
 *
 * @param video               The video which contains the surface to draw upon.
 * @param title               The title of the dialog.
 * @param message             The message to show in the dialog.
 * @param button_caption      The caption of the close button.
 * @param auto_close          When true the window will hide the ok button
 *                            when the message doesn't need a scrollbar to
 *                            show itself.
 */
void show_message(CVideo& video,
				  const std::string& title,
				  const std::string& message,
				  const std::string& button_caption = "",
				  const bool auto_close = true,
				  const bool message_use_markup = false);

/**
 * Shows a message to the user.
 *
 * @note this function is rather untested, and the API might change in the
 * near future.
 *
 * @param video               The video which contains the surface to draw
 *                            upon.
 * @param title               The title of the dialog.
 * @param message             The message to show in the dialog.
 * @param button_style        The style of the button(s) shown.
 * @param message_use_markup  Use markup for the message?
 * @param title_use_markup    Use markup for the title?
 *
 * @returns                   The retval of the dialog shown.
 */
int show_message(CVideo& video,
				 const std::string& title,
				 const std::string& message,
				 const dialogs::message::button_style button_style,
				 bool message_use_markup = false,
				 bool title_use_markup = false);

/**
 * Shows an error message to the user.
 *
 * @param video               The video which contains the surface to draw
 *                            upon.
 * @param message             The message to show in the dialog.
 * @param message_use_markup  Use markup for the message?
 */
void show_error_message(CVideo& video,
						const std::string& message,
						bool message_use_markup = false);

} // namespace gui2

#endif
