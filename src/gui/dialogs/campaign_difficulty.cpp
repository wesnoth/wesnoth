/*
   Copyright (C) 2010 - 2017 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "config.hpp"
#include "preferences/game.hpp"
#include "formula/string_utils.hpp"

#include "gui/dialogs/campaign_difficulty.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/auxiliary/old_markup.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

#include "log.hpp"

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
	if(result.empty()) {
		WRN_WML << "[campaign] difficulties,difficulty_descriptions= is deprecated. Use [difficulty] instead" << std::endl;

		std::vector<std::string> difficulty_list = utils::split(source["difficulties"]);
		std::vector<std::string> difficulty_opts = utils::split(source["difficulty_descriptions"], ';');

		if(difficulty_opts.size() != difficulty_list.size()) {
			difficulty_opts = difficulty_list;
		}

		for(std::size_t i = 0; i < difficulty_opts.size(); i++)
		{
			config temp;
			gui2::legacy_menu_item parsed(difficulty_opts[i]);

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
	: difficulties_()
	, campaign_id_(campaign["id"])
	, selected_difficulty_()
{
	set_restore(true);

	difficulties_ = generate_difficulty_config(campaign);
}

void campaign_difficulty::pre_show(window& window)
{
	listbox& list = find_widget<listbox>(&window, "listbox", false);
	window.keyboard_capture(&list);

	std::map<std::string, string_map> data;

	for (const config &d : difficulties_.child_range("difficulty"))
	{
		data["icon"]["label"] = d["image"];
		data["label"]["label"] = d["label"];
		data["label"]["use_markup"] = "true";
		data["description"]["label"] = d["old_markup"].to_bool() || d["description"].empty() ? d["description"]
			: std::string("(") + d["description"] + std::string(")");
		data["description"]["use_markup"] = "true";

		list.add_row(data);

		const int this_row = list.get_item_count() - 1;

		if(d["default"].to_bool(false)) {
			list.select_row(this_row);
		}

		grid* grid = list.get_row_grid(this_row);
		assert(grid);

		widget *widget = grid->find("victory", false);
		if (widget && !preferences::is_campaign_completed(campaign_id_, d["define"])) {
			widget->set_visible(widget::visibility::hidden);
		}
	}
}

void campaign_difficulty::post_show(window& window)
{
	if(get_retval() != window::OK) {
		selected_difficulty_ = "CANCEL";
		return;
	}

	listbox& list = find_widget<listbox>(&window, "listbox", false);
	selected_difficulty_ = difficulties_.child("difficulty", list.get_selected_row())["define"].str();
}
} // namespace dialogs
} // namespace gui2
