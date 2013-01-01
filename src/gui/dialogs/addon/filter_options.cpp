/* $Id$ */
/*
   Copyright (C) 2012 - 2013 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "gui/dialogs/field.hpp"
#include "gui/widgets/button.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
	#include "gui/widgets/list.hpp"
#else
	#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "gettext.hpp"

#include <boost/bind.hpp>

namespace {
	bool unchecked_bool_field_finder(gui2::twindow& window, gui2::tfield_bool* bool_field)
	{
		return bool_field->get_widget_value(window) == false;
	}
}

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
	, displayed_types_fields_()
{
	displayed_types_.assign(true);

	// This part has to be hardcoded, sadly.

	register_displayed_type_field("show_unknown", ADDON_UNKNOWN);
	register_displayed_type_field("show_sp_campaigns", ADDON_SP_CAMPAIGN);
	register_displayed_type_field("show_sp_scenarios", ADDON_SP_SCENARIO);
	register_displayed_type_field("show_mp_campaigns", ADDON_MP_CAMPAIGN);
	register_displayed_type_field("show_mp_scenarios", ADDON_MP_SCENARIO);
	register_displayed_type_field("show_mp_maps", ADDON_MP_MAPS);
	register_displayed_type_field("show_mp_eras", ADDON_MP_ERA);
	register_displayed_type_field("show_mp_factions", ADDON_MP_FACTION);
	register_displayed_type_field("show_mp_mods", ADDON_MP_MOD);
	register_displayed_type_field("show_media", ADDON_MEDIA);
	// FIXME: (also in WML) should this and Unknown be a single option in the UI?
	register_displayed_type_field("show_other", ADDON_OTHER);
}

void taddon_filter_options::register_displayed_type_field(const std::string& field_id, ADDON_TYPE addon_type)
{
	displayed_types_fields_.push_back(register_bool(field_id, true, displayed_types_[addon_type]));
}

void taddon_filter_options::read_types_vector(const std::vector<bool>& v)
{
	for(size_t k = 0; k < displayed_types_.size(); ++k) {
		// All unspecified types default to visible.
		displayed_types_[k] = k < v.size() ? v[k] : true;
	}
}

void taddon_filter_options::toggle_all_displayed_types_button_callback(twindow& window)
{
	const bool have_any_unchecked = displayed_types_fields_.end() == std::find_if(
		displayed_types_fields_.begin(),
		displayed_types_fields_.end(),
		boost::bind(&unchecked_bool_field_finder, boost::ref(window), _1));

	BOOST_FOREACH(tfield_bool* const field, displayed_types_fields_) {
		field->set_widget_value(window, !have_any_unchecked);
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

	connect_signal_mouse_left_click(
		find_widget<tbutton>(&window, "toggle_all_displayed_types", false),
		boost::bind(&taddon_filter_options::toggle_all_displayed_types_button_callback, this, boost::ref(window)));
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
