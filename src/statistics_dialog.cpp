/*
   Copyright (C) 2006 - 2016 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playturn Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "gettext.hpp"
#include "marked-up_text.hpp"
#include "statistics_dialog.hpp"
#include "unit_types.hpp"
#include "wml_separators.hpp"
#include "game_display.hpp"


bool statistics_dialog::use_campaign_ = false;
// These values just need to be larger than the number of rows in the dialog.
static const int BUTTON_SCENE  = 101;
static const int BUTTON_TOGGLE = 102;


namespace {

#ifdef LOW_MEM
std::vector<std::string> create_unit_table(const statistics::stats::str_int_map& m, unsigned int /*team*/)
#else
std::vector<std::string> create_unit_table(const statistics::stats::str_int_map& m, unsigned int team)
#endif
{
	std::vector<std::string> table;
	for(statistics::stats::str_int_map::const_iterator i = m.begin(); i != m.end(); ++i) {
		const unit_type *type = unit_types.find(i->first);
		if (!type) continue;

		std::stringstream str;

		str << IMAGE_PREFIX << type->image();
#ifndef LOW_MEM
		str << "~RC(" << type->flag_rgb() << ">" << team << ")";
#endif
		str << COLUMN_SEPARATOR	<< type->type_name() << COLUMN_SEPARATOR << i->second << "\n";
		table.push_back(str.str());
	}

	return table;
}


void make_damage_line(std::vector<std::string>& items, const std::string& header,
                      const long long& damage, const long long& expected,
                      const long long& turn_damage, const long long& turn_expected,
                      bool show_this_turn)
{
	int shift = statistics::stats::decimal_shift;

	long long dsa = shift * damage      - expected;
	long long dst = shift * turn_damage - turn_expected;

	std::ostringstream str;
	str << header << COLUMN_SEPARATOR
	    << damage << " / "
	    << (expected * 10 + shift / 2) / shift * 0.1
	    << COLUMN_SEPARATOR
	    << ((dsa < 0) ^ (expected < 0) ? "" : "+")
	    << (expected == 0 ? 0 : 100 * dsa / expected) << '%';
	if ( show_this_turn ) {
		str << COLUMN_SEPARATOR
		    << COLUMN_SEPARATOR
		    << turn_damage << " / "
		    << (turn_expected * 10 + shift / 2) / shift * 0.1
		    << COLUMN_SEPARATOR
		    << ((dst < 0) ^ (turn_expected < 0) ? "" : "+")
		    << (turn_expected == 0 ? 0 : 100 * dst / turn_expected) << '%';
	}
	items.push_back(str.str());
}

} //end anonymous namespace


/**
 * Picks out the stats structure that was selected for displaying.
 */
inline const statistics::stats & statistics_dialog::current_stats()
{
	return use_campaign_ ? campaign_ : *scenarios_[scenario_index_].second;
}


void statistics_dialog::action(gui::dialog_process_info &dp_info)
{
	int sel = get_menu().selection();
	bool has_details = sel < 5 && sel >= 0 && unit_count_[sel] > 0;
	detail_btn_->enable(has_details);
	if(dp_info.double_clicked && has_details) {
		set_result(sel);
	} else if(dp_info.new_key_down && !dp_info.key_down) {
		set_result(gui::CLOSE_DIALOG);
	}

	// Prepare the sub-dialog for Statistic Details
	std::string title;
	std::vector<std::string> items_sub;
	switch(result()) {
	case gui::CLOSE_DIALOG:
		break;
	case 0:
		items_sub = create_unit_table(current_stats().recruits, team_num_);
		title = _("Recruits");
		break;
	case 1:
		items_sub = create_unit_table(current_stats().recalls, team_num_);
		title = _("Recalls");
		break;
	case 2:
		items_sub = create_unit_table(current_stats().advanced_to, team_num_);
		title = _("Advancements");
		break;
	case 3:
		items_sub = create_unit_table(current_stats().deaths, team_num_);
		title = _("Losses");
		break;
	case 4:
		// Give kills a (probably) different team color.
		items_sub = create_unit_table(current_stats().killed, team_num_ == 1 ? 2 : 1);
		title = _("Kills");
		break;

	case BUTTON_SCENE:
		// Scenario selection.
		do_scene_selection();
		set_result(gui::CONTINUE_DIALOG);
		break;
	case BUTTON_TOGGLE:
		// Toggle between campaign and scenario stats.
		display_stats(!use_campaign_);
		set_result(gui::CONTINUE_DIALOG);
		break;

	default:
		break;
	}
	if (items_sub.empty() == false) {
		gui::dialog d(get_video(), title + " (" + player_name_ + ")", "", gui::CLOSE_ONLY);
		d.set_menu(items_sub);
		d.show();
		dp_info.clear_buttons();
		set_result(gui::CONTINUE_DIALOG);
	}
}


statistics_dialog::statistics_dialog(game_display &disp,
		const std::string& title,
		const unsigned int team,
		const std::string& team_id,
		const std::string& player) :
	dialog(disp.video(), title, "", gui::NULL_DIALOG),
	detail_btn_(new gui::standard_dialog_button(disp.video(), _("Details"), 0 , false)),
	toggle_btn_(new gui::dialog_button(disp.video(), "", gui::button::TYPE_PRESS, BUTTON_TOGGLE)),
	scene_btn_(new gui::dialog_button(disp.video(), _("Select Scenario"), gui::button::TYPE_PRESS, BUTTON_SCENE)),
	player_name_(player),
	campaign_(statistics::calculate_stats(team_id)),
	scenarios_(statistics::level_stats(team_id)),
	scenario_index_(scenarios_.size() - 1), // current scenario
	team_num_(team),
	unit_count_(5,0)
{
	if ( scenarios_.size() > 1 ) {
		add_button(scene_btn_, gui::dialog::BUTTON_EXTRA_LEFT);
		add_button(toggle_btn_, gui::dialog::BUTTON_EXTRA_LEFT);
	}
	add_button(detail_btn_, gui::dialog::BUTTON_EXTRA);
	add_button(new gui::standard_dialog_button(disp.video(), _("Close"), 1, true),
	           gui::dialog::BUTTON_STANDARD);

	// Initialize the displayed data.
	if ( use_campaign_  ||  scenarios_.size() == 1 )
		display_stats(use_campaign_);
	else {
		// Starting with the scenario stats, but we need to make sure the
		// window is wide enough for the campaign stats.
		display_stats(true);
		layout();
		display_stats(false);
	}
}


