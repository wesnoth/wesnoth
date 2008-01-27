/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "config.hpp"
#include "log.hpp"
#include "gettext.hpp"
#include "game_config.hpp"
#include "util.hpp"
#include "serialization/string_utils.hpp"
#include "wesconfig.h"
#include "revision_stamp.hpp"

#include <cstdlib>
#include <sstream>

#define DBG_NG LOG_STREAM(debug, engine)
#define ERR_NG LOG_STREAM(err, engine)

namespace game_config
{
	int base_income = 2;
	int village_income = 1;
	int poison_amount= 8;
	int rest_heal_amount= 2;
	int recall_cost = 20;
	int kill_experience = 8;
	unsigned lobby_refresh = 2000;
	const int gold_carryover_percentage = 80;
	const bool gold_carryover_add = false;
  	const std::string version = VERSION;
	const std::string svnrev = SVNREV;
	const std::string revision = (svnrev.empty() || svnrev == "exported" ? VERSION : VERSION " (" SVNREV ")");
	bool debug = false, editor = false, ignore_replay_errors = false, mp_debug = false, exit_at_end = false, no_delay = false, disable_autosave = false;

	std::string game_icon = "wesnoth-icon.png", game_title, game_logo, title_music;
	int title_logo_x = 0, title_logo_y = 0, title_buttons_x = 0, title_buttons_y = 0, title_buttons_padding = 0,
	    title_tip_x = 0, title_tip_width = 0, title_tip_padding = 0;

	std::string terrain_mask_image = "terrain/alphamask.png";
	std::string grid_image = "terrain/grid.png";
	std::string unreachable_image = "terrain/darken.png";    //!< overlay image for unreachable tiles
	std::string linger_image = "terrain/darken-linger.png";  //!< overlay image for tiles in linger mode

	std::string energy_image = "misc/bar-energy.png";
	std::string moved_ball_image = "misc/ball-moved.png";
	std::string unmoved_ball_image = "misc/ball-unmoved.png";
	std::string partmoved_ball_image = "misc/ball-partmoved.png";
	std::string enemy_ball_image = "misc/ball-enemy.png";
	std::string ally_ball_image = "misc/ball-ally.png";
	std::string flag_image = "flags/flag-1.png:150,flags/flag-2.png:150,flags/flag-3.png:150,flags/flag-4.png:150";
	std::string flag_icon_image = "flags/flag_icon.png";
	std::string flag_rgb = "flag_green";
	std::vector<Uint32> defense_color_scale;

	std::string cross_image = "misc/cross.png";

	std::vector<std::string> foot_speed_prefix;
	std::string foot_teleport_enter = "footsteps/teleport-in.png";
	std::string foot_teleport_exit = "footsteps/teleport-out.png";

	std::string observer_image = "misc/eye.png";
	std::string tod_bright_image = "misc/tod-bright.png";
	std::string unchecked_menu_image = "buttons/checkbox.png";
	std::string checked_menu_image = "buttons/checkbox-pressed.png";
	std::string wml_menu_image = "buttons/WML-custom.png";

	std::string level_image;
	std::string ellipsis_image;

	std::map<std::string, color_range > team_rgb_range;
	std::map<std::string, t_string > team_rgb_name;

	std::map<std::string, std::vector<Uint32> > team_rgb_colors;

	const struct game_version wesnoth_version(VERSION);
	const struct game_version min_savegame_version(MIN_SAVEGAME_VERSION);
	const struct game_version test_version("test");

	const std::string observer_team_name = "observer";

	namespace sounds {
		const std::string turn_bell = "bell.wav",
		timer_bell = "timer.wav",
		receive_message = "chat-1.ogg,chat-2.ogg,chat-3.ogg",
		receive_message_highlight = "chat-highlight.ogg",
		receive_message_friend = "chat-friend.ogg",
		receive_message_server = "receive.wav",
		user_arrive = "arrive.wav",
		user_leave = "leave.wav";

