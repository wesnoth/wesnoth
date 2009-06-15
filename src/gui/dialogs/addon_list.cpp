/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/addon_list.hpp"

#include "foreach.hpp"
#include "gui/auxiliary/window_builder.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/widget.hpp"
#include "gui/widgets/window.hpp"
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

	foreach (const config &c, cfg_.child_range("campaign"))
	{
		std::map<std::string, string_map> data;
		string_map item;

		std::string tmp = c["name"];
		utils::truncate_as_wstring(tmp, 20);
		item["label"] = tmp;
		data.insert(std::make_pair("name", item));

		tmp = c["version"];
		utils::truncate_as_wstring(tmp, 12);
		item["label"] = tmp;
		data.insert(std::make_pair("version", item));

		tmp = c["author"];
		utils::truncate_as_wstring(tmp, 16);
		item["label"] = tmp;
		data.insert(std::make_pair("author", item));

		item["label"] = c["downloads"];
		data.insert(std::make_pair("downloads", item));

		item["label"] = c["size"];
		data.insert(std::make_pair("size", item));

		list->add_row(data);

	}
}

} // namespace gui2

