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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/debug_clock.hpp"

#include "gui/auxiliary/find_widget.tpp"
#include "gui/dialogs/dialog.hpp"
#include "gui/widgets/integer_selector.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/pane.hpp"
#include "gui/widgets/progress_bar.hpp"
#include "utils/foreach.tpp"

#include <boost/bind.hpp>

#include <ctime>

namespace gui2
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_clock
 *
 * == Clock ==
 *
 * This shows the dialog for keeping track of the drawing events related to the
 * current time. (This window is used for debug purposes only.)
 *
 * @begin{table}{dialog_widgets}
 * hour_percentage   & & progress_bar     & o &
 *         This shows the hours as a percentage, where 24 hours is 100%. $
 * minute_percentage & & progress_bar     & o &
 *         This shows the minutes as a percentage, where 60 minutes is 100%. $
 * second_percentage & & progress_bar     & o &
 *         This shows the seconds as a percentage, where 60 seconds is 100%. $
 *
 * hour              & & integer_selector & o &
 *         This shows the seconds since the beginning of the day. The control
 *         should have a ''minimum_value'' of 0 and a ''maximum_value'' of 86399
 *         (24 * 60 * 60 - 1). $
 *
 * minute            & & integer_selector & o &
 *         This shows the seconds since the beginning of the current hour. The
 *         control should have a ''minimum_value'' of 0 and a ''maximum_value''
 *         of 3599 (60 * 60 - 1). $
 *
 * minute            & & integer_selector & o &
 *         This shows the seconds since the beginning of the current minute. The
 *         control should have a ''minimum_value'' of 0 and a ''maximum_value''
 *         of 59. $
 *
 * clock             & & control          & o &
 *         A control which will have set three variables in its canvas:
 *         @* hour, the same value as the hour integer_selector.
 *         @* minute, the same value as the minute integer_selector.
 *         @* second, the same value as the second integer_selector.
 *         @- the control can then should the time in its own preferred
 *         format(s). $
 * @end{table}
 */

REGISTER_DIALOG(debug_clock)

void tdebug_clock::pre_show(CVideo& /*video*/, twindow& window)
{
	hour_percentage_ = find_widget<tprogress_bar>(
			&window, "hour_percentage", false, false);
	minute_percentage_ = find_widget<tprogress_bar>(
			&window, "minute_percentage", false, false);
	second_percentage_ = find_widget<tprogress_bar>(
			&window, "second_percentage", false, false);

	hour_ = find_widget<tinteger_selector_>(&window, "hour", false, false);
	if(tcontrol *hour = dynamic_cast<tcontrol*>(hour_)) { //Note that the standard specifies that a dynamic cast of a null pointer is null
		hour->set_active(false);
	}
	minute_ = find_widget<tinteger_selector_>(&window, "minute", false, false);
	if(tcontrol *minute = dynamic_cast<tcontrol*>(minute_)) {
		minute->set_active(false);
	}
	second_ = find_widget<tinteger_selector_>(&window, "second", false, false);
	if(tcontrol *second = dynamic_cast<tcontrol*>(second_)) {
		second->set_active(false);
	}

	pane_ = find_widget<tpane>(&window, "pane", false, false);

	clock_ = find_widget<tcontrol>(&window, "clock", false, false);

	window_ = &window;

	signal_ = boost::bind(&tdebug_clock::update_time, this, false);
	window.connect_signal<event::DRAW>(signal_,
									   event::tdispatcher::front_child);

	time_.set_current_time();
	update_time(true);
}

void tdebug_clock::post_show(CVideo& /*video*/)
{
	window_->disconnect_signal<event::DRAW>(signal_);
}

void tdebug_clock::update_time(const bool force)
{
	if(!time_.step() && !force) {
		return;
	}

	if(hour_percentage_) {
		hour_percentage_->set_percentage(time_.hour / 0.24);
	}
	if(minute_percentage_) {
		minute_percentage_->set_percentage(time_.minute / 0.60);
	}
	if(second_percentage_) {
		second_percentage_->set_percentage(time_.second / 0.60);
	}

	const int hour_stamp = time_.hour * 3600 + time_.minute * 60 + time_.second;
	const int minute_stamp = time_.minute * 60 + time_.second;
	const int second_stamp = time_.second;

	if(hour_) {
		hour_->set_value(hour_stamp);
	}
	if(minute_) {
		minute_->set_value(minute_stamp);
	}
	if(second_) {
		second_->set_value(second_stamp);
	}

	if(clock_) {
		FOREACH(AUTO & canvas, clock_->canvas())
		{
			canvas.set_variable("hour", variant(hour_stamp));
			canvas.set_variable("minute", variant(minute_stamp));
			canvas.set_variable("second", variant(second_stamp));
		}
		clock_->set_is_dirty(true);
	}

	const std::map<std::string, std::string> tags;
	std::map<std::string, string_map> item_data;
	string_map item;

	item["label"] = str_cast(second_stamp);
	item_data.insert(std::make_pair("time", item));

	if(pane_) {
		pane_->create_item(item_data, tags);
	}
}

tdebug_clock::ttime::ttime() : hour(0), minute(0), second(0), millisecond(0)
{
}

void tdebug_clock::ttime::set_current_time()
{
	time_t now = time(NULL);
	tm* stamp = localtime(&now);

	hour = stamp->tm_hour;
	minute = stamp->tm_min;
	second = stamp->tm_sec;
	millisecond = 0;
}

bool tdebug_clock::ttime::step(const unsigned milliseconds)
{
	millisecond += milliseconds;

	if(millisecond < 1000)
		return false;

	millisecond -= 1000;
	++second;

	if(second < 60)
		return true;

	second -= 60;
	++minute;

	if(minute < 60)
		return true;

	minute -= 60;
	++hour;

	if(hour < 24)
		return true;

	hour -= 24;

	return true;
}

} // namespace gui2
