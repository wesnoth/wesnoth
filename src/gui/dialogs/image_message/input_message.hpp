/* $Id$ */
/*
   Copyright (C) 2008 - 2011 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_INPUT_MESSAGE_HPP_INCLUDED
#define GUI_DIALOGS_INPUT_MESSAGE_HPP_INCLUDED

#include "gui/dialogs/image_message/image_message.hpp"


namespace gui2 {

/**
 * image messages offering a text input dialog.
 *
 * We have a separate sub class for left and right images.
 */
class tinput_message_
	: public timage_message_
{
public:
	tinput_message_(const std::string& title, const std::string& message,
			const std::string& portrait, const bool mirror,
			const std::string& caption, std::string* text, const unsigned maximum_length)
		: timage_message_(title, message, portrait, mirror)
		, input_caption_(caption)
		, input_text_(text)
		, input_maximum_lenght_(maximum_length)
	{
	}

private:

	/** The caption to show for the input text. */
	std::string input_caption_;

	/** The text input. */
	std::string* input_text_;

	/** The maximum length of the input text. */
	unsigned input_maximum_lenght_;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
};

/** Shows a dialog with the portrait on the left side. */
class tinput_message_left : public tinput_message_
{
public:
	tinput_message_left(const std::string& title, const std::string& message,
			const std::string& portrait, const bool mirror,
			const std::string& caption, std::string* text, const unsigned maximum_length)
		: tinput_message_(title, message, portrait, mirror, caption, text, maximum_length)
	{
	}

private:

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;
};

/** Shows a dialog with the portrait on the right side. */
class tinput_message_right : public tinput_message_
{
public:
	tinput_message_right(const std::string& title, const std::string& message,
			const std::string& portrait, const bool mirror,
			const std::string& caption, std::string* text, const unsigned maximum_length)
		: tinput_message_(title, message, portrait, mirror, caption, text, maximum_length)
	{
	}

private:

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;
};


/**
 *  Helper function to show a portrait.
 *
 *  @param left_side              If true the portrait is shown on the left,
 *                                on the right side otherwise.
 *  @param video                  The display variable.
 *  @param title                  The title of the dialog.
 *  @param message                The message to show.
 *  @param portrait               Filename of the portrait.
 *  @param mirror                 Does the portrait need to be mirrored?
 *
 *  @param input_caption          The caption for the optional input text
 *                                box. If this value != "" there is an input
 *                                and the input text parameter is mandatory.
 *  @param input_text             Pointer to the initial text value will be
 *                                set to the result.
 *  @param maximum_length         The maximum length of the text.
 */
int show_input_message(const bool left_side
		, CVideo& video
		, const std::string& title
		, const std::string& message
		, const std::string& portrait
		, const bool mirror
		, const std::string& input_caption
		, std::string* input_text
	    , const unsigned maximum_length);

} // namespace gui2

#endif

