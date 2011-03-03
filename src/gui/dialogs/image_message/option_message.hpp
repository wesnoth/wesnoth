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

#ifndef GUI_DIALOGS_OPTION_MESSAGE_HPP_INCLUDED
#define GUI_DIALOGS_OPTION_MESSAGE_HPP_INCLUDED

#include "gui/dialogs/image_message/image_message.hpp"

namespace gui2 {

/**
 * Class for displaying a wml message with option list.
 *
 * We have a separate sub class for left and right images.
 */
class toption_message_
	: public timage_message_
{
public:
	toption_message_(const std::string& title, const std::string& message,
			const std::string& portrait, const bool mirror)
		: timage_message_(title, message, portrait, mirror)
		//, image_("")
		, option_list_()
		, chosen_option_(NULL)
	{
	}

	/**
	 * Sets the option list.
	 *
	 * @param option_list			Vector of options to choice from.
	 * @param chosen_option			Number of the option that was chosen.
	 */
	void set_option_list(
			const std::vector<std::string>& option_list, int* chosen_option);

private:

	/** The list of options the player can choose. */
	std::vector<std::string> option_list_;

	/** The chosen option. */
	int *chosen_option_;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
};

/** Shows a dialog with the portrait on the left side. */
class toption_message_left : public toption_message_
{
public:
	toption_message_left(const std::string& title, const std::string& message,
			const std::string& portrait, const bool mirror)
		: toption_message_(title, message, portrait, mirror)
	{
	}

private:

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;
};

/** Shows a dialog with the portrait on the right side. */
class toption_message_right : public toption_message_
{
public:
	toption_message_right(const std::string& title, const std::string& message,
			const std::string& portrait, const bool mirror)
		: toption_message_(title, message, portrait, mirror)
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
 *  @param option_list            A list of options to select in the dialog.
 *  @param chosen_option          Pointer to the initially chosen option.
 *                                Will be set to the chosen_option when the
 *                                dialog closes.
 */
int show_option_message(const bool left_side
		, CVideo& video
		, const std::string& title
		, const std::string& message
		, const std::string& portrait
		, const bool mirror
		, const std::vector<std::string>& option_list
		, int* chosen_option);

} // namespace gui2

#endif

