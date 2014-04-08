/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Show screen with scrolling credits.
 */

#include "leader_scroll_dialog.hpp"
#include "wml_separators.hpp"
#include "map.hpp"

//#include "construct_dialog.hpp"
//#include "display.hpp"
//#include "gettext.hpp"
#include "marked-up_text.hpp"
//
//#include <boost/foreach.hpp>

/**
 * @namespace about
 * Display credits %about all contributors.
 *
 * This module is used from the startup screen. \n
 * When show_about() is called, a list of contributors
 * to the game will be presented to the user.
 */

namespace gui {

void status_table(display& gui, int selected)
{
	std::stringstream heading;
	heading << HEADING_PREFIX << _("Leader") << COLUMN_SEPARATOR << ' ' << COLUMN_SEPARATOR
			<< _("Team")         << COLUMN_SEPARATOR
			<< _("Gold")         << COLUMN_SEPARATOR
			<< _("Villages")     << COLUMN_SEPARATOR
			<< _("status^Units") << COLUMN_SEPARATOR
			<< _("Upkeep")       << COLUMN_SEPARATOR
			<< _("Income");

	gui::menu::basic_sorter sorter;
	sorter.set_redirect_sort(0,1).set_alpha_sort(1).set_alpha_sort(2).set_numeric_sort(3)
		  .set_numeric_sort(4).set_numeric_sort(5).set_numeric_sort(6).set_numeric_sort(7);

	std::vector<std::string> items;
	std::vector<bool> leader_bools;
	items.push_back(heading.str());

	const gamemap& map = gui.get_map();
	const unit_map& units = gui.get_const_units();
	assert(&gui.get_teams() == resources::teams);
	const std::vector<team>& teams = gui.get_teams();

	const team& viewing_team = teams[gui.viewing_team()];

	unsigned total_villages = 0;
	// a variable to check if there are any teams to show in the table
	bool status_table_empty = true;

	//if the player is under shroud or fog, they don't get
	//to see details about the other sides, only their own
	//side, allied sides and a ??? is shown to demonstrate
	//lack of information about the other sides But he see
	//all names with in colors
	for(size_t n = 0; n != teams.size(); ++n) {
		if(teams[n].hidden()) {
			continue;
		}
		status_table_empty=false;

		const bool known = viewing_team.knows_about_team(n, network::nconnections() > 0);
		const bool enemy = viewing_team.is_enemy(n+1);

		std::stringstream str;

		const team_data data = calculate_team_data(teams[n],n+1);

		unit_map::const_iterator leader = units.find_leader(n + 1);
		std::string leader_name;
		//output the number of the side first, and this will
		//cause it to be displayed in the correct color
		if(leader != units.end()) {
			const bool fogged = viewing_team.fogged(leader->get_location());
			// Add leader image. If it's fogged
			// show only a random leader image.
			if (!fogged || known || game_config::debug) {
				str << IMAGE_PREFIX << leader->absolute_image();
				leader_bools.push_back(true);
				leader_name = leader->name();
			} else {
				str << IMAGE_PREFIX << std::string("units/unknown-unit.png");
				leader_bools.push_back(false);
				leader_name = "Unknown";
			}
	//	if (gamestate_.classification().campaign_type == "multiplayer")
	//			leader_name = teams[n].current_player();

#ifndef LOW_MEM
			str << leader->image_mods();
#endif
		} else {
			leader_bools.push_back(false);
		}
		str << COLUMN_SEPARATOR	<< team::get_side_highlight(n)
			<< leader_name << COLUMN_SEPARATOR
			<< (data.teamname.empty() ? teams[n].team_name() : data.teamname)
			<< COLUMN_SEPARATOR;

		if(!known && !game_config::debug) {
			// We don't spare more info (only name)
			// so let's go on next side ...
			items.push_back(str.str());
			continue;
		}

		if(game_config::debug) {
			str << utils::half_signed_value(data.gold) << COLUMN_SEPARATOR;
		} else if(enemy && viewing_team.uses_fog()) {
			str << ' ' << COLUMN_SEPARATOR;
		} else {
			str << utils::half_signed_value(data.gold) << COLUMN_SEPARATOR;
		}
		str << data.villages;
                if(!(viewing_team.uses_fog() || viewing_team.uses_shroud())) {
                        str << "/" << map.villages().size();
                }
		str << COLUMN_SEPARATOR
			<< data.units << COLUMN_SEPARATOR << data.upkeep << COLUMN_SEPARATOR
			<< (data.net_income < 0 ? font::BAD_TEXT : font::NULL_MARKUP) << utils::signed_value(data.net_income);
		total_villages += data.villages;
		items.push_back(str.str());
	}
	if (total_villages > map.villages().size()) {
		//TODO
//		ERR_NG << "Logic error: map has " << map.villages().size()
//				<< " villages but status table shows " << total_villages << " owned in total\n";
	}

	if (status_table_empty)
	{
		// no sides to show - display empty table
		std::stringstream str;
		str << " ";
		for (int i=0;i<7;++i)
			str << COLUMN_SEPARATOR << " ";
		leader_bools.push_back(false);
		items.push_back(str.str());
	}
	int result = 0;
	{
		leader_scroll_dialog slist(gui, _("Current Status"), leader_bools, selected, gui::DIALOG_FORWARD);
		slist.add_button(new gui::dialog_button(gui.video(), _("More >"),
												 gui::button::TYPE_PRESS, gui::DIALOG_FORWARD),
												 gui::dialog::BUTTON_EXTRA_LEFT);
		slist.set_menu(items, &sorter);
		slist.get_menu().move_selection(selected);
		result = slist.show();
		selected = slist.get_menu().selection();
	} // this will kill the dialog before scrolling

	if (result >= 0) {
		//TODO
	//	gui.scroll_to_leader(units_, selected+1);
	}
	else if (result == gui::DIALOG_FORWARD)
		scenario_settings_table(gui, selected);
}

void scenario_settings_table(display& gui, int selected)
{
	std::stringstream heading;
	heading << HEADING_PREFIX << _("scenario settings^Leader") << COLUMN_SEPARATOR
			<< COLUMN_SEPARATOR
			<< _("scenario settings^Side")              << COLUMN_SEPARATOR
			<< _("scenario settings^Start\nGold")       << COLUMN_SEPARATOR
			<< _("scenario settings^Base\nIncome")      << COLUMN_SEPARATOR
			<< _("scenario settings^Gold Per\nVillage") << COLUMN_SEPARATOR
			<< _("scenario settings^Support Per\nVillage") << COLUMN_SEPARATOR
			<< _("scenario settings^Fog")               << COLUMN_SEPARATOR
			<< _("scenario settings^Shroud");

	gui::menu::basic_sorter sorter;
	sorter.set_redirect_sort(0,1).set_alpha_sort(1).set_numeric_sort(2)
		  .set_numeric_sort(3).set_numeric_sort(4).set_numeric_sort(5)
		  .set_numeric_sort(6).set_alpha_sort(7).set_alpha_sort(8);

	std::vector<std::string> items;
	std::vector<bool> leader_bools;
	items.push_back(heading.str());

	//const gamemap& map = gui.get_map();
	const unit_map& units = gui.get_const_units();
	const std::vector<team>& teams = gui.get_teams();

	const team& viewing_team = teams[gui.viewing_team()];
	bool settings_table_empty = true;
	bool fogged;

	for(size_t n = 0; n != teams.size(); ++n) {
		if(teams[n].hidden()) {
			continue;
		}
		settings_table_empty = false;

		std::stringstream str;
		unit_map::const_iterator leader = units.find_leader(n + 1);

		if(leader != units.end()) {
			// Add leader image. If it's fogged
			// show only a random leader image.
			fogged=viewing_team.fogged(leader->get_location());
			if (!fogged || viewing_team.knows_about_team(n, network::nconnections() > 0) || game_config::debug) {
				str << IMAGE_PREFIX << leader->absolute_image();
				leader_bools.push_back(true);
			} else {
				str << IMAGE_PREFIX << std::string("units/unknown-unit.png");
				leader_bools.push_back(false);
			}
#ifndef LOW_MEM
			str << "~RC(" << leader->team_color() << '>'
			    << team::get_side_color_index(n+1) << ")";
#endif
		} else {
			leader_bools.push_back(false);
		}

		str << COLUMN_SEPARATOR	<< team::get_side_highlight(n)
			<< teams[n].current_player() << COLUMN_SEPARATOR
			<< n + 1 << COLUMN_SEPARATOR
			<< teams[n].start_gold() << COLUMN_SEPARATOR
			<< teams[n].base_income() << COLUMN_SEPARATOR
			<< teams[n].village_gold() << COLUMN_SEPARATOR
			<< teams[n].village_support() << COLUMN_SEPARATOR
			<< (teams[n].uses_fog()    ? _("yes") : _("no")) << COLUMN_SEPARATOR
			<< (teams[n].uses_shroud() ? _("yes") : _("no")) << COLUMN_SEPARATOR;

		items.push_back(str.str());
	}

	if (settings_table_empty)
	{
		// no sides to show - display empty table
		std::stringstream str;
		for (int i=0;i<8;++i)
			str << " " << COLUMN_SEPARATOR;
		leader_bools.push_back(false);
		items.push_back(str.str());
	}
	int result = 0;
	{
		leader_scroll_dialog slist(gui, _("Scenario Settings"), leader_bools, selected, gui::DIALOG_BACK);
		slist.set_menu(items, &sorter);
		slist.get_menu().move_selection(selected);
		slist.add_button(new gui::dialog_button(gui.video(), _(" < Back"),
				gui::button::TYPE_PRESS, gui::DIALOG_BACK),
				gui::dialog::BUTTON_EXTRA_LEFT);
		result = slist.show();
		selected = slist.get_menu().selection();
	} // this will kill the dialog before scrolling

	if (result >= 0) {
		//TODO
		//gui_->scroll_to_leader(units_, selected+1);
	}
	else if (result == gui::DIALOG_BACK)
		status_table(gui, selected);
}



} // end namespace about
