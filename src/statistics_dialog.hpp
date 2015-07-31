/*
   Copyright (C) 2006 - 2015 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
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

#ifndef STATISTICS_DIALOG_HPP_INCLUDED
#define STATISTICS_DIALOG_HPP_INCLUDED

#include "construct_dialog.hpp"
#include "statistics.hpp"

#include <vector>
#include <string>

class game_display;

class statistics_dialog : public gui::dialog
{
public:
	statistics_dialog(game_display &disp,
		const std::string& title,
		const unsigned int team,
		const std::string& team_id,
		const std::string& player);
	~statistics_dialog();
protected:
	void action(gui::dialog_process_info &dp_info);
private:
	/// Picks out the stats structure that was selected for displaying.
	inline const statistics::stats & current_stats();
	/// Fills in the text to be displayed in the dialog.
	void display_stats(bool campaign);
	/// Implements the scenario selection popup.
	void do_scene_selection();

	gui::dialog_button *detail_btn_;
	gui::dialog_button *toggle_btn_;
	gui::dialog_button *scene_btn_;
	std::string player_name_;
	const statistics::stats  campaign_;
	const statistics::levels scenarios_;
	size_t scenario_index_;
	unsigned int team_num_;
	std::vector<int> unit_count_;

	// This is static so the setting can be remembered throughout the session.
	static bool use_campaign_;
};

#endif
