/*
   Copyright (C) 2003 - 2015 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"
#include "game_config.hpp"

#include "color_range.hpp"
#include "config.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "util.hpp"
#include "version.hpp"
#include "wesconfig.h"
#include "serialization/string_utils.hpp"
#ifdef LOAD_REVISION
#include "revision.h"
#endif

#include <boost/foreach.hpp>

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

namespace game_config
{
	int base_income = 2;
	int village_income = 1;
	int village_support = 1;
	int poison_amount= 8;
	int rest_heal_amount= 2;
	int recall_cost = 20;
	int kill_experience = 8;
	int tile_size = 72;
	unsigned lobby_network_timer = 100;
	unsigned lobby_refresh = 4000;
	const int gold_carryover_percentage = 80;
	const std::string version = VERSION;
	std::string default_terrain;
#ifdef REVISION
	const std::string revision = VERSION " (" REVISION ")";
#elif defined(VCS_SHORT_HASH) && defined(VCS_WC_MODIFIED)
	const std::string revision
		= std::string(VERSION) + " ("
		+ VCS_SHORT_HASH + (VCS_WC_MODIFIED ? "-Modified" : "-Clean") + ")";
#else
	const std::string revision = VERSION;
#endif
	std::string wesnoth_program_dir;
	bool debug = false, debug_lua = false, editor = false,
		ignore_replay_errors = false, mp_debug = false, exit_at_end = false,
		no_delay = false, disable_autosave = false, no_addons = false;

	int cache_compression_level = 6;

	std::string title_music,
			lobby_music,
			default_victory_music,
			default_defeat_music;

	namespace colors {
	std::string moved_orb_color,
			unmoved_orb_color,
			partial_orb_color,
			enemy_orb_color,
			ally_orb_color;
	}

	bool show_ally_orb, show_enemy_orb, show_moved_orb, show_partial_orb, show_unmoved_orb;

	namespace images {
	std::string game_title,
			game_title_background,
			// orbs and hp/xp bar
			orb,
			energy,
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
			///@todo de-hardcode this
			selected_menu = "buttons/radiobox-pressed.png",
			deselected_menu = "buttons/radiobox.png",
			checked_menu = "buttons/checkbox-pressed.png",
			unchecked_menu = "buttons/checkbox.png",
			wml_menu = "buttons/WML-custom.png",
			level,
			ellipsis,
			missing,
			// notifications icon
			app_icon = "images/icons/icon-game.png";
	} //images

	std::string shroud_prefix, fog_prefix;

	std::string flag_rgb;
	std::vector<Uint32> red_green_scale;
	std::vector<Uint32> red_green_scale_text;

	static std::vector<Uint32> blue_white_scale;
	static std::vector<Uint32> blue_white_scale_text;

	double hp_bar_scaling = 0.666;
	double xp_bar_scaling = 0.5;

	double hex_brightening = 1.25;

	std::vector<std::string> foot_speed_prefix;
	std::string foot_teleport_enter;
	std::string foot_teleport_exit;

	std::map<std::string, color_range > team_rgb_range;
	std::map<std::string, t_string > team_rgb_name;

	std::map<std::string, std::vector<Uint32> > team_rgb_colors;

	const version_info wesnoth_version(VERSION);
	const version_info min_savegame_version(MIN_SAVEGAME_VERSION);
	const version_info test_version("test");

	const std::string observer_team_name = "observer";

	const size_t max_loop = 65536;

	namespace sounds {
		std::string turn_bell = "bell.wav",
		timer_bell = "timer.wav",
		public_message = "chat-[1~3].ogg",
		private_message = "chat-highlight.ogg",
		friend_message = "chat-friend.ogg",
		server_message = "receive.wav",
		player_joins = "arrive.wav",
		player_leaves = "leave.wav",
		game_user_arrive = "join.wav",
		game_user_leave = "leave.wav",
		ready_for_start = "bell.wav",
		game_has_begun = "gamestart.ogg";

		const std::string button_press = "button.wav",
		checkbox_release = "checkbox.wav",
		slider_adjust = "slider.wav",
		menu_expand = "expand.wav",
		menu_contract = "contract.wav",
		menu_select = "select.wav";

		namespace status {
			std::string poisoned = "poison.ogg",
			slowed = "slowed.wav",
			petrified = "petrified.ogg";
		}
	}

#ifdef WESNOTH_PATH
	std::string path = WESNOTH_PATH;
#else
	std::string path = "";
#endif

#ifdef DEFAULT_PREFS_PATH
	std::string default_preferences_path = DEFAULT_PREFS_PATH;
#else
	std::string default_preferences_path = "";
#endif

	std::vector<server_info> server_list;

	void load_config(const config &v)
	{
		base_income = v["base_income"].to_int(2);
		village_income = v["village_income"].to_int(1);
		village_support = v["village_support"].to_int(1);
		poison_amount = v["poison_amount"].to_int(8);
		rest_heal_amount = v["rest_heal_amount"].to_int(2);
		recall_cost = v["recall_cost"].to_int(20);
		kill_experience = v["kill_experience"].to_int(8);
		lobby_refresh = v["lobby_refresh"].to_int(2000);
		default_terrain = v["default_terrain"].str();
		tile_size = v["tile_size"].to_int(72);

		title_music = v["title_music"].str();
		lobby_music = v["lobby_music"].str();
		default_victory_music = v["default_victory_music"].str();
		default_defeat_music = v["default_defeat_music"].str();

		if(const config &i = v.child("colors")){
			using namespace game_config::colors;
			moved_orb_color = i["moved_orb_color"].str();
			unmoved_orb_color = i["unmoved_orb_color"].str();
			partial_orb_color = i["partial_orb_color"].str();
			enemy_orb_color = i["enemy_orb_color"].str();
			ally_orb_color = i["ally_orb_color"].str();
		} // colors

		show_ally_orb = v["show_ally_orb"].to_bool(true);
		show_enemy_orb = v["show_enemy_orb"].to_bool(false);
		show_moved_orb = v["show_moved_orb"].to_bool(true);
		show_partial_orb  = v["show_partly_orb"].to_bool(true);
		show_unmoved_orb = v["show_unmoved_orb"].to_bool(true);

		if(const config &i = v.child("images")){
			using namespace game_config::images;
			game_title = i["game_title"].str();
			game_title_background = i["game_title_background"].str();

			orb = i["orb"].str();
			energy = i["energy"].str();

			flag = i["flag"].str();
			flag_icon = i["flag_icon"].str();

			terrain_mask = i["terrain_mask"].str();
			grid_top = i["grid_top"].str();
			grid_bottom = i["grid_bottom"].str();
			mouseover = i["mouseover"].str();
			selected = i["selected"].str();
			editor_brush = i["editor_brush"].str();
			unreachable = i["unreachable"].str();
			linger = i["linger"].str();

			observer = i["observer"].str();
			tod_bright = i["tod_bright"].str();
			tod_dark = i["tod_dark"].str();
			level = i["level"].str();
			ellipsis = i["ellipsis"].str();
			missing = i["missing"].str();
		} // images

		hp_bar_scaling = v["hp_bar_scaling"].to_double(0.666);
		xp_bar_scaling = v["xp_bar_scaling"].to_double(0.5);
		hex_brightening = v["hex_brightening"].to_double(1.25);

		foot_speed_prefix = utils::split(v["footprint_prefix"]);
		foot_teleport_enter = v["footprint_teleport_enter"].str();
		foot_teleport_exit = v["footprint_teleport_exit"].str();

		shroud_prefix = v["shroud_prefix"].str();
		fog_prefix  = v["fog_prefix"].str();

		add_color_info(v);

		if (const config::attribute_value *a = v.get("flag_rgb")) {
			flag_rgb = a->str();
		}

		std::string color_string = v["red_green_scale"].str();
		if(!string2rgb(color_string, red_green_scale)) {
			ERR_NG << "can't parse color string red_green_scale, ignoring: " << color_string << std::endl;
		}
		if (red_green_scale.empty()) {
			red_green_scale.push_back(0x00FFFF00);
		}

		color_string = v["red_green_scale_text"].str();
		if(!string2rgb(color_string, red_green_scale_text)) {
			ERR_NG << "can't parse color string red_green_scale_text, ignoring: " << color_string << std::endl;
		}
		if (red_green_scale_text.empty()) {
			red_green_scale_text.push_back(0x00FFFF00);
		}

		color_string = v["blue_white_scale"].str();
		if(!string2rgb(color_string, blue_white_scale)) {
			ERR_NG << "can't parse color string blue_white_scale, ignoring: " << color_string << std::endl;
		}
		if (blue_white_scale.empty()) {
			blue_white_scale.push_back(0x00FFFFFF);
		}

		color_string = v["blue_white_scale_text"].str();
		if(!string2rgb(color_string, blue_white_scale_text)) {
			ERR_NG << "can't parse color string blue_white_scale_text, ignoring: " << color_string << std::endl;
		}
		if (blue_white_scale_text.empty()) {
			blue_white_scale_text.push_back(0x00FFFFFF);
		}

		server_list.clear();
		BOOST_FOREACH(const config &server, v.child_range("server"))
		{
			server_info sinf;
			sinf.name = server["name"].str();
			sinf.address = server["address"].str();
			server_list.push_back(sinf);
		}

		if(const config & s = v.child("sounds")) {
			using namespace game_config::sounds;
			if (s.has_attribute("turn_bell")) 		turn_bell = 			s["turn_bell"].str();
			if (s.has_attribute("timer_bell")) 		timer_bell = 			s["timer_bell"].str();
			if (s.has_attribute("public_message")) 		public_message = 		s["public_message"].str();
			if (s.has_attribute("private_message")) 	private_message = 		s["private_message"].str();
			if (s.has_attribute("friend_message")) 		friend_message = 		s["friend_message"].str();
			if (s.has_attribute("server_message")) 		server_message = 		s["server_message"].str();
			if (s.has_attribute("player_joins")) 		player_joins = 			s["player_joins"].str();
			if (s.has_attribute("player_leaves")) 		player_leaves = 		s["player_leaves"].str();
			if (s.has_attribute("game_user_arrive"))	game_user_arrive = 		s["game_user_arrive"].str();
			if (s.has_attribute("game_user_leave")) 	game_user_leave = 		s["game_user_leave"].str();
			if (s.has_attribute("ready_for_start")) 	ready_for_start = 		s["ready_for_start"].str();
			if (s.has_attribute("game_has_begun")) 		game_has_begun = 		s["game_has_begun"].str();

			if(const config & ss = s.child("status")) {
				using namespace game_config::sounds::status;
				if (ss.has_attribute("poisoned")) 		poisoned = 		ss["poisoned"].str();
				if (ss.has_attribute("slowed")) 		slowed = 		ss["slowed"].str();
				if (ss.has_attribute("petrified")) 		petrified = 		ss["petrified"].str();
			}
		}
	}

	void add_color_info(const config &v)
	{
		BOOST_FOREACH(const config &teamC, v.child_range("color_range"))
		{
			const config::attribute_value *a1 = teamC.get("id"),
				*a2 = teamC.get("rgb");
			if (!a1 || !a2) {
				continue;
			}
			std::string id = *a1;
			std::vector<Uint32> temp;
			if(!string2rgb(*a2, temp)) {
				std::stringstream ss;
				ss << "can't parse color string:\n" << teamC.debug() << "\n";
				throw config::error(ss.str());
			}
			team_rgb_range.insert(std::make_pair(id,color_range(temp)));
			team_rgb_name[id] = teamC["name"];

			LOG_NG << "registered color range '" << id << "': " << team_rgb_range[id].debug() << '\n';

			//generate palette of same name;
			std::vector<Uint32> tp = palette(team_rgb_range[id]);
			if (tp.empty()) {
				continue;
			}
			team_rgb_colors.insert(std::make_pair(id,tp));
		}

		BOOST_FOREACH(const config &cp, v.child_range("color_palette"))
		{
			BOOST_FOREACH(const config::attribute &rgb, cp.attribute_range())
			{
				std::vector<Uint32> temp;
				if(!string2rgb(rgb.second, temp)) {
					ERR_NG << "Invalid color palette: " << rgb.second << std::endl;
				}
				team_rgb_colors.insert(std::make_pair(rgb.first, temp));
				LOG_NG << "registered color palette: " << rgb.first << '\n';
			}
		}
	}

	const color_range& color_info(const std::string& name)
	{
		std::map<std::string, color_range>::const_iterator i = team_rgb_range.find(name);
		if(i == team_rgb_range.end()) {
			std::vector<Uint32> temp;
			if(!string2rgb(name, temp)) {
				throw config::error(_("Invalid color range: ") + name);
			}
			team_rgb_range.insert(std::make_pair(name,color_range(temp)));
			return color_info(name);
		}
		return i->second;
	}

	const std::vector<Uint32>& tc_info(const std::string& name)
	{
		std::map<std::string, std::vector<Uint32> >::const_iterator i = team_rgb_colors.find(name);
		if(i == team_rgb_colors.end()) {
			std::vector<Uint32> temp;
			if(!string2rgb(name, temp)) {
				static std::vector<Uint32> stv;
				ERR_NG << "Invalid color palette: " << name << std::endl;
				return stv;
			}
			team_rgb_colors.insert(std::make_pair(name,temp));
			return tc_info(name);
		}
		return i->second;
	}

	Uint32 red_to_green(int val, bool for_text){
		const std::vector<Uint32>& color_scale =
				for_text ? red_green_scale_text : red_green_scale;
		val = std::max<int>(0, std::min<int>(val, 100));
		int lvl = (color_scale.size()-1) * val / 100;
		return color_scale[lvl];
	}

	Uint32 blue_to_white(int val, bool for_text){
		const std::vector<Uint32>& color_scale =
				for_text ? blue_white_scale_text : blue_white_scale;
		val = std::max<int>(0, std::min<int>(val, 100));
		int lvl = (color_scale.size()-1) * val / 100;
		return color_scale[lvl];
	}

} // game_config
