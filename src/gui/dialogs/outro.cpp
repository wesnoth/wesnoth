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
#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/timer.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

#include <cmath>

namespace
{

// How long text fading should take - currently a hardcoded value.
using namespace std::chrono_literals;
constexpr auto FADE_DURATION = 500ms;

} // end unnamed namespace

namespace gui2::dialogs
{
REGISTER_DIALOG(outro)

outro::outro(const game_classification& info)
	: modal_dialog(window_id())
	, alpha_queue_()
	, text_()
	, current_text_()
	, text_index_(0)
{
	// 3.5 seconds by default
	const auto show_duration = info.end_text_duration > 0 ? std::chrono::milliseconds{info.end_text_duration} : 3500ms;

	alpha_queue_ = {
		{0,   255, FADE_DURATION, utils::ease_out_cubic},
		{255, 255, show_duration, utils::ease_out_cubic},
		{255, 0,   FADE_DURATION, utils::ease_out_cubic},
	};

	alpha_queue_.on_complete(std::bind(&outro::advance_text, this));

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

void outro::pre_show(window& window)
{
	window.set_enter_disabled(true);
	window.get_canvas(0).set_variable("outro_text", wfl::variant(current_text_));
}

void outro::update()
{
	// window doesn't immediately close, keep returning until it does
	if(text_index_ >= text_.size()) {
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

		// ...else show the next bit.
		window_canvas.set_variable("outro_text", wfl::variant{current_text()});

		fading_in_ = true;

		remove_timer(timer_id_);
		timer_id_ = 0;

		fade_start_ = SDL_GetTicks();
	}

	window_canvas.set_variable("fade_alpha", wfl::variant(fade_alpha_));
	window_canvas.update_size_variables();

	queue_redraw();
}

void outro::advance_text()
{
	// Advance to the next text block, and either display it or close the window if we've shown them all.
	text_index_++;

	if(text_index_ < text_.size()) {
		window::get_canvas(0).set_variable("outro_text", wfl::variant{current_text()});
	} else {
		window::close();
	}
}

void outro::post_show(window& /*window*/)
{
}

} // namespace dialogs
