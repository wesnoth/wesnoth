/*
	Copyright (C) 2017 - 2024
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
#include "gui/core/timer.hpp"
#include "gui/widgets/window.hpp"

#include <cmath>

namespace
{

// How long text fading should take - currently a hardcoded value.
const unsigned FADE_DURATION_MS = 500;

} // end unnamed namespace

namespace gui2::dialogs
{
REGISTER_DIALOG(outro)

outro::outro(const game_classification& info)
	: modal_dialog(window_id())
	, text_()
	, current_text_()
	, text_index_(0)
	, duration_(info.end_text_duration)
	, fade_alpha_(0)
	, fade_start_(0)
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

	current_text_ = text_[0];

	if(!duration_) {
		duration_ = 3500; // 3.5 seconds
	}
}

void outro::pre_show()
{
	set_enter_disabled(true);
	get_canvas(0).set_variable("outro_text", wfl::variant(current_text_));
}

void outro::update()
{
	// window doesn't immediately close, keep returning until it does
	if(text_index_ >= text_.size()) {
		return;
	}

	if(fade_start_ == 0) {
		fade_start_ = SDL_GetTicks();
	}

	// If we've faded fully in...
	if(fading_in_ && fade_alpha_ >= 255) {
		// Schedule the fadeout after the provided delay.
		if(timer_id_ == 0) {
			timer_id_ = add_timer(duration_, [this](std::size_t) {
				fading_in_ = false;
				fade_start_ = 0;
			});
		}

		return;
	}

	canvas& window_canvas = window::get_canvas(0);

	// If we've faded fully out...
	if(!fading_in_ && fade_alpha_ == 0) {
		// ...and we've just showed the last text bit, close the window.
		text_index_++;
		if(text_index_ >= text_.size()) {
			window::close();
			return;
		}
		current_text_ = text_[text_index_];

		// ...else show the next bit.
		window_canvas.set_variable("outro_text", wfl::variant(current_text_));

		fading_in_ = true;

		remove_timer(timer_id_);
		timer_id_ = 0;

		fade_start_ = SDL_GetTicks();
	}

	window_canvas.set_variable("fade_alpha", wfl::variant(fade_alpha_));
	window_canvas.update_size_variables();
	queue_redraw();

	auto current_ticks = SDL_GetTicks();

	if(fade_start_ > current_ticks) {
		// 32-bit ticks counter wraps around after about 49 days, the 64-bit version
		// requires SDL 2.0.18+. Just restart the counter in the worst case and let
		// the player deal with the sheer ridiculousness of their predicament.
		fade_start_ = current_ticks;
	}

	fade_alpha_ = std::clamp<int>(
		std::round(255.0 * double(current_ticks - fade_start_) / double(FADE_DURATION_MS)),
		0, 255);
	if(!fading_in_) {
		fade_alpha_ = 255 - fade_alpha_;
	}
}

void outro::post_show()
{
	remove_timer(timer_id_);
	timer_id_ = 0;
}

} // namespace dialogs
