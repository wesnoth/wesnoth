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
	void make_damage_line(std::vector<std::string>&,const std::string&,const long long&,const long long&,const long long&,const long long&);
	gui::dialog_button *detail_btn_;
	std::string player_name_;
	statistics::stats stats_;
	unsigned int team_num_;
	std::vector<int> unit_count_;
};
