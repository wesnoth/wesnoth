/*
	Copyright (C) 2010 - 2024
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

#include "gui/dialogs/modeless_dialog.hpp"

#include "gui/core/event/dispatcher.hpp"
#include "gui/core/top_level_drawable.hpp"

namespace gui2
{

class styled_widget;
class pane;
class progress_bar;
class integer_selector;

namespace dialogs
{

/**
 * @ingroup GUIWindowDefinitionWML
 *
 * Clock to test the draw events.
 *
 * This shows the dialog for keeping track of the drawing events related to the current time. (This window is used for debug purposes only.)
 * Key               |Type              |Mandatory|Description
 * ------------------|------------------|---------|-----------
 * hour_percentage   | progress_bar     |no       |This shows the hours as a percentage, where 24 hours is 100%.
 * minute_percentage | progress_bar     |no       |This shows the minutes as a percentage, where 60 minutes is 100%.
 * second_percentage | progress_bar     |no       |This shows the seconds as a percentage, where 60 seconds is 100%.
 * hour              | integer_selector |no       |This shows the seconds since the beginning of the day. The control should have a minimum_value of 0 and a maximum_value of 86399 (246060 - 1).
 * minute            | integer_selector |no       |This shows the seconds since the beginning of the current hour. The control should have a minimum_value of 0 and a maximum_value of 3599 (6060 - 1).
 * second            | integer_selector |no       |This shows the seconds since the beginning of the current minute. The control should have a minimum_value of 0 and a maximum_value of 59.
 * clock             | control          |no       |A control which will have set three variables in its canvas:<ul><li>hour - the same value as the hour integer_selector.</li><li>minute - the same value as the minute integer_selector.</li><li>second - the same value as the second integer_selector.</li></ul>The control can then show the time in its own preferred format(s).
 */
class debug_clock : public modeless_dialog
{
public:
	debug_clock();

private:
	/** Progress bar for displaying the hours as a percentage. */
	progress_bar* hour_percentage_;

	/** Progress bar for displaying the minutes as a percentage. */
	progress_bar* minute_percentage_;

	/** Progress bar for displaying the seconds as a percentage. */
	progress_bar* second_percentage_;

	/** An integer selector to display the total seconds. */
	integer_selector* hour_;

	/** An integer selector to display the total seconds this hour. */
	integer_selector* minute_;

	/** An integer selector to display the seconds this minute. */
	integer_selector* second_;

	pane* pane_;

	/** A widget that can display the time. */
	styled_widget* clock_;

	/** The signal patched in the drawing routine. */
	event::signal signal_;

	/** Helper struct to keep track of the time. */
	struct time
	{
		time();

		/**
		 * Sets the fields to the current time.
		 *
		 * @note The milliseconds aren't queried and set to zero.
		 */
		void set_current_time();

		/**
		 * Moves the clock x milliseconds forward.
		 *
		 * @note The default value of @p milliseconds is the same as the
		 * interval for the drawing routine.
		 *
		 * @pre @p milliseconds < 1000.
		 *
		 * @param milliseconds    The number of milliseconds to move ahead.
		 *
		 * @returns               Did the number of seconds alter?
		 */
		bool step(const unsigned milliseconds = 30);

		/** The number of hours. */
		unsigned hour;

		/** The number of minutes. */
		unsigned minute;

		/** The number of seconds. */
		unsigned second;

		/** The number of milliseconds. */
		unsigned millisecond;
	};

	/**
	 * The `current' time.
	 *
	 * @note Since the dialog is used to test the drawing routine by keeping
	 * track of the calls to the drawing routine, the clock might be off.
	 */
	time time_;

	/** The type of window this is. */
	virtual const std::string& window_id() const override;

	/**
	 * The callback for the drawing routine.
	 *
	 * It updates the `time' in the various controls.
	 *
	 * @param force               Force an update even it the time didn't
	 *                            change? (This is used to set the clock
	 *                            initially.)
	 */
	void update_time(const bool force);

	/* top_level_drawable interface */
	virtual void update() override;
};

} // namespace dialogs
} // namespace gui2
