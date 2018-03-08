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
#include "formatter.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/auxiliary/old_markup.hpp"
#include "preferences/game.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "deprecation.hpp"

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

	// Convert legacy format to new-style config if latter not present
	if(result.empty() && source.has_attribute("difficulties")) {
		deprecated_message("[campaign]difficulties", DEP_LEVEL::FOR_REMOVAL, {1, 15, 0}, "Use [difficulty] instead.");
		if(source.has_attribute("difficulty_descriptions")) {
			deprecated_message("[campaign]difficulty_descriptions", DEP_LEVEL::FOR_REMOVAL, {1, 15, 0}, "Use [difficulty] instead.");
		}

		std::vector<std::string> difficulty_list = utils::split(source["difficulties"]);
		std::vector<std::string> difficulty_opts = utils::split(source["difficulty_descriptions"], ';');

		if(difficulty_opts.size() != difficulty_list.size()) {
			difficulty_opts = difficulty_list;
		}

		for(std::size_t i = 0; i < difficulty_opts.size(); ++i) {
			config temp;
			gui2::legacy_menu_item parsed(difficulty_opts[i], "Use [difficulty] instead.");

			temp["define"] = difficulty_list[i];
			temp["image"] = parsed.icon();
			temp["label"] = parsed.label();
			temp["description"] = parsed.description();
			temp["default"] = parsed.is_default();
			temp["old_markup"] = true; // To prevent double parentheses in the dialog

			result.add_child("difficulty", std::move(temp));
		}
	}

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

		item["label"] = d["label"];
		data.emplace("label", item);

		const std::string descrip_text = d["old_markup"].to_bool() || d["description"].empty()
			? d["description"]
			: (formatter() << "(" << d["description"].str() << ")").str();

		item["label"] = descrip_text;
		data.emplace("description", item);

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
	if(get_retval() == window::OK) {
		listbox& list = find_widget<listbox>(&window, "listbox", false);
		selected_difficulty_ = difficulties_.child("difficulty", list.get_selected_row())["define"].str();
	}
}
} // namespace dialogs
} // namespace gui2
