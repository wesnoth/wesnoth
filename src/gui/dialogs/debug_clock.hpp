/*
   Copyright (C) 2010 - 2015 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_DEBUG_CLOCK_HPP_INCLUDED
#define GUI_DIALOGS_DEBUG_CLOCK_HPP_INCLUDED

#include "gui/dialogs/popup.hpp"

#include "gui/auxiliary/event/dispatcher.hpp"

namespace gui2
{

class tcontrol;
class tpane;
class tprogress_bar;
class tinteger_selector_;

/** Clock to test the draw events. */
class tdebug_clock : public tpopup
{
public:
	tdebug_clock()
		: tpopup()
		, hour_percentage_(NULL)
		, minute_percentage_(NULL)
		, second_percentage_(NULL)
		, hour_(NULL)
		, minute_(NULL)
		, second_(NULL)
		, pane_(NULL)
		, clock_(NULL)
		, window_(NULL)
		, signal_()
		, time_()
	{
	}

private:
	/** Progress bar for displaying the hours as a percentage. */
	tprogress_bar* hour_percentage_;

	/** Progress bar for displaying the minutes as a percentage. */
	tprogress_bar* minute_percentage_;

	/** Progress bar for displaying the seconds as a percentage. */
	tprogress_bar* second_percentage_;

	/** An integer selector to display the total seconds. */
	tinteger_selector_* hour_;

	/** An integer selector to display the total seconds this hour. */
	tinteger_selector_* minute_;

	/** An integer selector to display the seconds this minute. */
	tinteger_selector_* second_;

	tpane* pane_;

	/** A widget that can display the time. */
	tcontrol* clock_;

	/** The window being shown. */
	twindow* window_;

	/** The signal patched in the drawing routine. */
	event::tsignal_function signal_;

	/** Helper struct to keep track of the time. */
	struct ttime
	{
		ttime();

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
	ttime time_;

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(CVideo& video);

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
};

} // namespace gui2

#endif
