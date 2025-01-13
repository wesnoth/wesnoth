/*
	Copyright (C) 2008 - 2024
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

#include <functional>

#include "gui/dialogs/multiplayer/mp_report.hpp"

#include "gettext.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(mp_report)

// static t_string std::array causes a core dump on start
const std::array<std::string, 3> mp_report::occurrence_locations = {"Lobby", "Whisper", "Game"};

mp_report::mp_report(std::string& report_text)
	: modal_dialog(window_id())
	, report_text_(report_text)
	, reportee_empty_(true)
	, report_reason_empty_(true)
{
}

void mp_report::pre_show()
{
	std::vector<config> occurrence_location_entries;
	occurrence_location_entries.emplace_back("label", _("Lobby"));
	occurrence_location_entries.emplace_back("label", _("Whisper"));
	occurrence_location_entries.emplace_back("label", _("Game"));

	find_widget<menu_button>("occurrence_location").set_values(occurrence_location_entries);

	button& ok = find_widget<button>("ok");
	ok.set_active(false);

	text_box& reportee = find_widget<text_box>("reportee");
	reportee.on_modified([this](const auto& box) { reportee_changed(box.text()); });

	text_box& report_reason = find_widget<text_box>("report_reason");
	report_reason.on_modified([this](const auto& box) { report_reason_changed(box.text()); });

	set_exit_hook(window::exit_hook::ok_only, [this] { return !reportee_empty_ && !report_reason_empty_; });
}

void mp_report::post_show()
{
	if(get_retval() == gui2::retval::OK)
	{
		const text_box& reportee = find_widget<const text_box>("reportee");
		const text_box& report_reason = find_widget<const text_box>("report_reason");
		const menu_button& occurrence_location = find_widget<const menu_button>("occurrence_location");
		const std::string additional_information = find_widget<const text_box>("additional_information").get_value();

		std::ostringstream report;
		report << "Reporting player '" << reportee.get_value() << "' for reason '" << report_reason.get_value() << "'."
			   << " Location of occurrence is '" << occurrence_locations[occurrence_location.get_value()] << "'.";
		if(additional_information.size() > 0)
		{
			report << " Additional information provided: '" << additional_information << "'.";
		}
		report_text_ = report.str();
	}
}

void mp_report::reportee_changed(const std::string& text)
{
	reportee_empty_ = text.empty();

	button& ok = find_widget<button>("ok");
	ok.set_active(!reportee_empty_ && !report_reason_empty_);
}

void mp_report::report_reason_changed(const std::string& text)
{
	report_reason_empty_ = text.empty();

	button& ok = find_widget<button>("ok");
	ok.set_active(!reportee_empty_ && !report_reason_empty_);
}

} // namespace dialogs
