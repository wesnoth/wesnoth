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
#include "gui/widgets/window.hpp"
#include "serialization/chrono.hpp"
#include "serialization/markup.hpp"

#include <cmath>

using namespace std::chrono_literals;

namespace
{

// How long text fading should take - currently a hardcoded value.
constexpr auto fade_duration = 500ms;

}

namespace gui2::dialogs
{
REGISTER_DIALOG(outro)

outro::outro(const game_classification& info)
	: modal_dialog(window_id())
	, text_()
	, text_index_(0)
	, display_duration_(info.end_text_duration)
	, stage_(stage::fading_in)
	, stage_start_()
{
	if(!info.end_text.empty()) {
		text_.push_back(info.end_text);
	} else {
		text_.push_back(_("The End"));
	}

	if(info.end_credits) {
		text_.push_back(markup::span_size("large", info.campaign_name));

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
						ss << markup::span_size("xx-small", about.names[k].first) << "\n";
					}

					// Clean up the trailing newline
					std::string section_text = ss.str();
					section_text.pop_back();

					text_.push_back(std::move(section_text));
				}
			}
		}
	}

	if(display_duration_ == 0ms) {
		display_duration_ = 3500ms; // 3.5 seconds
	}
}

void outro::pre_show()
{
	set_enter_disabled(true);
	get_canvas(0).set_variable("outro_text", wfl::variant(text_[0]));
	get_canvas(0).set_variable("fade_alpha", wfl::variant(0));
}

void outro::update()
{
	// window doesn't immediately close, keep returning until it does
	if(text_index_ >= text_.size()) {
		return;
	}

	const auto now = std::chrono::steady_clock::now();
	canvas& window_canvas = window::get_canvas(0);

	// TODO: Setting this in pre_show doesn't work, since it looks like it gets
	// overwritten by styled_widget::update_canvas. But it's also weird to have
	// this here. Find a better way to do this...
	window_canvas.set_variable("text_wrap_mode", wfl::variant(PANGO_ELLIPSIZE_NONE));

	const auto goto_stage = [this, &now](stage new_stage) {
		stage_ = new_stage;
		stage_start_ = now;
	};

	if(stage_start_ == std::chrono::steady_clock::time_point{}) {
		stage_start_ = now;
	}

	switch(stage_) {
	case stage::fading_in:
		if(now <= stage_start_ + fade_duration) {
			window_canvas.set_variable("fade_alpha", wfl::variant(float_to_color(get_fade_progress(now))));
		} else {
			goto_stage(stage::waiting);
		}
		break;

	case stage::waiting:
		if(now <= stage_start_ + display_duration_) {
			return; // zzzzzzz....
		} else {
			goto_stage(stage::fading_out);
		}
		break;

	case stage::fading_out:
		if(now <= stage_start_ + fade_duration) {
			window_canvas.set_variable("fade_alpha", wfl::variant(float_to_color(1.0 - get_fade_progress(now))));
		} else if(++text_index_ < text_.size()) {
			window_canvas.set_variable("outro_text", wfl::variant(text_[text_index_]));
			goto_stage(stage::fading_in);
		} else {
			window::close();
		}
		break;

	default:
		break;
	}

	window_canvas.update_size_variables();
	queue_redraw();
}

double outro::get_fade_progress(const std::chrono::steady_clock::time_point& now) const
{
	return chrono::normalize_progress(now - stage_start_, fade_duration);
}

} // namespace dialogs