		const std::string button_press = "button.wav",
		checkbox_release = "checkbox.wav",
		slider_adjust = "slider.wav",
		menu_expand = "expand.wav",
		menu_contract = "contract.wav",
		menu_select = "select.wav";
	}



#ifdef __AMIGAOS4__
	std::string path = "PROGDIR:";
#else
#ifdef WESNOTH_PATH
	std::string path = WESNOTH_PATH;
#else
	std::string path = "";
#endif
#endif
	std::vector<server_info> server_list;

	void load_config(const config* cfg)
	{
		if(cfg == NULL)
			return;

		const config& v = *cfg;

		base_income = lexical_cast_default<int>(v["base_income"], 2);
		village_income = lexical_cast_default<int>(v["village_income"], 1);
		poison_amount = lexical_cast_default<int>(v["poison_amount"], 8);
		rest_heal_amount = lexical_cast_default<int>(v["rest_heal_amount"], 2);
		recall_cost = lexical_cast_default<int>(v["recall_cost"], 20);
		kill_experience = lexical_cast_default<int>(v["kill_experience"], 8);
		lobby_refresh = lexical_cast_default<unsigned>(v["lobby_refresh"], 2000);

		game_icon = v["icon"];
		game_title = v["title"];
		game_logo = v["logo"];
		title_music = v["title_music"];

		title_logo_x = lexical_cast_default<int>(v["logo_x"]);
		title_logo_y = lexical_cast_default<int>(v["logo_y"]);
		title_buttons_x = lexical_cast_default<int>(v["buttons_x"]);
		title_buttons_y = lexical_cast_default<int>(v["buttons_y"]);
		title_buttons_padding = lexical_cast_default<int>(v["buttons_padding"]);

		title_tip_x = lexical_cast_default<int>(v["tip_x"]);
		title_tip_width = lexical_cast_default<int>(v["tip_width"]);
		title_tip_padding = lexical_cast_default<int>(v["tip_padding"]);


		energy_image = v["energy_image"];
		moved_ball_image = v["moved_ball_image"];
		unmoved_ball_image = v["unmoved_ball_image"];
		partmoved_ball_image = v["partmoved_ball_image"];
		enemy_ball_image = v["enemy_ball_image"];
		ally_ball_image = v["ally_ball_image"];
		flag_image = v["flag_image"];
		flag_icon_image = v["flag_icon_image"];
		cross_image = v["cross_image"];

		foot_speed_prefix = utils::split(v["footprint_prefix"]);
		foot_teleport_enter = v["footprint_teleport_enter"];
		foot_teleport_exit = v["footprint_teleport_exit"];

		terrain_mask_image = v["terrain_mask_image"];
		grid_image = v["grid_image"];
		unreachable_image = v["unreachable_image"];

		observer_image = v["observer_image"];
		tod_bright_image = v["tod_bright_image"];

		level_image = v["level_image"];
		ellipsis_image = v["ellipsis_image"];

		add_color_info(v);

		flag_rgb = v["flag_rgb"];
		if( !flag_rgb.size()){
			flag_rgb="flag_green";
		}
		defense_color_scale = string2rgb(v["defense_color_scale"]);
		if (defense_color_scale.empty()) {
			defense_color_scale.push_back(0x00FFFF00);
		}

		server_list.clear();
		const std::vector<config *> &servers = v.get_children("server");
		std::vector<config *>::const_iterator server;
		for(server = servers.begin(); server != servers.end(); ++server) {
			server_info sinf;
			sinf.name = (**server)["name"];
			sinf.address = (**server)["address"];
			server_list.push_back(sinf);
		}
	}

