/*
   Copyright (C) 2017 by Charles Dang <exodia339@gmail.com>
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

#include "gui/dialogs/outro.hpp"

#include "formula/variant.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/timer.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

namespace gui2
{
namespace dialogs
{

REGISTER_DIALOG(outro)

outro::outro(const std::string& text, unsigned int duration)
	: text_(text)
	, duration_(duration)
	, fade_step_(0)
	, fading_in_(true)
	, timer_id_(0)
	, timer_id_secondary_(0)
{
	if(text_.empty()) {
		text_ = _("The End");
	}

	if(!duration_) {
		duration_ = 3500;
	}
}

void outro::pre_show(window& window)
{
	window.set_enter_disabled(true);
	window.get_canvas(0).set_variable("outro_text", wfl::variant(text_));

	timer_id_ = add_timer(50, std::bind(&outro::timer_callback, this, std::ref(window)), true);
}

void outro::timer_callback(window& window)
{
	// If we've faded fully in...
	if(fading_in_ && fade_step_ == 255) {
		// Schedule the fadeout after the provided delay.
		if(timer_id_secondary_ == 0) {
			timer_id_secondary_ = add_timer(duration_, [this](size_t) { fading_in_ = false; });
		}

		return;
	}

	// If we've faded fully out...
	if(!fading_in_ && fade_step_ < 0) {
		window.close();
		return;
	}

	canvas& window_canvas = window.get_canvas(0);

	window_canvas.set_variable("fade_step", wfl::variant(fade_step_));
	window_canvas.set_is_dirty(true);

	window.set_is_dirty(true);

	if(fading_in_) {
		fade_step_ += 5;
	} else {
		fade_step_ -= 5;
	}
}

void outro::post_show(window& /*window*/)
{
	remove_timer(timer_id_);
	remove_timer(timer_id_secondary_);

	timer_id_ = 0;
	timer_id_secondary_ = 0;
}

} // namespace dialogs
} // namespace gui2
