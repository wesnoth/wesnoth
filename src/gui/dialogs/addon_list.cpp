/* $Id$ */
/*
   Copyright (C) 2008 - 2011 by Mark de Wever <koraq@xs4all.nl>
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
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

namespace gui2 {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_addon_list
 *
 * == Addon list ==
 *
 * This shows the dialog with the addons to install. This dialog is under
 * construction and only used with --new-widgets.
 *
 * @start_table = grid
 *     (addons) (listbox) ()      A listbox that will contain the info about
 *                                all addons on the server.
 *     -[name] (control) ()       The name of the addon.
 *     -[version] (control) ()    The version number of the addon.
 *     -[author] (control) ()     The author of the addon.
 *     -[downloads] (control) ()  The number of times the addon has been
 *                                downloaded.
 *     -[size] (control) ()       The size of the addon.
 * @end_table
 */

twindow* taddon_list::build_window(CVideo& video)
{
	return build(video, get_id(ADDON_LIST));
}

void taddon_list::pre_show(CVideo& /*video*/, twindow& window)
{
	tlistbox& list = find_widget<tlistbox>(&window, "addons", false);

	/**
	 * @todo do we really want to keep the length limit for the various
	 * items?
	 */
	BOOST_FOREACH(const config &c, cfg_.child_range("campaign")) {
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

		list.add_row(data);
	}
}

} // namespace gui2

