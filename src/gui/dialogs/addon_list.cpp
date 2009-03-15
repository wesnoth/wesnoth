/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Mark de Wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/addon_list.hpp"

#include "gui/widgets/button.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/widget.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/window_builder.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "wml_exception.hpp"

namespace gui2 {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_addon_list
 *
 * == Addon list ==
 *
 * This shows the dialog with the addons to install. This dialog is under
 * construction and only used with --new-widgets.
 */

twindow* taddon_list::build_window(CVideo& video)
{
	return build(video, get_id(ADDON_LIST));
}

void taddon_list::pre_show(CVideo& /*video*/, twindow& window)
{
	tlistbox* list =
		dynamic_cast<tlistbox*>(window.find_widget("addon_list", false));
	VALIDATE(list, missing_widget("addon_list"));

	const config::child_list& cmps = cfg_.get_children("campaign");


	for(config::child_list::const_iterator itor = cmps.begin();
			itor != cmps.end(); ++itor) {

		std::map<std::string, std::map<std::string, t_string> > data;
		std::map<std::string, t_string> item;

		std::string tmp = (**itor)["name"];
		utils::truncate_as_wstring(tmp, 20);
		item["label"] = tmp;
		data.insert(std::make_pair("name", item));

		tmp = (**itor)["version"];
		utils::truncate_as_wstring(tmp, 12);
		item["label"] = tmp;
		data.insert(std::make_pair("version", item));

		tmp = (**itor)["author"];
		utils::truncate_as_wstring(tmp, 16);
		item["label"] = tmp;
		data.insert(std::make_pair("author", item));

		item["label"] = (**itor)["downloads"];
		data.insert(std::make_pair("downloads", item));

		item["label"] = (**itor)["size"];
		data.insert(std::make_pair("size", item));

		list->add_row(data);

	}
}

} // namespace gui2

