/*
	Copyright (C) 2011 - 2025
	by Sergey Popov <loonycyborg@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "gettext.hpp"
#include "gui/widgets/progress_bar.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/window.hpp"
#include "serialization/string_utils.hpp"

namespace gui2::dialogs
{
REGISTER_DIALOG(network_transmission)

void network_transmission::pump_monitor::process()
{
	if(!window_) {
		return;
	}

	connection_->poll();

	std::size_t completed = connection_->current();
	std::size_t total = connection_->total();

	if(total) {
		window_->find_widget<progress_bar>("progress")
			.set_percentage(std::round((completed * 100.) / total));

		auto ss = formatter()
			<< utils::si_string(completed, true, _("unit_byte^B")) << "/"
			<< utils::si_string(total, true, _("unit_byte^B"));

		window_->find_widget<label>("numeric_progress")
			.set_label(ss.str());
	}

	if(connection_->finished()) {
		window_->set_retval(retval::OK);
	}
}

network_transmission::network_transmission(
		connection_data& connection,
		const std::string& title,
		const std::string& subtitle)
	: modal_dialog(window_id())
	, connection_(&connection)
	, pump_monitor_(connection_)
	, subtitle_(subtitle)
{
	register_label("title", true, title, false);
}

void network_transmission::pre_show()
{
	// ***** ***** ***** ***** Set up the widgets ***** ***** ***** *****
	if(!subtitle_.empty()) {
		label& subtitle_label = find_widget<label>("subtitle");
		subtitle_label.set_label(subtitle_);
		subtitle_label.set_use_markup(true);
	}

	// NOTE: needed to avoid explicit calls to invalidate_layout()
	// in network_transmission::pump_monitor::process()
	find_widget<label>("numeric_progress").set_label(" ");
	pump_monitor_.window_ = this;
}

void network_transmission::post_show()
{
	// In case we close the dialog before transmission is complete
	pump_monitor_.window_ = nullptr;

	if(get_retval() == retval::CANCEL) {
		connection_->cancel();
	}
}

} // namespace dialogs
