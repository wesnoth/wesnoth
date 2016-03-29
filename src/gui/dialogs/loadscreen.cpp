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
#include "video.hpp"

namespace gui2
{

REGISTER_DIALOG(loadscreen)

void tloadscreen::show(CVideo& video)
{
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

void tloadscreen::pre_show(twindow& /*window*/)
{
	// FIXME
	cursor::setter cur(cursor::WAIT);
}

void tloadscreen::post_show(twindow& /*window*/)
{
	cursor::setter cur(cursor::NORMAL);
}

} // namespace gui2
