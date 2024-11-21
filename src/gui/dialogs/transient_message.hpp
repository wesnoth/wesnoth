/*
	Copyright (C) 2009 - 2024
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

#pragma once

#include "gui/dialogs/modal_dialog.hpp"

namespace gui2
{
namespace dialogs
{

/** Shows a transient message. */
class transient_message : public modal_dialog
{
public:
	transient_message(const std::string& title,
					   const bool title_use_markup,
					   const std::string& message,
					   const bool message_use_markup,
					   const std::string& image);

	DEFINE_SIMPLE_DISPLAY_WRAPPER(transient_message)

private:
	bool hide_title_;
	bool hide_image_;

	virtual const std::string& window_id() const override;

	virtual void pre_show() override;
};
} // namespace dialogs

/**
 * Shows a transient message to the user.
 *
 * This shows a dialog with a short message which can be dismissed with a
 * single click.
 *
 * @note The message _should_ be small enough to fit on the window, the text
 * can contain newlines and will wrap when needed.
 *
 * @param title               The title of the dialog.
 * @param message             The message to show in the dialog.
 * @param image               An image to show in the dialog.
 * @param message_use_markup  Use markup for the message?
 * @param title_use_markup    Use markup for the title?
 */
void show_transient_message(const std::string& title,
							const std::string& message,
							const std::string& image = std::string(),
							const bool message_use_markup = false,
							const bool title_use_markup = false);

/**
 * Shows a transient error message to the user.
 *
 * This shows a dialog with a short message which can be dismissed with a
 * single click.
 *
 * @param message             The message to show in the dialog.
 * @param image               An image to show in the dialog.
 * @param message_use_markup  Use markup for the message?
 */
void show_transient_error_message(const std::string& message,
								  const std::string& image = std::string(),
								  const bool message_use_markup = false);

} // namespace gui2
