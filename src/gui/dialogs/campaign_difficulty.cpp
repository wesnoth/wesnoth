/*
   Copyright (C) 2010 - 2016 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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
#include "game_preferences.hpp"
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
#include "utils/foreach.hpp"

#include "log.hpp"

static lg::log_domain log_wml("wml");
#define WRN_WML LOG_STREAM(warn, log_wml)

namespace gui2
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
 * -icon & & control & m &
 *         Widget which shows a listbox item icon, first item markup column. $
 *
 * -label & & control & m &
 *         Widget which shows a listbox item label, second item markup column. $
 *
 * -description & & control & m &
 *         Widget which shows a listbox item description, third item markup
 *         column. $
 *
 * @end{table}
 */

REGISTER_DIALOG(campaign_difficulty)

tcampaign_difficulty::tcampaign_difficulty(
		const config& campaign)
	: difficulties_()
	, campaign_id_(campaign["id"])
	, selected_difficulty_()
{
	set_restore(true);

	// Populate local config with difficulty children
	difficulties_.append_children(campaign, "difficulty");

	// Convert legacy format to new-style config if latter not present
	if(difficulties_.empty()) {
		WRN_WML << "[campaign] difficulties,difficulty_descriptions= is deprecated. Use [difficulty] instead" << std::endl;

		std::vector<std::string> difficulty_list = utils::split(campaign["difficulties"]);
		std::vector<std::string> difficulty_opts = utils::split(campaign["difficulty_descriptions"], ';');

		if(difficulty_opts.size() != difficulty_list.size()) {
			difficulty_opts = difficulty_list;
		}

		for(std::size_t i = 0; i < difficulty_opts.size(); i++)
		{
			config temp;
			gui2::tlegacy_menu_item parsed(difficulty_opts[i]);

			temp["define"] = difficulty_list[i];
			temp["image"] = parsed.icon();
			temp["label"] = parsed.label();
			temp["description"] = parsed.description();
			temp["default"] = parsed.is_default();
			temp["old_markup"] = true; // To prevent double parentheses in the dialog

			difficulties_.add_child("difficulty", temp);
		}
	}
}

void tcampaign_difficulty::pre_show(twindow& window)
{
	tlistbox& list = find_widget<tlistbox>(&window, "listbox", false);
	window.keyboard_capture(&list);

	std::map<std::string, string_map> data;

	BOOST_FOREACH(const config &d, difficulties_.child_range("difficulty"))
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

		tgrid* grid = list.get_row_grid(this_row);
		assert(grid);

		twidget *widget = grid->find("victory", false);
		if (widget && !preferences::is_campaign_completed(campaign_id_, d["define"])) {
			widget->set_visible(twidget::tvisible::hidden);
		}
	}
}

void tcampaign_difficulty::post_show(twindow& window)
{
	if(get_retval() != twindow::OK) {
		selected_difficulty_ = "CANCEL";
		return;
	}

	tlistbox& list = find_widget<tlistbox>(&window, "listbox", false);
	selected_difficulty_ = difficulties_.child("difficulty", list.get_selected_row())["define"].str();
}
}
