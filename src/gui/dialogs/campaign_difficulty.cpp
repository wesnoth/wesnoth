/*
	Copyright (C) 2010 - 2024
	by Iris Morelle <shadowm2006@gmail.com>
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

#include "gui/dialogs/campaign_difficulty.hpp"

#include "config.hpp"
#include "deprecation.hpp"
#include "serialization/markup.hpp"
#include "game_version.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "preferences/preferences.hpp"

static lg::log_domain log_wml("wml");
#define WRN_WML LOG_STREAM(warn, log_wml)

namespace gui2::dialogs
{

REGISTER_DIALOG(campaign_difficulty)

config generate_difficulty_config(const config& source)
{
	config result;

	// Populate local config with difficulty children
	result.append_children(source, "difficulty");

	// Issue deprecation warnings about the old difficulties syntax
	if(result.empty() && source.has_attribute("difficulties")) {
		deprecated_message("[campaign]difficulties", DEP_LEVEL::REMOVED, {1, 15, 0}, "Use [difficulty] instead.");
		if(source.has_attribute("difficulty_descriptions")) {
			deprecated_message("[campaign]difficulty_descriptions", DEP_LEVEL::REMOVED, {1, 15, 0}, "Use [difficulty] instead.");
		}
	}

	return result;
}

campaign_difficulty::campaign_difficulty(const config& campaign)
	: modal_dialog(window_id())
	, difficulties_(generate_difficulty_config(campaign))
	, campaign_id_(campaign["id"])
	, selected_difficulty_("CANCEL")
{
}

void campaign_difficulty::pre_show()
{
	listbox& list = find_widget<listbox>("listbox");
	keyboard_capture(&list);

	unsigned difficulty_count = 0;
	const unsigned difficulty_max = difficulties_.child_count("difficulty");
	for(const config& d : difficulties_.child_range("difficulty")) {
		widget_data data;
		widget_item item;

		item["label"] = d["image"];
		data.emplace("icon", item);

		item["use_markup"] = "true";

		std::ostringstream ss;
		ss << d["label"];

		if(!d["description"].empty()) {
			if (d["auto_markup"].to_bool(true) == false) {
				ss << "\n" << d["description"].str();
			} else if (!d["old_markup"].to_bool()) {
				ss << "\n" << markup::tag("small", markup::span_color(font::GRAY_COLOR, "(", d["description"], ")"));
			} else {
				ss << "\n" << markup::tag("small", markup::span_color(font::GRAY_COLOR, d["description"]));
			}
		}

		item["label"] = ss.str();
		data.emplace("label", item);

		grid& grid = list.add_row(data);

		if(d["default"].to_bool(false)) {
			list.select_last_row();
		}

		styled_widget& widget = grid.find_widget<styled_widget>("victory");
		if(prefs::get().is_campaign_completed(campaign_id_, d["define"])) {
			// Use different laurels according to the difficulty level, following the
			// pre-existing convention established in campaign_selection class.
			// Assumes ascending order of difficulty and gold laurel is set first
			// in case there is only one difficulty setting.
			if(difficulty_count + 1 >= difficulty_max) {
				widget.set_label(game_config::images::victory_laurel_hardest);
			} else if(difficulty_count == 0) {
				widget.set_label(game_config::images::victory_laurel_easy);
			} else {
				widget.set_label(game_config::images::victory_laurel);
			}
		} else {
			widget.set_visible(widget::visibility::hidden);
		}

		difficulty_count++;
	}
}

void campaign_difficulty::post_show()
{
	if(get_retval() == retval::OK) {
		listbox& list = find_widget<listbox>("listbox");
		selected_difficulty_ = difficulties_.mandatory_child("difficulty", list.get_selected_row())["define"].str();
	}
}
} // namespace dialogs
