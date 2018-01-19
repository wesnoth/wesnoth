/*
   Copyright (C) 2011 - 2018 Sergey Popov <loonycyborg@gmail.com>
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

#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/progress_bar.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "wesnothd_connection.hpp"

namespace gui2
{
namespace dialogs
{

REGISTER_DIALOG(network_transmission)

void network_transmission::pump_monitor::process(events::pump_info&)
{
	if(!window_)
		return;
	connection_->poll();
	if(connection_->finished()) {
		window_.get().set_retval(window::OK);
	} else {
		size_t completed, total;
			completed = connection_->current();
			total = connection_->total();
		if(total) {
			find_widget<progress_bar>(&(window_.get()), "progress", false)
					.set_percentage((completed * 100.) / total);

			std::stringstream ss;
			ss << utils::si_string(completed, true, _("unit_byte^B")) << "/"
			   << utils::si_string(total, true, _("unit_byte^B"));

			find_widget<label>(&(window_.get()), "numeric_progress", false)
					.set_label(ss.str());
			window_->invalidate_layout();
		}
	}
}

network_transmission::network_transmission(
		connection_data& connection,
		const std::string& title,
		const std::string& subtitle)
	: connection_(&connection)
	, pump_monitor_(connection_)
	, subtitle_(subtitle)
{
	register_label("title", true, title, false);
	set_restore(true);
}

void network_transmission::pre_show(window& window)
{
	// ***** ***** ***** ***** Set up the widgets ***** ***** ***** *****
	if(!subtitle_.empty()) {
		label& subtitle_label
				= find_widget<label>(&window, "subtitle", false);
		subtitle_label.set_label(subtitle_);
		subtitle_label.set_use_markup(true);
	}

	pump_monitor_.window_ = window;
}

void network_transmission::post_show(window& /*window*/)
{
	pump_monitor_.window_.reset();

	if(get_retval() == window::retval::CANCEL) {
		connection_->cancel();
	}
}

} // namespace dialogs
} // namespace gui2