statistics_dialog::~statistics_dialog()
{
}


/**
 * Fills in the text to be displayed in the dialog.
 * This also updates the scenario/campaign toggle button.
 *
 * @param[in]  campaign  Indicates whether or not the campaign stats are to
 *                       be displayed.
 */
void statistics_dialog::display_stats(bool campaign)
{
	// Record which stats we will display.
	use_campaign_ = campaign;
	const statistics::stats & stats = current_stats();
	const bool show_this_turn =
		use_campaign_ || scenario_index_ + 1 == scenarios_.size();

	int n, cost;
	std::vector<std::string> items;
	// The heading for the menu items:
	{
		std::stringstream str;
		str << HEADING_PREFIX
		    << COLUMN_SEPARATOR
		    << font::BOLD_TEXT << (use_campaign_ ? _("Campaign") : _("Scenario"));
		items.push_back(str.str());
	}
	// Prepare the menu items
	{
		std::stringstream str;
		n = statistics::sum_str_int_map(stats.recruits);
		cost = stats.recruit_cost;
		unit_count_[0] = n;
		str << _("Recruits") << COLUMN_SEPARATOR << n
		    << COLUMN_SEPARATOR
		    << COLUMN_SEPARATOR << IMAGE_PREFIX << "themes/gold.png"
		    << COLUMN_SEPARATOR << cost;
		items.push_back(str.str());
	}
	{
		std::stringstream str;
		n = statistics::sum_str_int_map(stats.recalls);
		cost = stats.recall_cost;
		unit_count_[1] = n;
		str << _("Recalls") << COLUMN_SEPARATOR << n
		    << COLUMN_SEPARATOR
		    << COLUMN_SEPARATOR << IMAGE_PREFIX << "themes/gold.png"
		    << COLUMN_SEPARATOR << cost;
		items.push_back(str.str());
	}
	{
		std::stringstream str;
		n = statistics::sum_str_int_map(stats.advanced_to);
		unit_count_[2] = n;
		str << _("Advancements") << COLUMN_SEPARATOR << n;
		items.push_back(str.str());
	}
	{
		std::stringstream str;
		n = statistics::sum_str_int_map(stats.deaths);
		unit_count_[3] = n;
		cost = statistics::sum_cost_str_int_map(stats.deaths);
		str << _("Losses") << COLUMN_SEPARATOR << n
		    << COLUMN_SEPARATOR
		    << COLUMN_SEPARATOR << IMAGE_PREFIX << "themes/gold.png"
		    << COLUMN_SEPARATOR << cost;
		items.push_back(str.str());
	}
	{
		std::stringstream str;
		n = statistics::sum_str_int_map(stats.killed);
		unit_count_[4] = n;
		cost = statistics::sum_cost_str_int_map(stats.killed);
		str << _("Kills") << COLUMN_SEPARATOR << n
		    << COLUMN_SEPARATOR
		    << COLUMN_SEPARATOR << IMAGE_PREFIX << "themes/gold.png"
		    << COLUMN_SEPARATOR << cost;
		items.push_back(str.str());
	}
	items.push_back("");
	{
		std::stringstream str;
		str << font::BOLD_TEXT << _("Damage")
		    << COLUMN_SEPARATOR << _("Overall");
		if ( show_this_turn ) {
			str << COLUMN_SEPARATOR
			    << COLUMN_SEPARATOR
			    << COLUMN_SEPARATOR << _("This Turn");
		}
		items.push_back(str.str());
	}

	make_damage_line(items, _("Inflicted"),
	                 stats.damage_inflicted, stats.expected_damage_inflicted,
	                 stats.turn_damage_inflicted, stats.turn_expected_damage_inflicted,
	                 show_this_turn);
	make_damage_line(items, _("Taken"),
	                 stats.damage_taken, stats.expected_damage_taken,
	                 stats.turn_damage_taken, stats.turn_expected_damage_taken,
	                 show_this_turn);

	set_menu_items(items, true);
	toggle_btn_->set_label(use_campaign_ ? _("Scenario") : _("Campaign"));
	scene_btn_->enable(!use_campaign_);
}


/**
 * Implements the scenario selection popup.
 */
void statistics_dialog::do_scene_selection()
{
	// Prepare a list of scenario names.
	std::vector<std::string> names;
	for ( size_t i = 0; i != scenarios_.size(); ++i )
		names.push_back(*scenarios_[i].first);

	// Let the player choose a scenario.
	SDL_Rect const &loc = scene_btn_->location();
	size_t new_scenario = gui::show_dialog(get_video(), NULL, "", "",
	                                       gui::MESSAGE, &names, NULL, "", NULL,
	                                       -1, NULL, loc.x, loc.y + loc.h);

	if ( new_scenario != scenario_index_  &&  new_scenario < scenarios_.size() )
	{
		// Switch the displayed data to the selected scenario.
		scenario_index_ = new_scenario;
		scene_btn_->set_label(*scenarios_[new_scenario].first);
		display_stats(false);
	}
}

