/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "config.hpp"
#include "game_config.hpp"
#include "wesconfig.h"
#include "serialization/string_utils.hpp"

#include <cstdlib>

namespace game_config
{
	int base_income = 2;
	int village_income = 1;
	int heal_amount = 4;
	int healer_heals_per_turn = 8;
	int cure_amount = 8;
	int curer_heals_per_turn = 18;
	int rest_heal_amount= 2;
	int recall_cost = 20;
	int kill_experience = 8;
	int leadership_bonus = 25;
	const std::string version = VERSION;
	bool debug = false, editor = false, ignore_replay_errors = false;

	std::string game_icon = "wesnoth-icon.png", game_title, game_logo, title_music, anonymous_music;
	int title_logo_x = 0, title_logo_y = 0, title_buttons_x = 0, title_buttons_y = 0, title_buttons_padding = 0,
	    title_tip_x = 0, title_tip_y = 0, title_tip_width = 0, title_tip_padding = 0;

	std::string missile_n_image, missile_ne_image;

	std::string terrain_mask_image = "terrain/alphamask.png";

	std::string map_image = "maps/wesnoth.png";
	std::string rightside_image = "misc/rightside.png";
	std::string rightside_image_bot = "misc/rightside-bottom.png";

	std::string moved_energy_image = "misc/bar-energy-moved.png";
	std::string unmoved_energy_image = "misc/bar-energy-unmoved.png";
	std::string partmoved_energy_image = "misc/bar-energy-partmoved.png";
	std::string enemy_energy_image = "misc/bar-energy-enemy.png";
	std::string ally_energy_image = "misc/bar-energy-ally.png";
	std::string flag_image = "terrain/flag-team%d-1.png:150,terrain/flag-team%d-2.png:150";

	std::string dot_image = "misc/dot.png";
	std::string cross_image = "misc/cross.png";

	std::vector<std::string> foot_left_nw, foot_left_n, foot_right_nw, foot_right_n;

	std::string observer_image;

	std::string unchecked_menu_image = "buttons/checkbox.png";
	std::string checked_menu_image = "buttons/checkbox-pressed.png";

	std::string download_campaign_image;

	std::string level_image;
	std::string ellipsis_image;

	namespace sounds {
		const std::string turn_bell = "bell.wav",
		                  receive_message = "receive.wav",
						  user_arrive = "arrive.wav",
						  user_leave = "leave.wav";
	}



#ifdef WESNOTH_PATH
	std::string path = WESNOTH_PATH;
#else
	std::string path = "";
#endif

	void load_config(const config* cfg)
	{
		if(cfg == NULL)
			return;

		const config& v = *cfg;

		base_income = atoi(v["base_income"].c_str());
		village_income = atoi(v["village_income"].c_str());
		heal_amount = atoi(v["heal_amount"].c_str());
		healer_heals_per_turn = atoi(v["healer_heals_per_turn"].c_str());
		cure_amount = atoi(v["cure_amount"].c_str());
		curer_heals_per_turn = atoi(v["curer_heals_per_turn"].c_str());
		rest_heal_amount = atoi(v["rest_heal_amount"].c_str());
		recall_cost = atoi(v["recall_cost"].c_str());
		kill_experience = atoi(v["kill_experience"].c_str());

		game_icon = v["icon"];
		game_title = v["title"];
		game_logo = v["logo"];
		title_music = v["title_music"];
		anonymous_music = v["anonymous_music"];

		title_logo_x = atoi(v["logo_x"].c_str());
		title_logo_y = atoi(v["logo_y"].c_str());
		title_buttons_x = atoi(v["buttons_x"].c_str());
		title_buttons_y = atoi(v["buttons_y"].c_str());
		title_buttons_padding = atoi(v["buttons_padding"].c_str());
		title_tip_x = atoi(v["tip_x"].c_str());
		title_tip_y = atoi(v["tip_y"].c_str());
		title_tip_width = atoi(v["tip_width"].c_str());
		title_tip_padding = atoi(v["tip_padding"].c_str());

		map_image = v["map_image"];
		rightside_image = v["sidebar_image"];
		rightside_image_bot = v["sidebar_image_bottom"];

		moved_energy_image = v["moved_energy_image"];
		unmoved_energy_image = v["unmoved_energy_image"];
		partmoved_energy_image = v["partmoved_energy_image"];
		enemy_energy_image = v["enemy_energy_image"];
		ally_energy_image = v["ally_energy_image"];
		flag_image = v["flag_image"];

		cross_image = v["cross_image"];
		dot_image = v["dot_image"];

		foot_left_nw = utils::split(v["footprint_left_nw"]);
		foot_left_n = utils::split(v["footprint_left_n"]);
		foot_right_nw = utils::split(v["footprint_right_nw"]);
		foot_right_n = utils::split(v["footprint_right_n"]);

		missile_n_image = v["missile_n_image"];
		missile_ne_image = v["missile_ne_image"];

		terrain_mask_image = v["terrain_mask_image"];

		observer_image = v["observer_image"];

		download_campaign_image = v["download_campaign_image"];
		level_image = v["level_image"];
		ellipsis_image = v["ellipsis_image"];
	}
}
