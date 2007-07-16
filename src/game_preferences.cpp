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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "config.hpp"
#include "filesystem.hpp"
#include "game_preferences.hpp"
#include "gamestatus.hpp"
#include "gettext.hpp"
#include "hotkeys.hpp"
#include "log.hpp"
#include "sound.hpp"
#include "util.hpp"
#include "video.hpp" // non_interactive()
#include "wassert.hpp"
#include "wesconfig.h"
#include "serialization/parser.hpp"
#include "serialization/string_utils.hpp"

#include <cstdlib>
#include <iostream>
#include <iterator>
#include <sstream>

namespace {

bool message_private_on = true;

bool haloes = true;

bool unit_genders = true;

std::set<std::string> encountered_units_set;
std::set<t_translation::t_letter> encountered_terrains_set;

}

namespace preferences {

manager::manager()
{
	set_music_volume(music_volume());
	set_sound_volume(sound_volume());

	set_show_haloes(preferences::get("show_haloes") != "no");
	if(preferences::get("remember_timer_settings") != "yes") {
		preferences::erase("mp_countdown_init_time");
		preferences::erase("mp_countdown_reservoir_time");
		preferences::erase("mp_countdown_turn_bonus");
		preferences::erase("mp_countdown_action_bonus");
	}

	const std::vector<std::string> v = utils::split(preferences::get("encountered_units"));
	std::copy(v.begin(), v.end(),
			  std::inserter(encountered_units_set, encountered_units_set.begin()));

	const t_translation::t_list terrain =
		t_translation::read_list(preferences::get("encountered_terrain_list"));
	std::copy(terrain.begin(), terrain.end(), 
			  std::inserter(encountered_terrains_set, encountered_terrains_set.begin()));
}

manager::~manager()
{
	std::vector<std::string> v;
	std::copy(encountered_units_set.begin(), encountered_units_set.end(), std::back_inserter(v));
	preferences::set("encountered_units", utils::join(v));
	t_translation::t_list terrain;
	std::copy(encountered_terrains_set.begin(), encountered_terrains_set.end(),
			  std::back_inserter(terrain));
	preferences::set("encountered_terrain_list", t_translation::write_list(terrain));

	encountered_units_set.clear();
	encountered_terrains_set.clear();
}

bool _set_relationship(std::string nick, std::string rela) {
	if (!get_prefs()->child("relationship")){
		get_prefs()->add_child("relationship");
	}
	config* cignore;
	cignore = get_prefs()->child("relationship");
	if(utils::isvalid_username(nick))
	{
		(*cignore)[nick] = rela;
		return true;
	} else {
		return false;
	}
}

int lobby_joins()
{
    if (preferences::get("lobby_joins") == "friends") {return SHOW_FRIENDS;}
    else if (preferences::get("lobby_joins") == "all") {return SHOW_ALL;}
    else {return SHOW_NON;}
}


void _set_lobby_joins(int show)
{
    if (show == SHOW_FRIENDS) {preferences::set("lobby_joins", "friends");}
    else if (show == SHOW_ALL) {preferences::set("lobby_joins", "all");}
    else {preferences::set("lobby_joins", "non");}
}

bool sort_list()
{
	return preferences::get("sort_list") != "no";
}


void _set_sort_list(bool sort)
{
	if(sort)
		preferences::set("sort_list", "yes");
	else
		preferences::set("sort_list", "no");
}

bool iconize_list()
{
	return preferences::get("iconize_list") != "no";
}


void _set_iconize_list(bool sort)
{
	if(sort)
		preferences::set("iconize_list", "yes");
	else
		preferences::set("iconize_list", "no");
}

const std::vector<game_config::server_info>& server_list()
{
	static std::vector<game_config::server_info> pref_servers;
	if(pref_servers.empty()) {
		std::vector<game_config::server_info> &game_servers = game_config::server_list;
		wassert(game_servers.size() > 0);
		pref_servers.insert(pref_servers.begin(), game_servers.begin(), game_servers.end());
		const std::vector<config *> &user_servers = get_prefs()->get_children("server");
		std::vector<config *>::const_iterator server;
		for(server = user_servers.begin(); server != user_servers.end(); ++server) {
			game_config::server_info sinf;
			sinf.name = (**server)["name"];
			sinf.address = (**server)["address"];
			pref_servers.push_back(sinf);
		}
	}
	return pref_servers;
}

const std::string network_host()
{
	const std::string res = preferences::get("host");
	if(res.empty())
		return server_list().front().address;
	else
		return res;
}

void set_network_host(const std::string& host)
{
	preferences::set("host", host);
}

const std::string campaign_server()
{
	if(preferences::get("campaign_server").empty())
		return preferences::get("campaign_server");
	else
		return "campaigns.wesnoth.org";
}

void set_campaign_server(const std::string& host)
{
	preferences::set("campaign_server", host);
}

const std::string login()
{
	const std::string res = preferences::get("login");
	if(res.empty()) {
		char* const login = getenv("USER");
		if(login != NULL) {
			return login;
		}

		if(res.empty()) {
			return _("player");
		}
	}

	return res;
}

void set_login(const std::string& username)
{
	preferences::set("login", username);
}

namespace {
	double scroll = 0.2;
}

bool turn_dialog()
{
	return preferences::get("turn_dialog") == "yes";
}

void set_turn_dialog(bool ison)
{
	preferences::set("turn_dialog", (ison ? "yes" : "no"));
}

bool show_combat()
{
	return preferences::get("show_combat") != "no";
}

bool allow_observers()
{
	return preferences::get("allow_observers") != "no";
}

void set_allow_observers(bool value)
{
	preferences::set("allow_observers", value ? "yes" : "no");
}

bool use_map_settings()
{
	return preferences::get("mp_use_map_settings") != "no";
}

void set_use_map_settings(bool value)
{
	preferences::set("mp_use_map_settings", value ? "yes" : "no");
}

bool random_start_time()
{
	return preferences::get("mp_random_start_time") != "no";
}

void set_random_start_time(bool value)
{
	preferences::set("mp_random_Start_time", value ? "yes" : "no");
}


bool fog()
{
	return preferences::get("mp_fog") == "yes";
}

void set_fog(bool value)
{
	preferences::set("mp_fog", value ? "yes" : "no");
}

bool shroud()
{
	return preferences::get("mp_shroud") == "yes";
}

void set_shroud(bool value)
{
	preferences::set("mp_shroud", value ? "yes" : "no");
}

int turns()
{
	static const int default_value = 50;
	int value = 0;
	const string_map::const_iterator i = get_prefs()->values.find("mp_turns");
	if(i != get_prefs()->values.end() && i->second.empty() == false) {
		value = atoi(i->second.c_str());
	}

	if(value < 20 || value > 100) {
		value = default_value;
	}

	return value;
}

void set_turns(int value)
{
	std::stringstream stream;
	stream << value;
	preferences::set("mp_turns", stream.str());
}

bool skip_mp_replay()
{
	return preferences::get("skip_mp_replay") == "yes";
}

void set_skip_mp_replay(bool value)
{
	preferences::set("skip_mp_replay", value ? "yes" : "no");
}


bool countdown()
{
	return preferences::get("mp_countdown") == "yes";
}

void set_countdown(bool value)
{
	preferences::set("mp_countdown", value ? "yes" : "no");
}


int countdown_init_time()
{
	static const int default_value = 270;
	int value = 0;
	const std::string& timer_init = preferences::get("mp_countdown_init_time");
	value = lexical_cast_default<int>(timer_init,default_value);

	if(value < 0 || value > 1500) {
		value = default_value;
	}

	return value;
}

void set_countdown_init_time(int value)
{
	preferences::set("mp_countdown_init_time", lexical_cast<std::string>(value));
}

int countdown_reservoir_time()
{
	static const int default_value = 330;
	int value = 0;
	const std::string& timer_init = preferences::get("mp_countdown_reservoir_time");
	value = lexical_cast_default<int>(timer_init,default_value);

	if(value < 30 || value > 1500) {
		value = default_value;
	}

	return value;
}

void set_countdown_reservoir_time(int value)
{
	preferences::set("mp_countdown_reservoir_time", lexical_cast<std::string>(value));
}

int countdown_turn_bonus()
{
	static const int default_value = 40;
	int value = 0;
	const std::string& timer_bonus = preferences::get("mp_countdown_turn_bonus");
	value = lexical_cast_default<int>(timer_bonus,default_value);

	if(value <= 0 || value > 300) {
		value = default_value;
	}

	return value;
}

void set_countdown_turn_bonus(int value)
{
	preferences::get("mp_countdown_turn_bonus") =lexical_cast<std::string>(value);
}

int countdown_action_bonus()
{
	static const int default_value = 13;
	int value = 0;
	const std::string& timer_bonus = preferences::get("mp_countdown_action_bonus");
	value = lexical_cast_default<int>(timer_bonus,default_value);

	if(value < 0 || value > 30) {
		value = default_value;
	}

	return value;
}

void set_countdown_action_bonus(int value)
{
	preferences::get("mp_countdown_action_bonus") =lexical_cast<std::string>(value);
}

int village_gold()
{
	static const int default_value = 2;
	int value = 0;
	const string_map::const_iterator i = get_prefs()->values.find("mp_village_gold");
	if(i != get_prefs()->values.end() && i->second.empty() == false) {
		value = atoi(i->second.c_str());
	}

	if(value < 1 || value > 5) {
		value = default_value;
	}

	return value;
}

void set_village_gold(int value)
{
	std::stringstream stream;
	stream << value;
	preferences::set("mp_village_gold", stream.str());
}

int xp_modifier()
{
	static const int default_value = 70;
	int value = 0;
	const string_map::const_iterator i = get_prefs()->values.find("mp_xp_modifier");
	if(i != get_prefs()->values.end() && i->second.empty() == false) {
		value = atoi(i->second.c_str());
	}

	if(value < 30 || value > 200) {
		value = default_value;
	}

	return value;
}

void set_xp_modifier(int value)
{
	std::stringstream stream;
	stream << value;
	preferences::set("mp_xp_modifier", stream.str());
}

int era()
{
	int value = 0;
	const string_map::const_iterator i = get_prefs()->values.find("mp_era");
	if(i != get_prefs()->values.end() && i->second.empty() == false) {
		value = atoi(i->second.c_str());
	}

	return value;
}

void set_era(int value)
{
	std::stringstream stream;
	stream << value;
	preferences::set("mp_era", stream.str());
}

int map()
{
	int value = 0;
	const string_map::const_iterator i = get_prefs()->values.find("mp_map");
	if(i != get_prefs()->values.end() && i->second.empty() == false) {
		value = atoi(i->second.c_str());
	}
	return value;
}

void set_map(int value)
{
	std::stringstream stream;
	stream << value;
	preferences::set("mp_map", stream.str());
}

bool show_ai_moves()
{
	return preferences::get("show_ai_moves") != "no";
}

void set_show_ai_moves(bool value)
{
	preferences::set("show_ai_moves", value ? "yes" : "no");
}

void set_show_side_colours(bool value)
{
	preferences::set("show_side_colours", value ? "yes" : "no");
}

bool show_side_colours()
{
	return preferences::get("show_side_colours") != "no";
}

void set_save_replays(bool value)
{
	preferences::set("save_replays", value ? "yes" : "no");
}

bool save_replays()
{
	return preferences::get("save_replays") != "no";
}

void set_delete_autosaves(bool value)
{
	preferences::set("delete_autosaves", value ? "yes" : "no");
}

bool delete_autosaves()
{
	return preferences::get("delete_autosaves") == "yes";
}

void set_ask_delete_saves(bool value)
{
	preferences::set("ask_delete", value ? "yes" : "no");
}

bool ask_delete_saves()
{
	return preferences::get("ask_delete") != "no";
}

std::string client_type()
{
	if(preferences::get("client_type") == "ai")
		return "ai";
	else
		return "human";
}

std::string clock_format()
{
	if(preferences::get("clock_format").size())
		return preferences::get("clock_format");
	else
		preferences::get("clock_format")="%H:%M";
	return "%H:%M";
}

const std::string theme()
{
	if(non_interactive()) {
		static const std::string null_theme = "null";
		return null_theme;
	}

	std::string res = preferences::get("theme");
	if(res.empty()) {
		return "Default";
	}

	return res;
}

void set_theme(const std::string& theme)
{
	if(theme != "null") {
		preferences::set("theme", theme);
	}
}

bool show_floating_labels()
{
	return preferences::get("floating_labels") != "no";
}

void set_show_floating_labels(bool value)
{
	preferences::set("floating_labels", value ? "yes" : "no");
}

bool message_private()
{
	return message_private_on;
}

void set_message_private(bool value)
{
	message_private_on = value;
}

bool show_tip_of_day()
{
	return preferences::get("tip_of_day") != "no";
}

bool show_haloes()
{
	return haloes;
}

void set_show_haloes(bool value)
{
	haloes = value;
	preferences::set("show_haloes", value ? "yes" : "no");
}

bool upload_log()
{
	return preferences::get("upload_log") == "yes";
}

void set_upload_log(bool value)
{
	preferences::set("upload_log", value ? "yes" : "no");
}

const std::string upload_id()
{
	// We create a unique id for each person, *when asked for* to increase
	// randomness.
	if (preferences::get("upload_id") == "") {
		srand(time(NULL));
		preferences::get("upload_id")
			= lexical_cast<std::string>(rand())
			+ lexical_cast<std::string>(SDL_GetTicks());
	}
	return preferences::get("upload_id");
}

bool compress_saves()
{
	return preferences::get("compress_saves") != "no";
}

bool chat_timestamp()
{
	return preferences::get("chat_timestamp") == "yes";
}

void set_chat_timestamp(bool value)
{
	preferences::set("chat_timestamp", value ? "yes" : "no");
}

int chat_lines()
{
	// defaults to 6 chat log lines displayed
	static const int default_value = 6;
	const string_map::const_iterator lines = get_prefs()->values.find("chat_lines");
	if(lines != get_prefs()->values.end() && lines->second.empty() == false)
		return atoi(lines->second.c_str());
	else
		return default_value;
}

void set_chat_lines(int lines)
{
	std::stringstream stream;
	stream << lines;
	preferences::set("chat_lines", stream.str());
}

std::set<std::string> &encountered_units() {
	return encountered_units_set;
}

std::set<t_translation::t_letter> &encountered_terrains() {
	return encountered_terrains_set;
}

bool compare_resolutions(const std::pair<int,int>& lhs, const std::pair<int,int>& rhs)
{
	return lhs.first*lhs.second < rhs.first*rhs.second;
}

bool green_confirm()
{
	std::string confirmation = preferences::get("confirm_end_turn");

	if (confirmation == "green" || confirmation == "yes")
		return true;
	return false;
}

bool yellow_confirm()
{
	return preferences::get("confirm_end_turn") == "yellow";
}

bool confirm_no_moves()
{
	//This is very non-intrusive so it is on by default
	const std::string confirmation = preferences::get("confirm_end_turn");
	return confirmation == "no_moves" || confirmation.empty();
}


void encounter_recruitable_units(std::vector<team>& teams){
	for (std::vector<team>::iterator help_team_it = teams.begin();
		help_team_it != teams.end(); help_team_it++) {
		help_team_it->log_recruitable();
		std::copy(help_team_it->recruits().begin(), help_team_it->recruits().end(),
				  std::inserter(encountered_units_set, encountered_units_set.begin()));
	}
}

void encounter_start_units(unit_map& units){
	for (unit_map::const_iterator help_unit_it = units.begin();
		 help_unit_it != units.end(); help_unit_it++) {
		const std::string name = help_unit_it->second.id();
		encountered_units_set.insert(name);
	}
}

void encounter_recallable_units(game_state& gamestate){
	for(std::map<std::string, player_info>::iterator pi = gamestate.players.begin(); pi!=gamestate.players.end(); ++pi) {
		for(std::vector<unit>::iterator help_recall_it = pi->second.available_units.begin(); help_recall_it != pi->second.available_units.end(); help_recall_it++) {
			encountered_units_set.insert(help_recall_it->id());
		}
	}
}

void encounter_map_terrain(gamemap& map){
	for (int map_x = 0; map_x < map.x(); map_x++) {
		for (int map_y = 0; map_y < map.y(); map_y++) {
			const t_translation::t_letter t = map.get_terrain(gamemap::location(map_x, map_y));
			preferences::encountered_terrains().insert(t);
		}
	}
}

}
