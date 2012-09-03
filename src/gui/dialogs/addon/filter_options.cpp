/* $Id$ */
/*
   Copyright (C) 2012 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "gui/dialogs/addon/filter_options.hpp"

#ifdef GUI2_EXPERIMENTAL_LISTBOX
	#include "gui/widgets/list.hpp"
#else
	#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "gettext.hpp"

namespace gui2 {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_addon_filter_options
 *
 * == Add-on filter options ==
 *
 * Advanced filtering options for the legacy (GUI1) Add-ons Manager dialog.
 *
 * @begin{table}{dialog_widgets}
 *
 *
 * @end{table}
 */

REGISTER_DIALOG(addon_filter_options)

taddon_filter_options::taddon_filter_options()
	: displayed_status_()
	, displayed_types_()
{
	displayed_types_.assign(true);

	// This part has to be hardcoded, sadly.

	register_bool("show_unknown", true, displayed_types_[ADDON_UNKNOWN]);
	register_bool("show_sp_campaigns", true, displayed_types_[ADDON_SP_CAMPAIGN]);
	register_bool("show_sp_scenarios", true, displayed_types_[ADDON_SP_SCENARIO]);
	register_bool("show_mp_campaigns", true, displayed_types_[ADDON_MP_CAMPAIGN]);
	register_bool("show_mp_scenarios", true, displayed_types_[ADDON_MP_SCENARIO]);
	register_bool("show_mp_maps", true, displayed_types_[ADDON_MP_MAPS]);
	register_bool("show_mp_eras", true, displayed_types_[ADDON_MP_ERA]);
	register_bool("show_mp_factions", true, displayed_types_[ADDON_MP_FACTION]);
	register_bool("show_media", true, displayed_types_[ADDON_MEDIA]);
	// FIXME: (also in WML) should this and Unknown be a single option in the UI?
	register_bool("show_other", true, displayed_types_[ADDON_OTHER]);
}

void taddon_filter_options::read_types_vector(const std::vector<bool>& v)
{
	for(size_t k = 0; k < displayed_types_.size(); ++k) {
		// All unspecified types default to visible.
		displayed_types_[k] = k < v.size() ? v[k] : true;
	}
}

void taddon_filter_options::pre_show(CVideo& /*video*/, twindow& window)
{
	tlistbox& list = find_widget<tlistbox>(&window, "statuses_list", false);
	window.keyboard_capture(&list);

	for(unsigned k = ADDON_STATUS_FILTER(); k < FILTER_COUNT; ++k) {
		std::map<std::string, string_map> row;
		string_map column;

		column["label"] = status_label(ADDON_STATUS_FILTER(k));
		row.insert(std::make_pair("status", column));

		list.add_row(row);
	}

	list.select_row(displayed_status_);
}

void taddon_filter_options::post_show(twindow& window)
{
	tlistbox& list = find_widget<tlistbox>(&window, "statuses_list", false);
	const int selected = list.get_selected_row();

	if(selected != -1) {
		displayed_status_ = ADDON_STATUS_FILTER(selected);
	}
}

std::string taddon_filter_options::status_label(ADDON_STATUS_FILTER s)
{
	switch(s) {
	case FILTER_NOT_INSTALLED:
		return _("addons_view^Not Installed");
	case FILTER_UPGRADABLE:
		return _("addons_view^Upgradable");
	case FILTER_INSTALLED:
		return _("addons_view^Installed");
	default:
		return _("addons_view^All Add-ons");
	}
}

} // end namespace gui2
