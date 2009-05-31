/* $Id$ */
/*
   Copyright (C) 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/campaign_selection.hpp"

#include "foreach.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

namespace gui2 {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_campaign_selection
 *
 * == Campaign selection ==
 *
 * This shows the dialog choose which campaign the user wants to play. This
 * dialog is under construction and only used with --new-widgets.
 */

void tcampaign_selection::campaign_selected(twindow& window)
{
	tlistbox* list = dynamic_cast<tlistbox*>(
			window.find_widget("campaign_list", false));
	VALIDATE(list, missing_widget("campaign_list"));


	tlistbox* page = dynamic_cast<tlistbox*>(
			window.find_widget("campaign_details", false));
	VALIDATE(page, missing_widget("campaign_details"));

	page->select_row(list->get_selected_row());
}

twindow* tcampaign_selection::build_window(CVideo& video)
{
	return build(video, get_id(CAMPAIGN_SELECTION));
}

void tcampaign_selection::pre_show(CVideo& /*video*/, twindow& window)
{
	tlistbox* list = dynamic_cast<tlistbox*>(
			window.find_widget("campaign_list", false));
	VALIDATE(list, missing_widget("campaign_list"));

	list->set_callback_value_change(dialog_callback
			<tcampaign_selection, &tcampaign_selection::campaign_selected>);

	tlistbox* page = dynamic_cast<tlistbox*>(
			window.find_widget("campaign_details", false));
	VALIDATE(page, missing_widget("campaign_details"));

	foreach (const config &c, campaigns_) {

		/*** Add list item ***/
		string_map list_item;

		list_item.insert(std::make_pair("icon", c["icon"]));
		list_item.insert(std::make_pair("label", c["name"]));

		list->add_row(list_item);

		/*** Add detail item ***/
		string_map detail_item;
		std::map<std::string, string_map> detail_page;

		detail_item["label"] = c["description"];
		detail_page.insert(std::make_pair("description", detail_item));

		detail_item["label"] = c["image"];
		detail_page.insert(std::make_pair("image", detail_item));

		page->add_row(detail_page);
	}

	campaign_selected(window);
}

void tcampaign_selection::post_show(twindow& window)
{
		tlistbox* list = dynamic_cast<tlistbox*>(
				window.find_widget("campaign_list", false));
		assert(list);

		choice_ = list->get_selected_row();
}

} // namespace gui2
