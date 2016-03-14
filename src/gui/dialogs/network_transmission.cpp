/*
   Copyright (C) 2011 - 2016 Sergey Popov <loonycyborg@gmail.com>
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

#include "formula_string_utils.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/find_widget.tpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/progress_bar.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"

namespace gui2
{

REGISTER_DIALOG(network_transmission)

void tnetwork_transmission::pump_monitor::process(events::pump_info&)
{
	connection_.poll();
	if(!window_)
		return;
	if(connection_.done()) {
		window_.get().set_retval(twindow::OK);
	} else {
		size_t completed, total;
		if(track_upload_) {
			completed = connection_.bytes_written();
			total = connection_.bytes_to_write();
		} else {
			completed = connection_.bytes_read();
			total = connection_.bytes_to_read();
		}
		if(total) {
			find_widget<tprogress_bar>(&(window_.get()), "progress", false)
					.set_percentage((completed * 100.) / total);

			std::stringstream ss;
			ss << utils::si_string(completed, true, _("unit_byte^B")) << "/"
			   << utils::si_string(total, true, _("unit_byte^B"));

			find_widget<tlabel>(&(window_.get()), "numeric_progress", false)
					.set_label(ss.str());
			window_->invalidate_layout();
		}
	}
}

tnetwork_transmission::tnetwork_transmission(
		network_asio::connection& connection,
		const std::string& title,
		const std::string& subtitle)
	: connection_(connection)
	, track_upload_(false)
	, pump_monitor_(connection, track_upload_)
	, subtitle_(subtitle)
{
	register_label("title", true, title, false);
	set_restore(true);
}

void tnetwork_transmission::set_subtitle(const std::string& subtitle)
{
	subtitle_ = subtitle;
}

void tnetwork_transmission::pre_show(twindow& window)
{
	// ***** ***** ***** ***** Set up the widgets ***** ***** ***** *****
	if(!subtitle_.empty()) {
		tlabel& subtitle_label
				= find_widget<tlabel>(&window, "subtitle", false);
		subtitle_label.set_label(subtitle_);
		subtitle_label.set_use_markup(true);
	}

	pump_monitor_.window_ = window;
}

void tnetwork_transmission::post_show(twindow& /*window*/)
{
	pump_monitor_.window_.reset();
	connection_.cancel();
}

} // namespace gui2
