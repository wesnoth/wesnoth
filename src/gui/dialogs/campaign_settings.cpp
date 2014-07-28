/*
   Copyright (C) 2014 by Nathan Walker <nathan.b.walker@vanderbilt.edu>
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
#include "gui/auxiliary/find_widget.tpp"
#include "gui/dialogs/campaign_settings.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"

#include <boost/bind.hpp>

namespace gui2
{

REGISTER_DIALOG(campaign_settings)

tcampaign_settings::tcampaign_settings(ng::create_engine& eng)
		: engine_(eng)
{ }


void tcampaign_settings::change_era(twindow& window)
{
	tlistbox& list = find_widget<tlistbox>(&window, "era_list", false);
	engine_.set_current_era_index(list.get_selected_row());
}

void tcampaign_settings::change_mod(size_t index, twindow&)
{
	engine_.set_current_mod_index(index);
	// force toggle mod.
	engine_.toggle_current_mod(true);
}

void tcampaign_settings::update_lists(twindow& window)
{
	tlistbox& era_list = find_widget<tlistbox>(&window, "era_list", false);
	tlistbox& mod_list = find_widget<tlistbox>(&window, "modification_list", false);
	tlabel& era_label = find_widget<tlabel>(&window, "era_label", false);

	era_list.clear();

	if (engine_.current_level().allow_era_choice()) {
		BOOST_FOREACH(std::string era, engine_.extras_menu_item_names(ng::create_engine::ERA)) {
			std::map<std::string, string_map> row;
			string_map column;

			column["label"] = era;
			row.insert(std::make_pair("era", column));

			era_list.add_row(row);
		}
		era_list.select_row(engine_.current_era_index());
	}
	else {
		era_label.set_label(_("Era:\nNot allowed"));
		era_list.set_active(false);
	}

	mod_list.clear();

	{
		int i = 0;
		BOOST_FOREACH(const ng::create_engine::extras_metadata_ptr mod, engine_.get_const_extras_by_type(ng::create_engine::MOD)) {
			std::map<std::string, string_map> row;
			string_map column;

			column["label"] = mod->name;
			row.insert(std::make_pair("mod", column));

			mod_list.add_row(row);

			std::vector<std::string> enabled = engine_.active_mods();
			mod_list.select_row(i,
				std::find(enabled.begin(), enabled.end(), mod->id)
				!= enabled.end());

			++i;
		}
	}
}

void tcampaign_settings::pre_show(CVideo&, twindow& window)
{
	find_widget<ttoggle_button>(&window, "mp_configure", false).set_value(
		engine_.get_state().mp_settings().show_configure);

	find_widget<ttoggle_button>(&window, "mp_connect", false).set_value(
		engine_.get_state().mp_settings().show_connect);

	update_lists(window);

	tlistbox& era_list = find_widget<tlistbox>(&window, "era_list", false);
	tlistbox& mod_list = find_widget<tlistbox>(&window, "modification_list", false);

	era_list.set_callback_item_change(
			boost::bind(&tcampaign_settings::change_era, this, boost::ref(window)));
	mod_list.set_callback_item_change(
			boost::bind(&tcampaign_settings::change_mod, this, _1, boost::ref(window)));
}

void tcampaign_settings::post_show(twindow& window)
{
	engine_.get_state().mp_settings().show_connect =
		find_widget<ttoggle_button>(&window, "mp_connect", false).get_value();
	engine_.get_state().mp_settings().show_configure =
		find_widget<ttoggle_button>(&window, "mp_configure", false).get_value();
}

} // end namespace gui2
