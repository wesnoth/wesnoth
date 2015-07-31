/*
   Copyright (C) 2010 - 2015 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "gui/auxiliary/find_widget.tpp"
#include "gui/auxiliary/old_markup.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "utils/foreach.tpp"

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
		const std::vector<std::pair<std::string, bool> >& items)
	: index_(-1), items_()
{
	FOREACH(const AUTO & p, items)
	{
		items_.push_back(std::make_pair(tlegacy_menu_item(p.first), p.second));
	}
}

void tcampaign_difficulty::pre_show(CVideo& /*video*/, twindow& window)
{
	tlistbox& list = find_widget<tlistbox>(&window, "listbox", false);
	window.keyboard_capture(&list);

	std::map<std::string, string_map> data;

	FOREACH(const AUTO & item, items_)
	{
		if(item.first.is_default()) {
			index_ = list.get_item_count();
		}

		data["icon"]["label"] = item.first.icon();
		data["label"]["label"] = item.first.label();
		data["label"]["use_markup"] = "true";
		data["description"]["label"] = item.first.description();
		data["description"]["use_markup"] = "true";

		list.add_row(data);

		tgrid* grid = list.get_row_grid(list.get_item_count() - 1);
		assert(grid);

		twidget *widget = grid->find("victory", false);
		if (widget && !item.second) {
			widget->set_visible(twidget::tvisible::hidden);
		}
	}

	if(index_ != -1) {
		list.select_row(index_);
	}
}

void tcampaign_difficulty::post_show(twindow& window)
{
	if(get_retval() != twindow::OK) {
		index_ = -1;
		return;
	}

	tlistbox& list = find_widget<tlistbox>(&window, "listbox", false);
	index_ = list.get_selected_row();
}
}
