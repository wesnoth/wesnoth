/* $Id$ */
/*
   Copyright (C) 2006 - 2009 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playturn Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
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
} //end anonymous namespace

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
		items_sub = create_unit_table(stats_.recruits, team_num_);
		title = _("Recruits");
		break;
	case 1:
		items_sub = create_unit_table(stats_.recalls, team_num_);
		title = _("Recalls");
		break;
	case 2:
		items_sub = create_unit_table(stats_.advanced_to, team_num_);
		title = _("Advancements");
		break;
	case 3:
		items_sub = create_unit_table(stats_.deaths, team_num_);
		title = _("Losses");
		break;
	case 4:
		items_sub = create_unit_table(stats_.killed, team_num_);
		/** @todo FIXME? Perhaps killed units shouldn't have the same team-color as your own. */
		title = _("Kills");
		break;
	default:
		break;
	}
	if (items_sub.empty() == false) {
		gui::dialog d(get_display(), title + " (" + player_name_ + ")", "", gui::CLOSE_ONLY);
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
	dialog(disp, title, "", gui::NULL_DIALOG),
	detail_btn_(new gui::standard_dialog_button(disp.video(), _("Details"), 0 , false)),
	player_name_(player),
	stats_(),
	team_num_(team),
	unit_count_(5,0)
{
	add_button(detail_btn_, gui::dialog::BUTTON_EXTRA);
	add_button(new gui::standard_dialog_button(disp.video(), _("Close"), 1, true),
				gui::dialog::BUTTON_STANDARD);

	stats_ = statistics::calculate_stats(0, team_id);
	int n, cost;
	std::vector<std::string> items;
	// Prepare the menu items
	{
		std::stringstream str;
		n = statistics::sum_str_int_map(stats_.recruits);
		cost = stats_.recruit_cost;
		unit_count_[0] = n;
		str << _("Recruits") << COLUMN_SEPARATOR << n
		    << COLUMN_SEPARATOR
		    << COLUMN_SEPARATOR << IMAGE_PREFIX << "themes/gold-t.png"
		    << COLUMN_SEPARATOR << cost;
		items.push_back(str.str());
	}
	{
		std::stringstream str;
		n = statistics::sum_str_int_map(stats_.recalls);
		cost = stats_.recall_cost;
		unit_count_[1] = n;
		str << _("Recalls") << COLUMN_SEPARATOR << n
		    << COLUMN_SEPARATOR
		    << COLUMN_SEPARATOR << IMAGE_PREFIX << "themes/gold-t.png"
		    << COLUMN_SEPARATOR << cost;
		items.push_back(str.str());
	}
	{
		std::stringstream str;
		n = statistics::sum_str_int_map(stats_.advanced_to);
		unit_count_[2] = n;
		str << _("Advancements") << COLUMN_SEPARATOR << n;
		items.push_back(str.str());
	}
	{
		std::stringstream str;
		n = statistics::sum_str_int_map(stats_.deaths);
		unit_count_[3] = n;
		cost = statistics::sum_cost_str_int_map(stats_.deaths);
		str << _("Losses") << COLUMN_SEPARATOR << n
		    << COLUMN_SEPARATOR
		    << COLUMN_SEPARATOR << IMAGE_PREFIX << "themes/gold-t.png"
		    << COLUMN_SEPARATOR << cost;
		items.push_back(str.str());
	}
	{
		std::stringstream str;
		n = statistics::sum_str_int_map(stats_.killed);
		unit_count_[4] = n;
		cost = statistics::sum_cost_str_int_map(stats_.killed);
		str << _("Kills") << COLUMN_SEPARATOR << n
		    << COLUMN_SEPARATOR
		    << COLUMN_SEPARATOR << IMAGE_PREFIX << "themes/gold-t.png"
		    << COLUMN_SEPARATOR << cost;
		items.push_back(str.str());
	}
	items.push_back("");
	{
		std::stringstream str;
		str << font::BOLD_TEXT << _("Damage")
		    << COLUMN_SEPARATOR << _("Overall") << COLUMN_SEPARATOR
		    << COLUMN_SEPARATOR
		    << COLUMN_SEPARATOR << _("This Turn");
		items.push_back(str.str());
	}

	statistics_dialog::make_damage_line(items, _("Inflicted"),
			stats_.damage_inflicted,
			stats_.expected_damage_inflicted,
			stats_.turn_damage_inflicted,
			stats_.turn_expected_damage_inflicted);
	statistics_dialog::make_damage_line(items, _("Taken"),
			stats_.damage_taken,
			stats_.expected_damage_taken,
			stats_.turn_damage_taken,
			stats_.turn_expected_damage_taken);

	set_menu(items);
}

statistics_dialog::~statistics_dialog()
{
}

void statistics_dialog::make_damage_line(std::vector<std::string>& items,
					 const std::string& header,
					 const long long& damage,
					 const long long& expected,
					 const long long& turn_damage,
					 const long long& turn_expected)
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
		<< (expected == 0 ? 0 : 100 * dsa / expected)
		<< '%' << COLUMN_SEPARATOR
		<< COLUMN_SEPARATOR
		<< turn_damage << " / "
		<< (turn_expected * 10 + shift / 2) / shift * 0.1
		<< COLUMN_SEPARATOR
		<< ((dst < 0) ^ (turn_expected < 0) ? "" : "+")
		<< (turn_expected == 0 ? 0 : 100 * dst / turn_expected)
		<< '%';
	items.push_back(str.str());

}
