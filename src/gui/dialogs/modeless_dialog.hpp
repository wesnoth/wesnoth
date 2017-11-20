/*
   Copyright (C) 2011 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include <memory>
#include <string>

namespace gui2
{

class window;

namespace dialogs
{

/**
 * The popup class shows windows that are shown non-modal.
 *
 * At the moment these windows also don't capture the mouse and keyboard so can
 * only be used for things like tooltips. This behavior might change later.
 */
class modeless_dialog
{
	/**
	 * Special helper function to get the id of the window.
	 *
	 * This is used in the unit tests, but these implementation details
	 * shouldn't be used in the normal code.
	 */
	friend std::string unit_test_mark_popup_as_tested(const modeless_dialog& dialog);

	/**
	 * Special helper function for the unit test to the the window.
	 *
	 * This is used in the unit tests, but these implementation details
	 * shouldn't be used in the normal code.
	 */
	friend window* unit_test_window(const modeless_dialog& dialog);

public:
	modeless_dialog();

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


	/**
	 * Hides the window.
	 *
	 * The hiding also destroys the window. It is save to call the function
	 * when the window is not shown.
	 */
	void hide();

	/** Returns a pointer to the dialog's window. Will be null if it hasn't been built yet. */
	window* get_window() const
	{
		return window_.get();
	}

private:
	/** The window, used in show. */
	std::unique_ptr<window> window_;

	/** The id of the window to build. */
	virtual const std::string& window_id() const = 0;

	/**
	 * Builds the window.
	 *
	 * Every dialog shows it's own kind of window, this function should return
	 * the window to show.
	 *
	 * @returns                   The window to show.
	 */
	window* build_window() const;

	/**
	 * Actions to be taken directly after the window is build.
	 *
	 * @param window              The window just created.
	 */
	virtual void post_build(window& window);

	/**
	 * Actions to be taken before showing the window.
	 *
	 * @param window              The window to be shown.
	 */
	virtual void pre_show(window& window);
};

} // namespace dialogs
} // namespace gui2
