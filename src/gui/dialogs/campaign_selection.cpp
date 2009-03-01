/* $Id$ */
/*
   Copyright (C) 2009 by Mark de Wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/campaign_selection.hpp"

#include "gui/widgets/listbox.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

namespace gui2 {

twindow* tcampaign_selection::build_window(CVideo& video)
{
	return build(video, get_id(CAMPAIGN_SELECTION));
}

void tcampaign_selection::pre_show(CVideo& /*video*/, twindow& window)
{
	tlistbox* list = dynamic_cast<tlistbox*>(
			window.find_widget("campaign_list", false));
	VALIDATE(list, missing_widget("campaign_list"));

	for(config::child_list::const_iterator itor = begin_;
			itor != end_; ++itor) {

		string_map item;
		item.insert(std::make_pair("icon", (**itor)["icon"]));
		item.insert(std::make_pair("label", (**itor)["name"]));
		list->add_row(item);
	}

}

void tcampaign_selection::post_show(twindow& window)
{
		tlistbox* list = dynamic_cast<tlistbox*>(
				window.find_widget("campaign_list", false));
		assert(list);

		choice_ = list->get_selected_row();
}

} // namespace gui2
