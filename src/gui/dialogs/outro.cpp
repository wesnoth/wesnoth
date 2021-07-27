/*
	Copyright (C) 2017 - 2021
	by Charles Dang <exodia339@gmail.com>
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

#include "gui/dialogs/outro.hpp"

#include "about.hpp"
#include "formula/variant.hpp"
#include "game_classification.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/timer.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

#include <cmath>

namespace gui2::dialogs
{
REGISTER_DIALOG(outro)

outro::outro(const game_classification& info)
	: text_()
	, current_text_()
	, duration_(info.end_text_duration)
	, fade_step_(0)
	, fading_in_(true)
	, timer_id_(0)
{
	if(!info.end_text.empty()) {
		text_.push_back(info.end_text);
	} else {
		text_.push_back(_("The End"));
	}

	if(info.end_credits) {
		text_.push_back("<span size='large'>" + info.campaign_name + "</span>");

		if(const auto campaign_credits = about::get_campaign_credits(info.campaign)) {
			for(const auto& about : (*campaign_credits)->sections) {
				if(about.names.empty()) {
					continue;
				}

				// Split the names into chunks of 5. Use float for proper ceil function!
				static const float chunk_size = 5.0;

				const unsigned num_names = about.names.size();
				const unsigned num_chunks = std::ceil(num_names / chunk_size);

				for(std::size_t i = 0; i < num_chunks; ++i) {
					std::stringstream ss;

					// Only include section title on first chunk
					if(i == 0) {
						ss << about.title << "\n\n";
					}

					for(std::size_t k = i * chunk_size; k < std::min<unsigned>((i + 1) * chunk_size, num_names); ++k) {
						ss << "<span size='xx-small'>" << about.names[k].first << "</span>\n";
					}

					// Clean up the trailing newline
					std::string section_text = ss.str();
					section_text.pop_back();

					text_.push_back(std::move(section_text));
				}
			}
		}
	}

	current_text_ = text_.begin();

	if(!duration_) {
		duration_ = 3500; // 3.5 seconds
	}
}

void outro::pre_show(window& window)
{
	window.set_enter_disabled(true);
	window.get_canvas(0).set_variable("outro_text", wfl::variant(*current_text_));

	connect_signal_on_draw(window, std::bind(&outro::draw_callback, this));
}

void outro::draw_callback()
{
	/* If we've faded fully in...
	 *
	 * NOTE: we want fading to take around half a second. Given this function runs about every 3 frames, we
	 * limit ourselves to a reasonable 10 fade steps with an alpha difference (rounded up) of 25.5 each cycle.
	 * The actual calculation for alpha is done in the window definition in WFL.
	 */
	if(fading_in_ && fade_step_ > 10) {
		// Schedule the fadeout after the provided delay.
		if(timer_id_ == 0) {
			timer_id_ = add_timer(duration_, [this](std::size_t) { fading_in_ = false; });
		}

		return;
	}

	canvas& window_canvas = get_window()->get_canvas(0);

	// If we've faded fully out...
	if(!fading_in_ && fade_step_ < 0) {
		std::advance(current_text_, 1);

		// ...and we've just showed the last text bit, close the window.
		if(current_text_ == text_.end()) {
			get_window()->close();
			return;
		}

		// ...else show the next bit.
		window_canvas.set_variable("outro_text", wfl::variant(*current_text_));

		fading_in_ = true;
		fade_step_ = 0;

		remove_timer(timer_id_);
		timer_id_ = 0;
	}

	window_canvas.set_variable("fade_step", wfl::variant(fade_step_));
	window_canvas.set_is_dirty(true);

	get_window()->set_is_dirty(true);

	if(fading_in_) {
		fade_step_++;
	} else {
		fade_step_--;
	}
}

void outro::post_show(window& /*window*/)
{
	remove_timer(timer_id_);
	timer_id_ = 0;
}

} // namespace dialogs
