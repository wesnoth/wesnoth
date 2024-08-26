/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "game_config.hpp"

#include "color_range.hpp"
#include "config.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "game_version.hpp"
#include "serialization/string_utils.hpp"

#include <cmath>
#include <random>

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

namespace game_config
{
//
// Gameplay constants
//
int base_income      = 2;
int village_income   = 1;
int village_support  = 1;
int recall_cost      = 20;
int kill_experience  = 8;
int combat_experience  = 1;

int poison_amount    = 8;
int rest_heal_amount = 2;

const int gold_carryover_percentage = 80;

//
// Terrain-related constants
//
unsigned int tile_size = 72;

std::string default_terrain;
std::string shroud_prefix, fog_prefix;

std::vector<unsigned int> zoom_levels {36, 72, 144};

//
// Display scale constants
//
double hp_bar_scaling  = 0.666;
double xp_bar_scaling  = 0.5;

//
// Misc
//
unsigned lobby_network_timer  = 100;
unsigned lobby_refresh        = 4000;

const std::size_t max_loop = 65536;

std::vector<server_info> server_list;

bool allow_insecure = false;

//
// Gamestate flags
//
bool
	debug_impl           = false,
	debug_lua            = false,
	strict_lua           = false,
	editor               = false,
	ignore_replay_errors = false,
	mp_debug             = false,
	exit_at_end          = false,
	no_delay             = false,
	disable_autosave     = false,
	no_addons            = false;

const bool& debug = debug_impl;

void set_debug(bool new_debug) {
    // TODO: remove severity static casts and fix issue #7894
	if(debug_impl && !new_debug) {
		// Turning debug mode off; decrease deprecation severity
		lg::severity severity;
		if(lg::get_log_domain_severity("deprecation", severity)) {
            int severityInt = static_cast<int>(severity);
			lg::set_log_domain_severity("deprecation", static_cast<lg::severity>(severityInt - 2));
		}
	} else if(!debug_impl && new_debug) {
		// Turning debug mode on; increase deprecation severity
        lg::severity severity;
		if(lg::get_log_domain_severity("deprecation", severity)) {
            int severityInt = static_cast<int>(severity);
			lg::set_log_domain_severity("deprecation", static_cast<lg::severity>(severityInt + 2));
		}
	}
	debug_impl = new_debug;
}

//
// Orb display flags
//
bool show_ally_orb;
bool show_disengaged_orb;
bool show_enemy_orb;
bool show_moved_orb;
bool show_partial_orb;
bool show_status_on_ally_orb;
bool show_unmoved_orb;

//
// Music constants
//
std::string title_music, lobby_music;

std::vector<std::string> default_defeat_music;
std::vector<std::string> default_victory_music;

//
// Color info
//
std::string flag_rgb, unit_rgb;

std::vector<color_t> red_green_scale;
std::vector<color_t> red_green_scale_text;

static std::vector<color_t> blue_white_scale;
static std::vector<color_t> blue_white_scale_text;

std::map<std::string, color_range, std::less<>> team_rgb_range;
// Map [color_range]id to [color_range]name, or "" if no name
std::map<std::string, t_string, std::less<>> team_rgb_name;

std::map<std::string, std::vector<color_t>, std::less<>> team_rgb_colors;

std::vector<std::string> default_colors;

namespace colors
{
std::string ally_orb_color;
std::string enemy_orb_color;
std::string moved_orb_color;
std::string partial_orb_color;
std::string unmoved_orb_color;
std::string default_color_list;
} // namespace colors

//
// Image constants
//
std::vector<std::string> foot_speed_prefix;

std::string foot_teleport_enter;
std::string foot_teleport_exit;

namespace images {

std::string
	game_title,
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
	// TODO: de-hardcode this
	selected_menu   = "buttons/radiobox-pressed.png",
	deselected_menu = "buttons/radiobox.png",
	checked_menu    = "buttons/checkbox-pressed.png",
	unchecked_menu  = "buttons/checkbox.png",
	wml_menu        = "buttons/WML-custom.png",
	level,
	ellipsis,
	missing,
	blank,
	// notifications icon
	app_icon = "images/icons/icon-game.png";

} //images

//
// Sound constants
//
namespace sounds {

std::string
	turn_bell        = "bell.wav",
	timer_bell       = "timer.wav",
	public_message   = "chat-[1~3].ogg",
	private_message  = "chat-highlight.ogg",
	friend_message   = "chat-friend.ogg",
	server_message   = "receive.wav",
	player_joins     = "arrive.wav",
	player_leaves    = "leave.wav",
	game_user_arrive = "join.wav",
	game_user_leave  = "leave.wav",
	ready_for_start  = "bell.wav",
	game_has_begun   = "gamestart.ogg",
	game_created	 = "join.wav";

const std::string
	button_press     = "button.wav",
	checkbox_release = "checkbox.wav",
	slider_adjust    = "slider.wav",
	menu_expand      = "expand.wav",
	menu_contract    = "contract.wav",
	menu_select      = "select.wav";

namespace status {

std::string
	poisoned  = "poison.ogg",
	slowed    = "slowed.wav",
	petrified = "petrified.ogg";

} // status

} // sounds

static void add_color_info(const game_config_view& v, bool build_defaults);
void add_color_info(const game_config_view& v)
{
	add_color_info(v, false);
}

void load_config(const config &v)
{
	base_income      = v["base_income"].to_int(2);
	village_income   = v["village_income"].to_int(1);
	village_support  = v["village_support"].to_int(1);
	poison_amount    = v["poison_amount"].to_int(8);
	rest_heal_amount = v["rest_heal_amount"].to_int(2);
	recall_cost      = v["recall_cost"].to_int(20);
	kill_experience  = v["kill_experience"].to_int(8);
	combat_experience= v["combat_experience"].to_int(1);
	lobby_refresh    = v["lobby_refresh"].to_int(2000);
	default_terrain  = v["default_terrain"].str();
	tile_size        = v["tile_size"].to_int(72);

	std::vector<std::string> zoom_levels_str = utils::split(v["zoom_levels"]);
	if(!zoom_levels_str.empty()) {
		zoom_levels.clear();
		std::transform(zoom_levels_str.begin(), zoom_levels_str.end(), std::back_inserter(zoom_levels), [](const std::string zoom) {
			int z = std::stoi(zoom);
			if((z / 4) * 4 != z) {
				ERR_NG << "zoom level " << z << " is not divisible by 4."
					<< " This will cause graphical glitches!";
			}
			return z;
		});
	}

	title_music           = v["title_music"].str();
	lobby_music           = v["lobby_music"].str();

	default_victory_music = utils::split(v["default_victory_music"].str());
	default_defeat_music  = utils::split(v["default_defeat_music"].str());

	if(auto i = v.optional_child("colors")){
		using namespace game_config::colors;

		moved_orb_color    = i["moved_orb_color"].str();
		unmoved_orb_color  = i["unmoved_orb_color"].str();
		partial_orb_color  = i["partial_orb_color"].str();
		enemy_orb_color    = i["enemy_orb_color"].str();
		ally_orb_color     = i["ally_orb_color"].str();
	} // colors

	show_ally_orb     = v["show_ally_orb"].to_bool(true);
	show_enemy_orb    = v["show_enemy_orb"].to_bool(false);
	show_moved_orb    = v["show_moved_orb"].to_bool(true);
	show_partial_orb  = v["show_partial_orb"].to_bool(true);
	show_status_on_ally_orb = v["show_status_on_ally_orb"].to_bool(true);
	show_unmoved_orb  = v["show_unmoved_orb"].to_bool(true);
	show_disengaged_orb = v["show_disengaged_orb"].to_bool(true);

	if(auto i = v.optional_child("images")){
		using namespace game_config::images;

		if (!i["game_title_background"].blank()) {
			// Select a background at random
			const auto backgrounds = utils::split(i["game_title_background"].str());
			if (backgrounds.size() > 1) {
				int r = rand() % (backgrounds.size());
				game_title_background = backgrounds.at(r);
			} else if (backgrounds.size() == 1) {
				game_title_background = backgrounds.at(0);
			}
		}

		// Allow game_title to be empty
		game_title            = i["game_title"].str();
		game_logo             = i["game_logo"].str();
		game_logo_background  = i["game_logo_background"].str();

		victory_laurel = i["victory_laurel"].str();
		victory_laurel_hardest = i["victory_laurel_hardest"].str();
		victory_laurel_easy = i["victory_laurel_easy"].str();

		orb    = i["orb"].str();
		orb_two_color = i["orb_two_color"].str();
		energy = i["energy"].str();

		battery_icon = i["battery_icon"].str();
		time_icon = i["time_icon"].str();

		flag      = i["flag"].str();
		flag_icon = i["flag_icon"].str();

		terrain_mask = i["terrain_mask"].str();
		grid_top     = i["grid_top"].str();
		grid_bottom  = i["grid_bottom"].str();
		mouseover    = i["mouseover"].str();
		selected     = i["selected"].str();
		editor_brush = i["editor_brush"].str();
		unreachable  = i["unreachable"].str();
		linger       = i["linger"].str();

		observer   = i["observer"].str();
		tod_bright = i["tod_bright"].str();
		tod_dark   = i["tod_dark"].str();
		level      = i["level"].str();
		ellipsis   = i["ellipsis"].str();
		missing    = i["missing"].str();
		blank      = i["blank"].str();
	} // images

	hp_bar_scaling  = v["hp_bar_scaling"].to_double(0.666);
	xp_bar_scaling  = v["xp_bar_scaling"].to_double(0.5);

	foot_speed_prefix   = utils::split(v["footprint_prefix"]);
	foot_teleport_enter = v["footprint_teleport_enter"].str();
	foot_teleport_exit  = v["footprint_teleport_exit"].str();

	shroud_prefix = v["shroud_prefix"].str();
	fog_prefix    = v["fog_prefix"].str();

	add_color_info(game_config_view::wrap(v), true);

	if(const config::attribute_value* a = v.get("flag_rgb")) {
		flag_rgb = a->str();
	}

	if(const config::attribute_value* a = v.get("unit_rgb")) {
		unit_rgb = a->str();
	}

	const auto parse_config_color_list = [&](
			const std::string& key,
			const color_t fallback)->std::vector<color_t>
	{
		std::vector<color_t> color_vec;

		for(const auto& s : utils::split(v[key].str())) {
			try {
				color_vec.push_back(color_t::from_hex_string(s));
			} catch(const std::invalid_argument& e) {
				ERR_NG << "Error parsing color list '" << key << "'.\n" << e.what();
				color_vec.push_back(fallback);
			}
		}

		return color_vec;
	};

	red_green_scale       = parse_config_color_list("red_green_scale",       {255, 255, 255});
	red_green_scale_text  = parse_config_color_list("red_green_scale_text",  {255, 255, 255});
	blue_white_scale      = parse_config_color_list("blue_white_scale",      {0  , 0  , 255});
	blue_white_scale_text = parse_config_color_list("blue_white_scale_text", {0  , 0  , 255});

	server_list.clear();

	for(const config& server : v.child_range("server")) {
		server_info sinf;
		sinf.name = server["name"].str();
		sinf.address = server["address"].str();
		server_list.push_back(sinf);
	}

	if(auto s = v.optional_child("sounds")) {
		using namespace game_config::sounds;

		const auto load_attribute = [](const config& c, const std::string& key, std::string& member) {
			if(c.has_attribute(key)) {
				member = c[key].str();
			}
		};

		load_attribute(*s, "turn_bell",        turn_bell);
		load_attribute(*s, "timer_bell",       timer_bell);
		load_attribute(*s, "public_message",   public_message);
		load_attribute(*s, "private_message",  private_message);
		load_attribute(*s, "friend_message",   friend_message);
		load_attribute(*s, "server_message",   server_message);
		load_attribute(*s, "player_joins",     player_joins);
		load_attribute(*s, "player_leaves",    player_leaves);
		load_attribute(*s, "game_created",     game_created);
		load_attribute(*s, "game_user_arrive", game_user_arrive);
		load_attribute(*s, "game_user_leave",  game_user_leave);
		load_attribute(*s, "ready_for_start",  ready_for_start);
		load_attribute(*s, "game_has_begun",   game_has_begun);

		if(auto ss = s->optional_child("status")) {
			using namespace game_config::sounds::status;

			load_attribute(*ss, "poisoned",  poisoned);
			load_attribute(*ss, "slowed",    slowed);
			load_attribute(*ss, "petrified", petrified);
		}
	}
}

void add_color_info(const game_config_view& v, bool build_defaults)
{
	if(build_defaults) {
		default_colors.clear();
	}

	for(const config& teamC : v.child_range("color_range")) {
		const config::attribute_value* a1 = teamC.get("id"), *a2 = teamC.get("rgb");
		if(!a1 || !a2) {
			continue;
		}

		std::string id = *a1;
		std::vector<color_t> temp;

		for(const auto& s : utils::split(*a2)) {
			try {
				temp.push_back(color_t::from_hex_string(s));
			} catch(const std::invalid_argument&) {
				std::stringstream ss;
				ss << "can't parse color string:\n" << teamC.debug() << "\n";
				throw config::error(ss.str());
			}
		}

		team_rgb_range.emplace(id, color_range(temp));
		team_rgb_name.emplace(id, teamC["name"].t_str());

		LOG_NG << "registered color range '" << id << "': " << team_rgb_range[id].debug();

		// Generate palette of same name;
		team_rgb_colors.emplace(id, palette(team_rgb_range[id]));

		if(build_defaults && teamC["default"].to_bool()) {
			default_colors.push_back(*a1);
		}
	}

	for(const config &cp : v.child_range("color_palette")) {
		for(const config::attribute& rgb : cp.attribute_range()) {
			std::vector<color_t> temp;
			for(const auto& s : utils::split(rgb.second)) {
				try {
					temp.push_back(color_t::from_hex_string(s));
				} catch(const std::invalid_argument& e) {
					ERR_NG << "Invalid color in palette: " << s << " (" << e.what() << ")";
				}
			}

			team_rgb_colors.emplace(rgb.first, temp);
			LOG_NG << "registered color palette: " << rgb.first;
		}
	}
}

void reset_color_info()
{
	default_colors.clear();
	team_rgb_colors.clear();
	team_rgb_name.clear();
	team_rgb_range.clear();
}

const color_range& color_info(std::string_view name)
{
	auto i = team_rgb_range.find(name);
	if(i != team_rgb_range.end()) {
		return i->second;
	}

	std::vector<color_t> temp;
	for(const auto& s : utils::split(name)) {
		try {
			temp.push_back(color_t::from_hex_string(s));
		} catch(const std::invalid_argument&) {
			throw config::error(_("Invalid color in range: ") + s);
		}
	}

	team_rgb_range.emplace(name, color_range(temp));
	return color_info(name);
}

const std::vector<color_t>& tc_info(std::string_view name)
{
	auto i = team_rgb_colors.find(name);
	if(i != team_rgb_colors.end()) {
		return i->second;
	}

	std::vector<color_t> temp;
	for(const auto& s : utils::split(name)) {
		try {
			temp.push_back(color_t::from_hex_string(s));
		} catch(const std::invalid_argument& e) {
			static std::vector<color_t> stv;
			ERR_NG << "Invalid color in palette: " << s << " (" << e.what() << ")";
			return stv;
		}
	}

	team_rgb_colors.emplace(name, temp);
	return tc_info(name);
}

color_t red_to_green(double val, bool for_text)
{
	const std::vector<color_t>& color_scale = for_text ? red_green_scale_text : red_green_scale;

	const double val_scaled = std::clamp(0.01 * val, 0.0, 1.0);
	const int lvl = int(std::nearbyint((color_scale.size() - 1) * val_scaled));

	return color_scale[lvl];
}

color_t blue_to_white(double val, bool for_text)
{
	const std::vector<color_t>& color_scale = for_text ? blue_white_scale_text : blue_white_scale;

	const double val_scaled = std::clamp(0.01 * val, 0.0, 1.0);
	const int lvl = int(std::nearbyint((color_scale.size() - 1) * val_scaled));

	return color_scale[lvl];
}

std::string get_default_title_string()
{
	std::string ret = _("The Battle for Wesnoth") + " - " + revision;
	return ret;
}

} // game_config
