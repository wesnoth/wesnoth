/*
   Copyright (C) 2017-2018 by Charles Dang <exodia339@gmail.com>
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
	, next_draw_(0)
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

	window.connect_signal<event::DRAW>(
		std::bind(&outro::draw_callback, this, std::ref(window)), event::dispatcher::front_child);

	set_next_draw();
}

void outro::set_next_draw()
{
	/* The UI is rendered at approximately 50 FPS - 1 frame every 20 ms - meaning fading progresses every 3 frames.
	 * TODO: not sure if 60 is a better value in that case?
	 */
	next_draw_ = SDL_GetTicks() + 60;
}

void outro::draw_callback(window& window)
{
	if(SDL_GetTicks() < next_draw_) {
		return;
	}

	/* If we've faded fully in...
	 *
	 * NOTE: we want fading to take around half a second. Given this function runs about every 3 frames, we
	 * limit ourselves to a reasonable 10 fade steps with an alpha difference (rounded up) of 25.5 each cycle.
	 * The actual calculation for alpha is done in the window definition in WFL.
	 */
	if(fading_in_ && fade_step_ > 10) {
		// Schedule the fadeout after the provided delay.
		if(timer_id_ == 0) {
			timer_id_ = add_timer(duration_, [this](size_t) { fading_in_ = false; });
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

	if(fading_in_) {
		fade_step_ ++;
	} else {
		fade_step_ --;
	}

	set_next_draw();
}

void outro::post_show(window& /*window*/)
{
	remove_timer(timer_id_);
	timer_id_ = 0;
}

} // namespace dialogs
} // namespace gui2
