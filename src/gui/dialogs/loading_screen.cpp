/*
   Copyright (C) 2016 - 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "preferences/general.hpp"
#include "utils/functional.hpp"
#include "video.hpp"

#include <boost/thread.hpp>

#include <cstdlib>

#if defined(_MSC_VER) && _MSC_VER <= 1800
#include <Windows.h>
#endif

static lg::log_domain log_loadscreen("loadscreen");
#define ERR_LS LOG_STREAM(err, log_loadscreen)
#define WRN_LS LOG_STREAM(warn, log_loadscreen)
#define LOG_LS LOG_STREAM(info, log_loadscreen)
#define DBG_LS LOG_STREAM(debug, log_loadscreen)

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

namespace gui2
{
namespace dialogs
{
REGISTER_DIALOG(loading_screen)

loading_screen::loading_screen(std::function<void()> f)
	: timer_id_(0)
	, animation_counter_(0)
	, work_(f)
	, worker_()
	, cursor_setter_()
	, progress_stage_label_(nullptr)
	, animation_label_(nullptr)
	, current_stage_(loading_stage::none)
	, visible_stages_()
	, animation_stages_()
	, current_visible_stage_()
	, is_worker_running_(false)
{
	for(const auto& pair : stage_names) {
		visible_stages_[pair.first] = t_string(pair.second, "wesnoth-lib") + "...";
	}

	for(int i = 0; i != 20; ++i) {
		std::string s(20, ' ');
		s[i] = '.';
		animation_stages_.push_back(s);
	}

	current_visible_stage_ = visible_stages_.end();
	current_load = this;
}

void loading_screen::pre_show(window& window)
{
	if(work_) {
		worker_.reset(new boost::thread([this]() {
			is_worker_running_ = true;

			try {
				work_();
			} catch(...) {
				// TODO: guard this with a mutex.
				exception_ = std::current_exception();
			}

			is_worker_running_ = false;
		}));
	}

	timer_id_ = add_timer(100, std::bind(&loading_screen::timer_callback, this, std::ref(window)), true);
	cursor_setter_.reset(new cursor::setter(cursor::WAIT));
	progress_stage_label_ = find_widget<label>(&window, "status", false, true);
	animation_label_ = find_widget<label>(&window, "test_animation", false, true);

	window.set_enter_disabled(true);
	window.set_escape_disabled(true);
}

void loading_screen::post_show(window& /*window*/)
{
	worker_.reset();
	clear_timer();
	cursor_setter_.reset();
}

void loading_screen::progress(loading_stage stage)
{
	if(!current_load) {
		return;
	}

	if(stage != loading_stage::none) {
		current_load->current_stage_.store(stage, std::memory_order_release);
	}
}

loading_screen* loading_screen::current_load = nullptr;

void loading_screen::timer_callback(window& window)
{
	if(!work_ || !worker_ || worker_->timed_join(boost::posix_time::milliseconds(0))) {
		if(exception_) {
			clear_timer();
			std::rethrow_exception(exception_);
		}

		window.close();
	}

	if(!work_) {
		return;
	}

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

	++animation_counter_;
	if(animation_counter_ % 2 == 0) {
		animation_label_->set_label(animation_stages_[(animation_counter_ / 2) % animation_stages_.size()]);
	}
}

loading_screen::~loading_screen()
{
	/* If the worker thread is running, exit the application to prevent memory corruption.
	 * TODO: this is still not optimal. The main problem is that this code assumes that this
	 * happened because the window was closed, which is not necessarily the case (other
	 * possibilities might be a 'dialog doesn't fit on screen' exception caused by resizing
	 * the window).
	 *
	 * Another approach might be to add exit points (boost::this_thread::interruption_point())
	 * to the worker functions (filesystem.cpp, config parsing code, etc.) and then use that
	 * to end the thread faster.
	 */
	if(is_worker_running_) {
#if defined(_MSC_VER) && _MSC_VER <= 1800
		HANDLE process = GetCurrentProcess();
		TerminateProcess(process, 0u);
#elif defined(_LIBCPP_VERSION) || defined(__MINGW32__)
		std::_Exit(0);
#else
		std::quick_exit(0);
#endif
	}

	clear_timer();
	current_load = nullptr;
}

void loading_screen::display(CVideo& video, std::function<void()> f)
{
	const bool use_loadingscreen_animation = !preferences::disable_loadingscreen_animation();

	if(current_load || video.faked()) {
		f();
	} else if(use_loadingscreen_animation) {
		loading_screen(f).show();
	} else {
		loading_screen(std::function<void()>()).show();
		f();
	}
}

void loading_screen::clear_timer()
{
	if(timer_id_ != 0) {
		remove_timer(timer_id_);
		timer_id_ = 0;
	}
}

} // namespace dialogs
} // namespace gui2
