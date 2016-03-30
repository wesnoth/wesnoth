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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "cursor.hpp"
#include "gui/dialogs/loadscreen.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/core/timer.hpp"
#include "gui/auxiliary/find_widget.hpp"

#include "video.hpp"
#include "cursor.hpp"
#include <boost/bind.hpp>
#include <boost/thread.hpp>

namespace gui2
{

REGISTER_DIALOG(loadscreen)

tloadscreen::tloadscreen(boost::function<void()> f)
	: window_(NULL)
	, timer_id_(0)
	, work_(f)
	, worker_()
	, cursor_setter_()
	, current_stage_(NULL)
	, current_visible_stage_(NULL)
{
	current_load = this;
}
void tloadscreen::show(CVideo& video)
{
	tdialog::show(video);
	return;
	if(video.faked()) {
		return;
	}

	window_ = build_window(video);

	pre_show(*window_);

	window_->show_non_modal();

	post_show(*window_);
}

void tloadscreen::close()
{
	if(window_) {
		window_->undraw();
		delete window_;
		window_ = NULL;
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
	// Currently this is a no-op stub
	if(stage) {
		current_load->current_stage_ = stage;
	}
}

tloadscreen* tloadscreen::current_load = NULL;

void tloadscreen::timer_callback(twindow& window)
{
	if (!worker_ || worker_->timed_join(boost::posix_time::milliseconds(0))) {
		window.close();
	}
	if (current_stage_ != current_visible_stage_)
	{
		current_visible_stage_ = current_stage_;
		progress_stage_label_->set_label(current_visible_stage_);

	}
}

tloadscreen::~tloadscreen()
{
	close();
	current_load = NULL;
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
