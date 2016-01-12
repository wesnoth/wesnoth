/*
   Copyright (C) 2015 by the Battle for Wesnoth Project
   <http://www.wesnoth.org/>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "quit_confirmation.hpp"
#include "gettext.hpp"
#include "display.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/window.hpp"
#include "resources.hpp"

int quit_confirmation::count_ = 0;
bool quit_confirmation::open_ = false;

void quit_confirmation::quit()
{
	if(count_ != 0 && !open_)
	{
		quit(CVideo::get_singleton());
	}
	else
	{
		throw CVideo::quit();
	}
}

void quit_confirmation::quit(CVideo& video)
{
	assert(!open_);
	open_ = true;
	const int res = gui2::show_message(video, _("Quit"),
		_("Do you really want to quit?"), gui2::tmessage::yes_no_buttons);
	open_ = false;
	if(res != gui2::twindow::CANCEL) {
		throw CVideo::quit();
	} else {
		return;
	}
}
