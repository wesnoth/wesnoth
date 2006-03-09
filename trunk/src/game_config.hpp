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
#ifndef GAME_CONFIG_H_INCLUDED
#define GAME_CONFIG_H_INCLUDED

class config;

#include "color_range.hpp"

#include <string>
#include <vector>
#include <map>

//basic game configuration information is here.
namespace game_config
{
	extern int base_income;
	extern int village_income;
	extern int heal_amount;
	extern int healer_heals_per_turn;
	extern int cure_amount;
	extern int curer_heals_per_turn;
	extern int rest_heal_amount;
	extern int recall_cost;
	extern int kill_experience;
	extern int leadership_bonus;
	extern const std::string version;

	extern bool debug, editor, ignore_replay_errors;

	extern std::string path;

	extern std::string game_icon, game_title, game_logo, title_music, map_image, rightside_image, rightside_image_bot,
	  anonymous_music, victory_music, defeat_music,
	  moved_ball_image, unmoved_ball_image, partmoved_ball_image,
	  enemy_ball_image, ally_ball_image, energy_image,
	  flag_image, dot_image, cross_image,
	  missile_n_image, missile_ne_image,
	  terrain_mask_image, observer_image, download_campaign_image,
	  checked_menu_image, unchecked_menu_image, level_image,ellipsis_image;

  extern std::vector<Uint32> flag_rgb;

	extern std::vector<std::string> foot_left_nw,foot_left_n,foot_right_nw,foot_right_n;

	extern int title_logo_x, title_logo_y, title_buttons_x, title_buttons_y, title_buttons_padding, title_tip_x, title_tip_y, title_tip_width, title_tip_padding;

        extern std::map<int, color_range> team_rgb_range;
        extern std::map<int, std::string > team_rgb_name;
	namespace sounds {
		extern const std::string turn_bell, receive_message, user_arrive, user_leave;
	}

        void load_config(const config* cfg);
}

#endif
