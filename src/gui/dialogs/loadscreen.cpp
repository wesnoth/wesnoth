/*
   Copyright (C) 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

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

#include "cursor.hpp"
#include "gui/dialogs/loadscreen.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/core/timer.hpp"
#include "gui/auxiliary/find_widget.hpp"

#include "video.hpp"
#include "cursor.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "preferences.hpp"
#include "utils/functional.hpp"
#include <boost/thread.hpp>

static lg::log_domain log_loadscreen("loadscreen");
#define ERR_LS LOG_STREAM(err, log_loadscreen)
#define WRN_LS LOG_STREAM(warn, log_loadscreen)
#define LOG_LS LOG_STREAM(info, log_loadscreen)
#define DBG_LS LOG_STREAM(debug, log_loadscreen)

static const std::map<std::string, std::string> stages =
{
	{ "build terrain", N_("Building terrain rules") },
	{ "create cache", N_("Reading files and creating cache") },
	{ "init display", N_("Initializing display") },
	{ "init fonts", N_("Reinitialize fonts for the current language") },
	{ "init teams", N_("Initializing teams") },
	{ "init theme", N_("Initializing display") },
	{ "load config", N_("Loading game configuration") },
	{ "load data", N_("Loading data files") },
	{ "load level", N_("Loading level") },
	{ "init lua", N_("Initializing scripting engine") },
	{ "init whiteboard", N_("Initializing planning mode") },
	{ "load unit types", N_("Reading unit files") },
	{ "load units", N_("Loading units") },
	{ "refresh addons", N_("Searching for installed add-ons") },
	{ "start game", N_("Starting game") },
	{ "verify cache", N_("Verifying cache") },
};

namespace gui2
{

REGISTER_DIALOG(loadscreen)

tloadscreen::tloadscreen(std::function<void()> f)
	: window_(nullptr)
	, timer_id_(0)
	, animation_counter_(0)
	, work_(f)
	, worker_()
	, cursor_setter_()
	, progress_stage_label_(nullptr)
	, animation_label_(nullptr)
	, current_stage_(nullptr)
	, visible_stages_()
	, animation_stages_()
	, current_visible_stage_()
{
	for (const auto& pair : stages) {
		visible_stages_[pair.first] = t_string(pair.second, "wesnoth-lib") + "...";
	}
	for (int i = 0; i != 20; ++i) {
		std::string s(20, ' ');
		s[i] = '.';
		animation_stages_.push_back(s);
	}
	current_visible_stage_ = visible_stages_.end();
	current_load = this;
}

void tloadscreen::close()
{
	if(window_) {
		window_->undraw();
		delete window_;
		window_ = nullptr;
	}
}

twindow* tloadscreen::build_window(CVideo& video) const
{
	return build(video, window_id());
}

void tloadscreen::pre_show(twindow& window)
{
	if (work_) {
		worker_.reset(new boost::thread([this]() {
			try {
				work_();
			}
			catch (const game::error&) {
				//TODO: guard this with a mutex.
				exception_ = std::current_exception();
			}
		}));
	}
	timer_id_ = add_timer(100, std::bind(&tloadscreen::timer_callback, this, std::ref(window)), true);
	cursor_setter_.reset(new cursor::setter(cursor::WAIT));
	progress_stage_label_ = &find_widget<tlabel>(&window, "status", false);
	animation_label_ = &find_widget<tlabel>(&window, "test_animation", false);
	
	window.set_enter_disabled(true);
	window.set_escape_disabled(true);
}

void tloadscreen::post_show(twindow& /*window*/)
{
	worker_.reset();
	clear_timer();
	cursor_setter_.reset();
}

void tloadscreen::progress(const char* stage)
{
	if(!current_load) {
		return;
	}
	if(stage) {
		current_load->current_stage_
#if defined(_MSC_VER) && _MSC_VER < 1900
			= stage;
#else
			.store(stage, std::memory_order_release);
#endif
	}
}

tloadscreen* tloadscreen::current_load = nullptr;

void tloadscreen::timer_callback(twindow& window)
{
	if (!work_ || !worker_ || worker_->timed_join(boost::posix_time::milliseconds(0))) {
		if (exception_) {
			clear_timer();
			std::rethrow_exception(exception_);
		}
		window.close();
	}
	if (!work_) {
		return;
	}
	const char* stage = current_stage_
#if defined(_MSC_VER) && _MSC_VER < 1900
		;
#else
		.load(std::memory_order_acquire);
#endif
	if (stage && (current_visible_stage_ == visible_stages_.end() || stage != current_visible_stage_->first))
	{
		auto iter = visible_stages_.find(stage);
		if(iter == visible_stages_.end()) {
			WRN_LS << "Stage ID '" << stage << "' missing description." << std::endl;
			return;
		}
		current_visible_stage_ = iter;
		progress_stage_label_->set_label(iter->second);
	}
	++animation_counter_;
	if (animation_counter_ % 2 == 0) {
		animation_label_->set_label(animation_stages_[(animation_counter_ / 2) % animation_stages_.size()]);
	}
}

tloadscreen::~tloadscreen()
{
	clear_timer();
	close();
	current_load = nullptr;
}

void tloadscreen::display(CVideo& video, std::function<void()> f)
{
	const bool use_loadingscreen_animation = !preferences::disable_loadingscreen_animation();
	if (current_load || video.faked()) {
		f();
	}
	else if(use_loadingscreen_animation) {
		tloadscreen(f).show(video);
	}
	else {
		tloadscreen(std::function<void()>()).show(video);
		f();
	}
}
void tloadscreen::clear_timer()
{
	if (timer_id_ != 0) {
		remove_timer(timer_id_);
		timer_id_ = 0;
	}
}

} // namespace gui2
