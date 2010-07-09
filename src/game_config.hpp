/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef GAME_CONFIG_H_INCLUDED
#define GAME_CONFIG_H_INCLUDED

class config;
class version_info;
class color_range;

#include "tstring.hpp"

#include <SDL_types.h>

#include <string>
#include <vector>
#include <map>

//basic game configuration information is here.
namespace game_config
{
	extern int base_income;
	extern int village_income;
	extern int poison_amount;
	extern int rest_heal_amount;
	extern int recall_cost;
	extern int kill_experience;
	extern unsigned lobby_network_timer;
	extern unsigned lobby_refresh;
	extern const std::string version;
	extern const std::string revision;

	inline int kill_xp(int level)
	{
		return level ? kill_experience * level : kill_experience / 2;
	}

	extern std::string wesnoth_program_dir;

	/** Default percentage gold carried over to the next scenario. */
	extern const int gold_carryover_percentage;

	extern bool debug, editor, ignore_replay_errors, mp_debug, exit_at_end, no_delay, small_gui, disable_autosave;

	extern int cache_compression_level;

	extern std::string path;
	extern std::string preferences_dir;

	struct server_info {
		server_info() : name(""), address("") { }
		std::string name;
		std::string address; /**< may include ':' followed by port number */
	};
	extern std::vector<server_info> server_list;

	extern std::string game_icon, game_title, game_logo, title_music, lobby_music,
		moved_ball_image, unmoved_ball_image, partmoved_ball_image,
		enemy_ball_image, ally_ball_image, energy_image,
		flag_image, flag_icon_image,
		terrain_mask_image,
		grid_image_top, grid_image_bottom,
		unreachable_image, linger_image,
		shroud_prefix, fog_prefix,
		observer_image, tod_bright_image, tod_dark_image,
		checked_menu_image, unchecked_menu_image, wml_menu_image, level_image,
		ellipsis_image, default_victory_music, default_defeat_music;

	extern double hp_bar_scaling, xp_bar_scaling;

	extern std::string flag_rgb;
	extern std::vector<Uint32> red_green_scale;
	extern std::vector<Uint32> red_green_scale_text;

	extern std::vector<std::string> foot_speed_prefix;
	extern std::string foot_teleport_enter, foot_teleport_exit;

	extern int title_logo_x, title_logo_y, title_buttons_x, title_buttons_y, title_buttons_padding, title_tip_x, title_tip_width, title_tip_padding;

	extern std::map<std::string, color_range> team_rgb_range;
	extern std::map<std::string, t_string> team_rgb_name;
	extern std::map<std::string, std::vector<Uint32> > team_rgb_colors;

	/** observer team name used for observer team chat */
	extern const std::string observer_team_name;

	/**
	 * The maximum number of hexes on a map and items in an array and also used
	 * as maximum in wml loops.
	 */
	extern const size_t max_loop;

	namespace sounds {
		extern const std::string turn_bell, timer_bell, receive_message,
				receive_message_highlight, receive_message_friend,
				receive_message_server, user_arrive, user_leave,
				game_user_arrive, game_user_leave;
		extern const std::string button_press, checkbox_release, slider_adjust,
				menu_expand, menu_contract, menu_select;
	}

	void load_config(const config* cfg);

	void add_color_info(const config& v);
	const std::vector<Uint32>& tc_info(const std::string& name);
	const color_range& color_info(const std::string& name);

	/**
	 * Return a color corresponding to the value val
	 * red for val=0 to green for val=100, passing by yellow.
	 * Colors are defined by [game_config] keys
	 * red_green_scale and red_green_scale_text
	 */

	Uint32 red_to_green(int val, bool for_text = true);

	extern const version_info wesnoth_version;
	extern const version_info min_savegame_version;
	extern const std::string  test_version;
}

#endif
