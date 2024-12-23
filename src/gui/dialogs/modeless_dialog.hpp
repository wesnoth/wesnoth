/*
	Copyright (C) 2011 - 2024
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

#include "gui/widgets/window.hpp"

#include <string>

namespace gui2::dialogs
{
/**
 * The popup class shows windows that are shown non-modal.
 *
 * At the moment these windows also don't capture the mouse and keyboard so can
 * only be used for things like tooltips. This behavior might change later.
 */
class modeless_dialog : public window
{
	/**
	 * Special helper function to get the id of the window.
	 *
	 * This is used in the unit tests, but these implementation details
	 * shouldn't be used in the normal code.
	 */
	friend std::string get_modeless_dialog_id(const modeless_dialog& dialog);

public:
	explicit modeless_dialog(const std::string& window_id);

	virtual ~modeless_dialog();

	/**
	 * Shows the window.
	 *
	 * @param allow_interaction   Does the dialog allow interaction?
	 *                            * true a non modal window is shown
	 *                            * false a tooltip window is shown
	 * @param auto_close_time     The time in ms after which the dialog will
	 *                            automatically close, if 0 it doesn't close.
	 *                            @note the timeout is a minimum time and
	 *                            there's no guarantee about how fast it closes
	 *                            after the minimum.
	 */
	void show(const bool allow_interaction = false,
			  const unsigned auto_close_time = 0);

private:
	/**
	 * The ID of the window to build. Usually defined by REGISTER_DIALOG.
	 *
	 * Falls back to widget::id(), which is set during construction.
	 */
	virtual const std::string& window_id() const { return widget::id(); }
};

} // namespace gui2::dialogs
