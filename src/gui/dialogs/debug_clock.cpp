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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/debug_clock.hpp"

#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/integer_selector.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/pane.hpp"
#include "gui/widgets/progress_bar.hpp"


#include <ctime>

namespace gui2::dialogs
{

REGISTER_DIALOG(debug_clock)

debug_clock::debug_clock()
	: modeless_dialog(window_id())
	, signal_()
	, time_()
{
	hour_percentage_ = find_widget<progress_bar>("hour_percentage", false, false);
	minute_percentage_ = find_widget<progress_bar>("minute_percentage", false, false);
	second_percentage_ = find_widget<progress_bar>("second_percentage", false, false);

	hour_ = find_widget<integer_selector>("hour", false, false);
	if(styled_widget *hour = dynamic_cast<styled_widget*>(hour_)) { //Note that the standard specifies that a dynamic cast of a null pointer is null
		hour->set_active(false);
	}
	minute_ = find_widget<integer_selector>("minute", false, false);
	if(styled_widget *minute = dynamic_cast<styled_widget*>(minute_)) {
		minute->set_active(false);
	}
	second_ = find_widget<integer_selector>("second", false, false);
	if(styled_widget *second = dynamic_cast<styled_widget*>(second_)) {
		second->set_active(false);
	}

	pane_ = find_widget<pane>("pane", false, false);

	clock_ = find_widget<styled_widget>("clock", false, false);

	time_.set_current_time();
	update_time(true);
}

void debug_clock::update()
{
	update_time(false);
	window::update();
}

void debug_clock::update_time(const bool force)
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
		for(auto & canvas : clock_->get_canvases())
		{
			canvas.set_variable("hour", wfl::variant(hour_stamp));
			canvas.set_variable("minute", wfl::variant(minute_stamp));
			canvas.set_variable("second", wfl::variant(second_stamp));
		}
		clock_->queue_redraw();
	}

	const std::map<std::string, std::string> tags;
	widget_data item_data;
	widget_item item;

	item["label"] = std::to_string(second_stamp);
	item_data.emplace("time", item);

	if(pane_) {
		pane_->create_item(item_data, tags);
	}
}

debug_clock::time::time() : hour(0), minute(0), second(0), millisecond(0)
{
}

void debug_clock::time::set_current_time()
{
	std::time_t now = ::std::time(nullptr);
	std::tm* stamp = std::localtime(&now);

	hour = stamp->tm_hour;
	minute = stamp->tm_min;
	second = stamp->tm_sec;
	millisecond = 0;
}

bool debug_clock::time::step(const unsigned milliseconds)
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

} // namespace dialogs
