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
#include "gui/widgets/window.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/core/timer.hpp"
#include "gui/auxiliary/find_widget.hpp"

#include "video.hpp"
#include "cursor.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include <boost/bind.hpp>
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

tloadscreen::tloadscreen(boost::function<void()> f)
	: window_(nullptr)
	, timer_id_(0)
	, animation_counter_(0)
	, work_(f)
	, worker_()
	, cursor_setter_()
	, current_stage_()
	, current_visible_stage_()
{
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
	worker_.reset(new boost::thread(work_));
	timer_id_ = add_timer(100, boost::bind(&tloadscreen::timer_callback, this, boost::ref(window)), true);
	cursor_setter_.reset(new cursor::setter(cursor::WAIT));
	progress_stage_label_ = &find_widget<tlabel>(&window, "status", false);
	animation_label_ = &find_widget<tlabel>(&window, "test_animation", false);
	
}

void tloadscreen::post_show(twindow& /*window*/)
{
	worker_.reset();
	remove_timer(timer_id_);
	cursor_setter_.reset();
}

void tloadscreen::progress(const char* stage)
{
	if(!current_load) {
		return;
	}
	if(stage) {
		auto iter = stages.find(stage);
		if(iter == stages.end()) {
			WRN_LS << "Stage ID '" << stage << "' missing description." << std::endl;
			return;
		}
		current_load->current_stage_ = iter;
	}
}

tloadscreen* tloadscreen::current_load = nullptr;

void tloadscreen::timer_callback(twindow& window)
{
	if (!worker_ || worker_->timed_join(boost::posix_time::milliseconds(0))) {
		window.close();
	}
	if (current_stage_ != current_visible_stage_)
	{
		current_visible_stage_ = current_stage_;
		progress_stage_label_->set_label(t_string(current_stage_->second, "wesnoth-lib") + "...");
	}
	++animation_counter_;
	if (animation_counter_ % 2 == 0) {
		int animation_state = (animation_counter_ / 2) % 20;
		std::string s(20, ' ');
		s[animation_state] = '.';
		animation_label_->set_label(s);
	}
}

tloadscreen::~tloadscreen()
{
	close();
	current_load = nullptr;
}

void tloadscreen::display(CVideo& video, boost::function<void()> f)
{
	if (current_load || video.faked()) {
		f();
	}
	else {
		tloadscreen(f).show(video);
	}
}

} // namespace gui2
