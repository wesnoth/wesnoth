/*
	Copyright (C) 2016 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/**
 * @file
 * Screen with logo and loading status info during program-startup.
 */

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/loading_screen.hpp"

#include "cursor.hpp"
#include "draw_manager.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/timer.hpp"
#include "gui/widgets/drawing.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "preferences/general.hpp"
#include "sdl/rect.hpp"
#include "video.hpp"

#include <cstdlib>
#include <functional>

static lg::log_domain log_loadscreen("loadscreen");
#define LOG_LS LOG_STREAM(info, log_loadscreen)
#define ERR_LS LOG_STREAM(err, log_loadscreen)
#define WRN_LS LOG_STREAM(warn, log_loadscreen)

static lg::log_domain log_display("display");
#define DBG_DP LOG_STREAM(debug, log_display)

static const std::map<loading_stage, std::string> stage_names {
	{ loading_stage::build_terrain,       N_("Building terrain rules") },
	{ loading_stage::create_cache,        N_("Reading files and creating cache") },
	{ loading_stage::init_display,        N_("Initializing display") },
	{ loading_stage::init_fonts,          N_("Reinitialize fonts for the current language") },
	{ loading_stage::init_teams,          N_("Initializing teams") },
	{ loading_stage::init_theme,          N_("Initializing display") },
	{ loading_stage::load_config,         N_("Loading game configuration") },
	{ loading_stage::load_data,           N_("Loading data files") },
	{ loading_stage::load_level,          N_("Loading level") },
	{ loading_stage::init_lua,            N_("Initializing scripting engine") },
	{ loading_stage::init_whiteboard,     N_("Initializing planning mode") },
	{ loading_stage::load_unit_types,     N_("Reading unit files") },
	{ loading_stage::load_units,          N_("Loading units") },
	{ loading_stage::refresh_addons,      N_("Searching for installed add-ons") },
	{ loading_stage::start_game,          N_("Starting game") },
	{ loading_stage::verify_cache,        N_("Verifying cache") },
	{ loading_stage::connect_to_server,   N_("Connecting to server") },
	{ loading_stage::login_response,      N_("Logging in") },
	{ loading_stage::waiting,             N_("Waiting for server") },
	{ loading_stage::redirect,            N_("Connecting to redirected server") },
	{ loading_stage::next_scenario,       N_("Waiting for next scenario") },
	{ loading_stage::download_level_data, N_("Getting game data") },
	{ loading_stage::download_lobby_data, N_("Downloading lobby data") },
};

namespace { int last_spin_ = 0; }

namespace gui2::dialogs
{
REGISTER_DIALOG(loading_screen)

loading_screen* loading_screen::singleton_ = nullptr;

loading_screen::loading_screen(std::function<void()> f)
	: modal_dialog(window_id())
	, load_funcs_{f}
	, worker_result_()
	, cursor_setter_()
	, progress_stage_label_(nullptr)
	, animation_(nullptr)
	, animation_start_()
	, current_stage_(loading_stage::none)
	, visible_stages_()
	, current_visible_stage_()
	, running_(false)
{
	for(const auto& [stage, description] : stage_names) {
		visible_stages_[stage] = t_string(description, "wesnoth-lib") + "...";
	}

	current_visible_stage_ = visible_stages_.end();
	singleton_ = this;
}

void loading_screen::pre_show(window& window)
{
	window.set_enter_disabled(true);
	window.set_escape_disabled(true);

	cursor_setter_.reset(new cursor::setter(cursor::WAIT));

	progress_stage_label_ = find_widget<label>(&window, "status", false, true);
	animation_ = find_widget<drawing>(&window, "animation", false, true);
}

void loading_screen::post_show(window& /*window*/)
{
	cursor_setter_.reset();
}

void loading_screen::progress(loading_stage stage)
{
	if(singleton_ && stage != loading_stage::none) {
		singleton_->current_stage_.store(stage, std::memory_order_release);
		// Allow display to update, close events to be handled, etc.
		events::pump_and_draw();
	}
}

void loading_screen::spin()
{
	// If we're not showing a loading screen, do nothing.
	if (!singleton_) {
		return;
	}

	// If we're not the main thread, do nothing.
	if (!events::is_in_main_thread()) {
		return;
	}

	// Restrict actual update rate.
	int elapsed = SDL_GetTicks() - last_spin_;
	if (elapsed > draw_manager::get_frame_length() || elapsed < 0) {
		last_spin_ = SDL_GetTicks();
		events::pump_and_draw();
	}
}

void loading_screen::raise()
{
	if (singleton_) {
		draw_manager::raise_drawable(singleton_);
	}
}

// This will be run inside the window::show() loop.
void loading_screen::process(events::pump_info&)
{
	if (load_funcs_.empty()) {
		return;
	}

	// Do not automatically recurse.
	if (running_) { return; }
	running_ = true;

	// Run the loading function.
	auto func = load_funcs_.back();
	load_funcs_.pop_back();
	LOG_LS << "Executing loading screen worker function.";
	func();

	running_ = false;

	// If there's nothing more to do, close.
	if (load_funcs_.empty()) {
		queue_redraw();
		window::close();
	}
}

void loading_screen::layout()
{
	modal_dialog::layout();

	DBG_DP << "loading_screen::layout";

	loading_stage stage = current_stage_.load(std::memory_order_acquire);

	if(stage != loading_stage::none && (current_visible_stage_ == visible_stages_.end() || stage != current_visible_stage_->first)) {
		auto iter = visible_stages_.find(stage);
		if(iter == visible_stages_.end()) {
			WRN_LS << "Stage missing description.";
			return;
		}

		current_visible_stage_ = iter;
		progress_stage_label_->set_label(iter->second);
	}

	using namespace std::chrono;
	const auto now = steady_clock::now();

	// We only need to set the start time once;
	if(!animation_start_.has_value()) {
		animation_start_ = now;
	}

	animation_->get_drawing_canvas().set_variable("time", wfl::variant(duration_cast<milliseconds>(now - *animation_start_).count()));
	animation_->queue_redraw();
}

loading_screen::~loading_screen()
{
	LOG_LS << "Loading screen destroyed.";
	singleton_ = nullptr;
}

void loading_screen::display(std::function<void()> f)
{
	if(singleton_ || video::headless()) {
		LOG_LS << "Directly executing loading function.";
		f();
	} else {
		LOG_LS << "Creating new loading screen.";
		loading_screen(f).show();
	}
}

} // namespace dialogs
