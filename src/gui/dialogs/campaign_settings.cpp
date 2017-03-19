/*
   Copyright (C) 2014 - 2017 by Nathan Walker <nathan.b.walker@vanderbilt.edu>
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

#include "saved_game.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/campaign_settings.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"

#include "utils/functional.hpp"

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_campaign_settings
 *
 * == Campaign settings ==
 *
 * This shows the dialog which allows the user to select an era
 * and modifications for the selected campaign, and to show
 * dialogs for additional level configuration.
 *
 * @begin{table}{dialog_widgets}
 *
 * title & & label & m &
 *         Dialog title label. $
 *
 * mp_configure & & toggle_button & m &
 *         Option to show the multiplayer configure dialog. $
 *
 * mp_connect & & toggle_button & m &
 *         Option to show the multiplayer connect dialog. $
 *
 * era_label & & label & m &
 *         The label for the era list. $
 *
 * era_list & & listbox & m &
 *         List of available eras. $
 *
 * -era & & label & o &
 *         The name of the era. One can be selected at a time. $
 *
 * mod_label & & label & m &
 *         The label for the modification list. $
 *
 * modification_list & & listbox & m &
 *         List of available modifications. $
 *
 * -mod & & toggle_button & o &
 *         The name of the modification. Each can be enabled or disabled. $
 *
 * @end{table}
 */

REGISTER_DIALOG(campaign_settings)

campaign_settings::campaign_settings(ng::create_engine& eng)
		: engine_(eng)
{ }


void campaign_settings::change_era(window& window)
{
	listbox& list = find_widget<listbox>(&window, "era_list", false);
	engine_.set_current_era_index(list.get_selected_row());
}

void campaign_settings::change_mod(size_t index, window&)
{
	engine_.set_current_mod_index(index);
	// force toggle mod.
	engine_.toggle_current_mod(true);
}

void campaign_settings::update_lists(window& window)
{
	listbox& era_list = find_widget<listbox>(&window, "era_list", false);
	listbox& mod_list = find_widget<listbox>(&window, "modification_list", false);
	label& era_label = find_widget<label>(&window, "era_label", false);
	label& mod_label = find_widget<label>(&window, "mod_label", false);

	era_list.clear();

	if (engine_.current_level().allow_era_choice()) {
		for (std::string era : engine_.extras_menu_item_names(ng::create_engine::ERA)) {
			std::map<std::string, string_map> row;
			string_map column;

			column["label"] = era;
			row.emplace("era", column);

			era_list.add_row(row);
		}
		era_list.select_row(engine_.current_era_index());
	}
	else {
		era_label.set_label(_("Era:\n\nNot allowed."));
		era_list.set_active(false);
	}

	mod_list.clear();

	{
		int i = 0;
		for (const ng::create_engine::extras_metadata_ptr mod : engine_.get_const_extras_by_type(ng::create_engine::MOD)) {
			std::map<std::string, string_map> row;
			string_map column;

			column["label"] = mod->name;
			row.emplace("mod", column);

			mod_list.add_row(row);

			std::vector<std::string> enabled = engine_.active_mods();
			mod_list.select_row(i,
				std::find(enabled.begin(), enabled.end(), mod->id)
				!= enabled.end());

			++i;
		}
		if (i == 0) {
			mod_label.set_label(_("Modifications:\n\nNone found."));
			mod_list.set_active(false);
		}
	}
}

void campaign_settings::pre_show(window& window)
{
	find_widget<toggle_button>(&window, "mp_connect", false).set_value(
		engine_.get_state().mp_settings().show_connect);

	update_lists(window);

	listbox& era_list = find_widget<listbox>(&window, "era_list", false);
	listbox& mod_list = find_widget<listbox>(&window, "modification_list", false);

	toggle_button& connect_toggle = find_widget<toggle_button>(&window, "mp_connect", false);

	const config::attribute_value& show_connect = engine_.current_level().data()["show_connect"];
	if (show_connect.blank() || show_connect.str() == "user") {
		connect_toggle.set_active(true);
	} else {
		connect_toggle.set_value(show_connect.to_bool());
		connect_toggle.set_active(false);
	}

	era_list.set_callback_item_change(
			std::bind(&campaign_settings::change_era, this, std::ref(window)));
	mod_list.set_callback_item_change(
			std::bind(&campaign_settings::change_mod, this, _1, std::ref(window)));
}

void campaign_settings::post_show(window& window)
{
	engine_.get_state().mp_settings().show_connect =
		find_widget<toggle_button>(&window, "mp_connect", false).get_value_bool();
}

} // namespace dialogs
} // namespace gui2
