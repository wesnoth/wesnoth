/* $Id$ */
/*
   Copyright (C) 2011 Sergey Popov <loonycyborg@gmail.com>
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

#include "gui/dialogs/network_transmission.hpp"

#include "foreach.hpp"
#include "gettext.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/progress_bar.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"

namespace gui2 {

REGISTER_DIALOG(network_transmission)

void tnetwork_transmission::pump_monitor::process(events::pump_info&)
{
	connection_.poll();
	if(connection_.connected() && window_) {
		window_.get().set_retval(twindow::OK);
	}
}

void tnetwork_transmission::pre_show(CVideo& /*video*/, twindow& window)
{
	// ***** ***** ***** ***** Set up the widgets ***** ***** ***** *****
	if(!title_.empty()) {
		find_widget<tlabel>(&window, "title", false).set_label(title_);
	}

	pump_monitor.window_ = window;

}

void tnetwork_transmission::post_show(twindow& /*window*/)
{
	pump_monitor.window_.reset();
}

} // namespace gui2

