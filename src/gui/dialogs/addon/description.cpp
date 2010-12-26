/* $Id$ */
/*
   Copyright (C) 2010 by Ignacio R. Morelle <shadowm2006@gmail.com>
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

#include "foreach.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "language.hpp"

namespace {
	std::string langcode_to_string(const std::string& lcode)
	{
		foreach(const language_def& ld, get_languages()) {
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
 *         Text label for displaying the add-on's description. $
 *
 * translations & & control & m &
 *         Label for displaying a list of translations provided by the add-on. $
 *
 * @end{table}
 */

REGISTER_WINDOW(addon_description)

void taddon_description::pre_show(CVideo& /*video*/, twindow& window)
{
	find_widget<tcontrol>(&window, "image", false).set_label(ainfo_.icon);
	find_widget<tcontrol>(&window, "title", false).set_label(ainfo_.name);
	find_widget<tcontrol>(&window, "version", false).set_label(ainfo_.version);
	find_widget<tcontrol>(&window, "author", false).set_label(ainfo_.author);
	find_widget<tcontrol>(&window, "size", false).set_label(ainfo_.sizestr);

	// Validate widget presence in either path
	tcontrol& ctl_description = find_widget<tcontrol>(&window, "description", false);
	if(ainfo_.description.empty() == false) {
		ctl_description.set_label(ainfo_.description);
	}

	std::string languages;

	foreach(const std::string& lc, ainfo_.translations) {
		const std::string& langlabel = langcode_to_string(lc);
		if(langlabel.empty() == false) {
			if(languages.empty() == false) {
				languages += ", ";
			}
			languages += langlabel;
		}
	}

	// Validate widget presence in either path
	tcontrol& ctl_languages = find_widget<tcontrol>(&window, "translations", false);
	if(languages.empty() == false) {
		ctl_languages.set_label(languages);
	}
}

}

