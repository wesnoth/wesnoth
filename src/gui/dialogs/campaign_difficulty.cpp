/*
   Copyright (C) 2010 - 2018 by Iris Morelle <shadowm2006@gmail.com>
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

#include "gui/dialogs/campaign_difficulty.hpp"

#include "config.hpp"
#include "font/text_formatting.hpp"
#include "formatter.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "preferences/game.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"

static lg::log_domain log_wml("wml");
#define WRN_WML LOG_STREAM(warn, log_wml)

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_campaign_difficulty
 *
 * == Campaign difficulty ==
 *
 * The campaign mode difficulty menu.
 *
 * @begin{table}{dialog_widgets}
 *
 * title & & label & m &
 *         Dialog title label. $
 *
 * message & & scroll_label & o &
 *         Text label displaying a description or instructions. $
 *
 * listbox & & listbox & m &
 *         Listbox displaying user choices, defined by WML for each campaign. $
 *
 * -icon & & styled_widget & m &
 *         Widget which shows a listbox item icon, first item markup column. $
 *
 * -label & & styled_widget & m &
 *         Widget which shows a listbox item label, second item markup column. $
 *
 * -description & & styled_widget & m &
 *         Widget which shows a listbox item description, third item markup
 *         column. $
 *
 * @end{table}
 */

REGISTER_DIALOG(campaign_difficulty)

config generate_difficulty_config(const config& source)
{
	config result;

	// Populate local config with difficulty children
	result.append_children(source, "difficulty");
	return result;
}

campaign_difficulty::campaign_difficulty(const config& campaign)
	: difficulties_(generate_difficulty_config(campaign))
	, campaign_id_(campaign["id"])
	, selected_difficulty_("CANCEL")
{
}

void campaign_difficulty::pre_show(window& window)
{
	listbox& list = find_widget<listbox>(&window, "listbox", false);
	window.keyboard_capture(&list);

	for(const config& d : difficulties_.child_range("difficulty")) {
		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = d["image"];
		data.emplace("icon", item);

		item["use_markup"] = "true";

		std::ostringstream ss;
		ss << d["label"];

		if(!d["description"].empty()) {
			ss << "\n<small>" << font::span_color(font::GRAY_COLOR) << "(" << d["description"].str() << ")</span></small>";
		}

		item["label"] = ss.str();
		data.emplace("label", item);

		grid& grid = list.add_row(data);

		if(d["default"].to_bool(false)) {
			list.select_last_row();
		}

		widget* widget = grid.find("victory", false);
		if(widget && !preferences::is_campaign_completed(campaign_id_, d["define"])) {
			widget->set_visible(widget::visibility::hidden);
		}
	}
}

void campaign_difficulty::post_show(window& window)
{
	if(get_retval() == retval::OK) {
		listbox& list = find_widget<listbox>(&window, "listbox", false);
		selected_difficulty_ = difficulties_.child("difficulty", list.get_selected_row())["define"].str();
	}
}
} // namespace dialogs
} // namespace gui2
