/*
	Copyright (C) 2016 - 2021
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
#include "gettext.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/timer.hpp"
#include "gui/widgets/drawing.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "preferences/general.hpp"
#include "video.hpp"

#include <cstdlib>
#include <functional>

static lg::log_domain log_loadscreen("loadscreen");
#define ERR_LS LOG_STREAM(err, log_loadscreen)
#define WRN_LS LOG_STREAM(warn, log_loadscreen)

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

namespace gui2::dialogs
{
REGISTER_DIALOG(loading_screen)

loading_screen* loading_screen::singleton_ = nullptr;

loading_screen::loading_screen(std::function<void()> f)
	: load_func_(f)
	, worker_result_()
	, cursor_setter_()
	, progress_stage_label_(nullptr)
	, animation_(nullptr)
	, animation_start_()
	, current_stage_(loading_stage::none)
	, visible_stages_()
	, current_visible_stage_()
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

	if(load_func_) {
		// Run the load function in its own thread.
		try {
			worker_result_ = std::async(std::launch::async, load_func_);
		} catch(const std::system_error& e) {
			ERR_LS << "Failed to create worker thread: " << e.what() << "\n";
			throw;
		}
	}

	progress_stage_label_ = find_widget<label>(&window, "status", false, true);
	animation_ = find_widget<drawing>(&window, "animation", false, true);

	// Add a draw callback to handle the animation, et al.
	window.connect_signal<event::DRAW>(
		std::bind(&loading_screen::draw_callback, this), event::dispatcher::front_child);
}

void loading_screen::post_show(window& /*window*/)
{
	cursor_setter_.reset();
}

void loading_screen::progress(loading_stage stage)
{
	if(singleton_ && stage != loading_stage::none) {
		singleton_->current_stage_.store(stage, std::memory_order_release);
	}
}

void loading_screen::process(events::pump_info&)
{
	using namespace std::chrono_literals;

	if(!load_func_ || worker_result_.wait_for(0ms) == std::future_status::ready) {
		// The worker returns void, so this is only to handle any exceptions thrown from the worker.
		// worker_result_.valid() will return false after.
		if(worker_result_.valid()) {
			worker_result_.get();
		}

		get_window()->close();
	}
}

void loading_screen::draw_callback()
{
	loading_stage stage = current_stage_.load(std::memory_order_acquire);

	if(stage != loading_stage::none && (current_visible_stage_ == visible_stages_.end() || stage != current_visible_stage_->first)) {
		auto iter = visible_stages_.find(stage);
		if(iter == visible_stages_.end()) {
			WRN_LS << "Stage missing description." << std::endl;
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
	animation_->set_is_dirty(true);
}

loading_screen::~loading_screen()
{
	/* If the worker thread is running, exit the application to prevent memory corruption.
	 * TODO: this is still not optimal. The main problem is that this code assumes that this
	 * happened because the window was closed, which is not necessarily the case (other
	 * possibilities might be a 'dialog doesn't fit on screen' exception caused by resizing
	 * the window).
	 */
	if(worker_result_.valid()) {
#if defined(_LIBCPP_VERSION) || defined(__MINGW32__)
		std::_Exit(0);
#else
		std::quick_exit(0);
#endif
	}

	singleton_ = nullptr;
}

void loading_screen::display(std::function<void()> f)
{
	if(singleton_ || CVideo::get_singleton().faked()) {
		f();
	} else {
		loading_screen(f).show();
	}
}

} // namespace dialogs
