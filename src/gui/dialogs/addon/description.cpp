/* $Id$ */
/*
   Copyright (C) 2010 - 2012 by Ignacio R. Morelle <shadowm2006@gmail.com>
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
#include "formula_string_utils.hpp"
#include "gettext.hpp"
#include "gui/widgets/settings.hpp"
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

	std::string describe_addon_state_info(const addon_tracking_info& state)
	{
		std::string s;

		utils::string_map i18n_symbols;
		i18n_symbols["local_version"] = state.installed_version.str();

		switch(state.state) {
		case ADDON_NONE:
			if(!state.can_publish) {
				s += _("addon_state^Not installed");
			} else {
				s += _("addon_state^Published, not installed");
			}
			break;
		case ADDON_INSTALLED:
			s += "<span color='green'>";
			if(!state.can_publish) {
				s += _("addon_state^Installed");
			} else {
				s += _("addon_state^Published");
			}
			s += "</span>";
			break;
		case ADDON_INSTALLED_UPGRADABLE:
			s += "<span color='yellow'>";
			{
				const char* const vstr = !state.can_publish
					? _("addon_state^Installed ($local_version|), upgradable")
					: _("addon_state^Published ($local_version| installed), upgradable");
				s += utils::interpolate_variables_into_string(vstr, &i18n_symbols);
			}
			s += "</span>";
			break;
		case ADDON_INSTALLED_OUTDATED:
			s += "<span color='orange'>";
			{
				const char* const vstr = !state.can_publish
					? _("addon_state^Installed ($local_version|), outdated in the server")
					: _("addon_state^Published ($local_version| installed), outdated in the server");
				s += utils::interpolate_variables_into_string(vstr, &i18n_symbols);
			}
			s += "</span>";
			break;
		case ADDON_INSTALLED_BROKEN:
			s += "<span color='red'>";
			if(!state.can_publish) {
				s += _("addon_state^Installed, broken");
			} else {
				s += _("addon_state^Published, broken");
			}
			s += "</span>";
			break;
		default:
			if(!state.can_publish) {
				s += "<span color='red'>";
				s += _("addon_state^Not tracked");
			} else {
				// Published add-ons often don't have local status information,
				// hence untracked. This should be considered normal.
				s += "<span color='green'>";
				s += _("addon_state^Published");
			}
			s += "</span>";
		}

		return s;
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
 * status & & control & m &
 *         Label for displaying the current installation/upgradability status. $
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

taddon_description::taddon_description(const addon_info& addon, const addon_tracking_info& state)
{
	register_label("image", true, addon.display_icon());
	register_label("title", true, addon.title);
	register_label("version", true, addon.version);
	register_label("status", true, describe_addon_state_info(state), true);
	register_label("author", true, addon.author);
	register_label("size", true, size_display_string(addon.size));
	if(!addon.description.empty()) {
		register_label("description", true, addon.description);
	}

	std::string languages;

	foreach(const std::string& lc, addon.locales) {
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