	const void add_color_info(const config& v){
		const config::child_list& team_colors = v.get_children("color_range");
		for(config::child_list::const_iterator teamC = team_colors.begin(); teamC != team_colors.end(); ++teamC) {
			if(!(**teamC)["id"].empty() && !(**teamC)["rgb"].empty()){
				std::string id = (**teamC)["id"];
				std::vector<Uint32> temp = string2rgb((**teamC)["rgb"]);
				team_rgb_range.insert(std::make_pair(id,color_range(temp)));
				team_rgb_name[id] = (**teamC)["name"];
				//generate palette of same name;
				std::vector<Uint32> tp = palette(team_rgb_range[id]);
				if(tp.size()){
					team_rgb_colors.insert(std::make_pair(id,tp));
					//if this is being used, output log of palette for artists use.
					DBG_NG << "color palette creation:\n";
					std::stringstream str;
					str << id <<" = ";
					for(std::vector<Uint32>::const_iterator r=tp.begin();r!=tp.end();r++){
						int red = ((*r) & 0x00FF0000)>>16;
						int green = ((*r) & 0x0000FF00)>>8;
						int blue = ((*r) & 0x000000FF);
						if(r!=tp.begin()) {
							str<<",";
						}
						str << red << "," << green << "," << blue;
					}
					DBG_NG << str.str() <<"\n";
				}
			}
		}

		const config::child_list& colors = v.get_children("color_palette");
		for(config::child_list::const_iterator cp = colors.begin(); cp != colors.end(); ++cp) {
			for(string_map::const_iterator rgb_it = (*cp)->values.begin(); rgb_it != (*cp)->values.end(); ++rgb_it) {
				try {
					team_rgb_colors.insert(std::make_pair(rgb_it->first,string2rgb(rgb_it->second)));
				} catch(bad_lexical_cast&) {
					//throw config::error(_("Invalid team color: ") + rgb_it->second);
					ERR_NG << "Invalid team color: " << rgb_it->second << "\n";
				}
			}
		}
	}

	const color_range& color_info(const std::string& name)
	{
		std::map<std::string, color_range>::const_iterator i = team_rgb_range.find(name);
		if(i == team_rgb_range.end()) {
			try {
				team_rgb_range.insert(std::make_pair(name,color_range(string2rgb(name))));
				return color_info(name);
			} catch(bad_lexical_cast&) {
				//ERR_NG << "Invalid color range: " << name;
				//return color_info();
				throw config::error(_("Invalid color range: ") + name);
			}
		}
		return i->second;
	}

	const std::vector<Uint32>& tc_info(const std::string& name)
	{
		std::map<std::string, std::vector<Uint32> >::const_iterator i = team_rgb_colors.find(name);
		if(i == team_rgb_colors.end()) {
			try {
				team_rgb_colors.insert(std::make_pair(name,string2rgb(name)));
				return tc_info(name);
			} catch(bad_lexical_cast&) {
				static std::vector<Uint32> stv;
				//throw config::error(_("Invalid team color: ") + name);
				ERR_NG << "Invalid team color: " << name << "\n";
				return stv;
			}
		}
		return i->second;
	}

game_version::game_version(std::string str) :
		major_nr(0), minor_nr(0), patch(0), extra(""), full(str)
{

	size_t offset = str.find_first_not_of("0123456789");
	major_nr = lexical_cast_default<unsigned int>(std::string(str, 0, offset), 0);
	str.erase(0, offset + 1);

	if(! str.empty()) {
		offset = str.find_first_not_of("0123456789");
		minor_nr = lexical_cast_default<unsigned int>(std::string(str, 0, offset ), 0);
		str.erase(0, offset + 1);
	}

	if(! str.empty()) {
		offset = str.find_first_not_of("0123456789");
		patch = lexical_cast_default<unsigned int>(std::string(str, 0, offset ), 0);
		if(offset != std::string::npos) {
			extra = std::string(str, offset, std::string::npos);
		}
	}
}

bool operator<(const struct game_version& a, const struct game_version& b)
{
	if(a.major_nr != b.major_nr) return a.major_nr < b.major_nr;
	if(a.minor_nr != b.minor_nr) return a.minor_nr < b.minor_nr;
	return a.patch < b.patch;
}

bool operator<=(const struct game_version& a, const struct game_version& b)
{
	return a < b || a == b;
}

bool operator>(const struct game_version& a, const struct game_version& b)
{
	return !(a <= b);
}

bool operator>=(const struct game_version& a, const struct game_version& b)
{
	return !(a < b);
}

bool operator==(const struct game_version& a, const struct game_version& b)
{
	return a.full == b.full;
}

bool operator!=(const struct game_version& a, const struct game_version& b)
{
	return a.full != b.full;
}

} // game_config
