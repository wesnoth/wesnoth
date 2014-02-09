/*
   Copyright (C) 2011 - 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/popup.hpp"

#include "gui/widgets/window.hpp"
#include "video.hpp"

namespace gui2
{

tpopup::tpopup() : window_(NULL)
{
}

tpopup::~tpopup()
{
	hide();
}

void tpopup::show(CVideo& video,
				  const bool allow_interaction,
				  const unsigned /*auto_close_time*/)
{
	if(video.faked()) {
		return;
	}

	hide();

	window_ = build_window(video);

	post_build(video, *window_);

	pre_show(video, *window_);

	if(allow_interaction) {
		window_->show_non_modal();
	} else {
		window_->show_tooltip(/*auto_close_time*/);
	}
}

void tpopup::hide()
{
	if(window_) {
		window_->undraw();
		delete window_;
		window_ = NULL;
	}
}

twindow* tpopup::build_window(CVideo& video) const
{
	return build(video, window_id());
}

void tpopup::post_build(CVideo& /*video*/, twindow& /*window*/)
{
	/* DO NOTHING */
}

void tpopup::pre_show(CVideo& /*video*/, twindow& /*window*/)
{
	/* DO NOTHING */
}

} // namespace gui2
