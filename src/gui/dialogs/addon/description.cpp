/* $Id$ */
/*
   Copyright (C) 2010 - 2013 by Ignacio R. Morelle <shadowm2006@gmail.com>
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

#include "gui/dialogs/addon/description.hpp"

#include "gui/widgets/settings.hpp"
#include "language.hpp"

#include <boost/foreach.hpp>

namespace {
	std::string langcode_to_string(const std::string& lcode)
	{
		BOOST_FOREACH(const language_def& ld, get_languages()) {
			if(ld.localename == lcode || ld.localename.substr(0, 2) == lcode) {
				return ld.language;
			}
		}

		return "";
	}
}

namespace gui2 {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_addon_description
 *
 * == Add-on description ==
 *
 * Add-on description and details for the add-ons manager interface.
 *
 * @begin{table}{dialog_widgets}
 *
 * image & & control & m &
 *         Label for displaying the add-on icon, in a 72x72 area. $
 *
 * title & & control & m &
 *         Dialog title label, corresponding to the add-on name. $
 *
 * version & & control & m &
 *         Label for displaying the add-on version number. $
 *
 * author & & control & m &
 *         Label for displaying the add-on author/maintainer name. $
 *
 * size & & control & m &
 *         Label for displaying the add-on package size. $
 *
 * description & & control & m &
 *         Text label for displaying the add-on's description. The control can
 *         be given a text, this text is shown when the addon has no
 *         description. If the addon has a description this field shows the
 *         description of the addon. $
 *
 * translations & & control & m &
 *         Label for displaying a list of translations provided by the add-on.
 *         Like the ''description'' it can also show a default text if no
 *         translations are available. $
 *
 * @end{table}
 */

REGISTER_DIALOG(addon_description)

taddon_description::taddon_description(const addon_info& addon)
{
	register_label("image", true, addon.icon);
	register_label("title", true, addon.name);
	register_label("version", true, addon.version);
	register_label("author", true, addon.author);
	register_label("size", true, addon.sizestr);
	if(!addon.description.empty()) {
		register_label("description", true, addon.description);
	}

	std::string languages;

	BOOST_FOREACH(const std::string& lc, addon.translations) {
		const std::string& langlabel = langcode_to_string(lc);
		if(!langlabel.empty()) {
			if(!languages.empty()) {
				languages += ", ";
			}
			languages += langlabel;
		}
	}

	if(!languages.empty()) {
		register_label("translations", true, languages);
	}
}

} // namespace  gui2

