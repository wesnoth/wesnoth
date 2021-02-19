/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

class config;
class color_range;

#include "color.hpp"
#include "tstring.hpp"
#include "game_config_view.hpp"

#include <vector>
#include <map>
#include <cstdint>

//basic game configuration information is here.
namespace game_config
{
	extern int base_income;
	extern int village_income;
	extern int village_support;
	extern int poison_amount;
	extern int rest_heal_amount;
	extern int recall_cost;
	extern int kill_experience;
	extern int combat_experience;
	extern unsigned int tile_size;
	extern unsigned lobby_network_timer;
	extern unsigned lobby_refresh;
	extern const std::string default_title_string;
	extern std::string default_terrain;

	extern std::vector<unsigned int> zoom_levels;

	inline int kill_xp(int level)
	{
		return level ? kill_experience * level : kill_experience / 2;
	}

	inline int combat_xp(int level)
	{
		return combat_experience * level;
	}

	extern std::string wesnoth_program_dir;

	/** Default percentage gold carried over to the next scenario. */
	extern const int gold_carryover_percentage;

	extern bool debug_lua, strict_lua, editor, ignore_replay_errors, mp_debug,
		exit_at_end, no_delay, disable_autosave, no_addons;

	extern const bool& debug;
	void set_debug(bool new_debug);

	extern int cache_compression_level;

	extern std::string path;
	extern std::string default_preferences_path;

	struct server_info
	{
		std::string name;
		std::string address; /**< may include ':' followed by port number */
	};

	extern std::vector<server_info> server_list;

	extern std::string title_music,
		lobby_music;

	extern std::vector<std::string> default_defeat_music;
	extern std::vector<std::string> default_victory_music;

	namespace colors {
	extern std::string ally_orb_color;
	extern std::string disengaged_orb_color;
	extern std::string enemy_orb_color;
	extern std::string moved_orb_color;
	extern std::string partial_orb_color;
	extern std::string unmoved_orb_color;
	extern std::string default_color_list;
	} // colors

	extern bool show_ally_orb;
	extern bool show_disengaged_orb;
	extern bool show_enemy_orb;
	extern bool show_moved_orb;
	extern bool show_partial_orb;
	extern bool show_unmoved_orb;

	namespace images {
	extern std::string game_title,
			game_title_background,
			game_logo,
			game_logo_background,
			victory_laurel,
			victory_laurel_hardest,
			victory_laurel_easy,
			// orbs and hp/xp bar
			orb,
			orb_two_color,
			energy,
			// top bar icons
			battery_icon,
			time_icon,
			// flags
			flag,
			flag_icon,
			// hex overlay
			terrain_mask,
			grid_top,
			grid_bottom,
			mouseover,
			selected,
			editor_brush,
			unreachable,
			linger,
			// GUI elements
			observer,
			tod_bright,
			tod_dark,
			selected_menu,
			deselected_menu,
			checked_menu,
			unchecked_menu,
			wml_menu,
			level,
			ellipsis,
			missing,
			// notifications icon
			app_icon;
	} //images


	extern std::string shroud_prefix, fog_prefix;

	extern double hp_bar_scaling, xp_bar_scaling;
	extern double hex_brightening;
	extern double hex_semi_brightening;

	extern std::string flag_rgb, unit_rgb;
	extern std::vector<color_t> red_green_scale;
	extern std::vector<color_t> red_green_scale_text;

	extern std::vector<std::string> foot_speed_prefix;
	extern std::string foot_teleport_enter, foot_teleport_exit;

	/**
	 * Colors defined by WML [color_range] tags. In addition to team colors such as "red" and
	 * "blue", this also contains the colors used on the minimap for "cave", "fungus" etc.
	 */
	extern std::map<std::string, color_range> team_rgb_range;
	extern std::map<std::string, t_string> team_rgb_name;
	extern std::map<std::string, std::vector<color_t>> team_rgb_colors;

	extern std::vector<std::string> default_colors;

	/** observer team name used for observer team chat */
	extern const std::string observer_team_name;

	/**
	 * The maximum number of hexes on a map and items in an array and also used
	 * as maximum in wml loops.
	 * WARNING : This should not be set to less than the max map area
	 */
	extern const std::size_t max_loop;

	namespace sounds {
		extern std::string turn_bell, timer_bell, public_message,
				private_message, friend_message,
				server_message, player_joins, player_leaves,
				game_user_arrive, game_user_leave, ready_for_start,
				game_has_begun, game_created;
		extern const std::string button_press, checkbox_release, slider_adjust,
				menu_expand, menu_contract, menu_select;
		namespace status {
			extern std::string poisoned, slowed, petrified;
		}
	}

	void load_config(const config &cfg);

	void add_color_info(const game_config_view& v);
	void reset_color_info();
	const std::vector<color_t>& tc_info(const std::string& name);
	const color_range& color_info(const std::string& name);

	/**
	 * Return a color corresponding to the value val
	 * red for val=0 to green for val=100, passing by yellow.
	 * Colors are defined by [game_config] keys
	 * red_green_scale and red_green_scale_text
	 */

	color_t red_to_green(int val, bool for_text = true);
	color_t blue_to_white(int val, bool for_text = true);

	std::string get_default_title_string();
}
