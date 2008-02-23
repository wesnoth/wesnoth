/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
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
#include "network.hpp" // ping_timeout
#include "sound.hpp"
#include "settings.hpp"
#include "util.hpp"
#include "video.hpp" // non_interactive()
#include "wesconfig.h"
#include "wml_exception.hpp"
#include "serialization/parser.hpp"
#include "serialization/string_utils.hpp"

#include <cstdlib>
#include <iostream>
#include <iterator>
#include <sstream>

namespace {

bool message_private_on = true;

bool haloes = true;

std::set<std::string> encountered_units_set;
std::set<t_translation::t_terrain> encountered_terrains_set;

//! Add a nick to the specified relation setting.
void add_relation(const std::string nick, const std::string relation) {
	std::vector<std::string> r = utils::split(preferences::get(relation));
	r.push_back(nick);
	preferences::set(relation, utils::join(r));
}

//! Remove a nick from the specified relation setting.
void remove_relation(const std::string nick, const std::string relation) {
	std::vector<std::string> r = utils::split(preferences::get(relation));
	r.erase(std::remove(r.begin(), r.end(), nick), r.end());
	preferences::set(relation, utils::join(r));
}

} // anon namespace

namespace preferences {

manager::manager()
{
	set_music_volume(music_volume());
	set_sound_volume(sound_volume());

	set_show_haloes(utils::string_bool(preferences::get("show_haloes"), true));
	if(!utils::string_bool(preferences::get("remember_timer_settings"), false)) {
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

	network::ping_timeout = get_ping_timeout();
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
	set_ping_timeout(network::ping_timeout);
}

std::string get_friends() {
	return preferences::get("friends");
}

std::string get_ignores() {
	return preferences::get("ignores");
}

bool add_friend(const std::string nick) {
	if (!utils::isvalid_username(nick)) return false;
	add_relation(nick, "friends");
	return true;
}

bool add_ignore(const std::string nick) {
	if (!utils::isvalid_username(nick)) return false;
	add_relation(nick, "ignores");
	return true;
}

void remove_friend(const std::string nick) {
	remove_relation(nick, "friends");
}

void remove_ignore(const std::string nick) {
	remove_relation(nick, "ignores");
}

void clear_friends() {
	preferences::set("friends", "");	
}

void clear_ignores() {
	preferences::set("ignores", "");	
}

bool is_friend(const std::string nick) {
	const std::vector<std::string>& friends = utils::split(get_friends());
	return (std::find(friends.begin(), friends.end(), nick) != friends.end());
}

bool is_ignored(const std::string nick) {
	const std::vector<std::string>& ignores = utils::split(get_ignores());
	return (std::find(ignores.begin(), ignores.end(), nick) != ignores.end());
}

bool show_lobby_join(const std::string& sender, const std::string& message) {
	// If it's actually not a lobby join message return true (show it).
	if (sender != "server" || message.find("has logged into the lobby") == std::string::npos) return true;
	if (lobby_joins() == SHOW_NONE) return false;
	if (lobby_joins() == SHOW_ALL) return true;
	const std::string::const_iterator i =
			std::find(message.begin(), message.end(), ' ');
	const std::string joiner(message.begin(), i);
	if (lobby_joins() == SHOW_FRIENDS && is_friend(joiner)) return true;
	return false;
}

int lobby_joins()
{
    if(preferences::get("lobby_joins") == "friends") {
		return SHOW_FRIENDS;
	} else if(preferences::get("lobby_joins") == "all") {
		return SHOW_ALL;
	} else if(preferences::get("lobby_joins") == "none") {
		return SHOW_NONE;
	} else {
		return SHOW_FRIENDS;
	}
}


void _set_lobby_joins(int show)
{
	if (show == SHOW_FRIENDS) {
		preferences::set("lobby_joins", "friends");
	} else if (show == SHOW_ALL) {
		preferences::set("lobby_joins", "all");
	} else if (show == SHOW_NONE) {
		preferences::set("lobby_joins", "none");
	}
}

bool sort_list()
{
	return utils::string_bool(preferences::get("sort_list"), true);
}


void _set_sort_list(bool sort)
{
	preferences::set("sort_list", sort ? "yes" : "no");
}

bool iconize_list()
{
	return utils::string_bool(preferences::get("iconize_list"), true);
}

void _set_iconize_list(bool sort)
{
	preferences::set("iconize_list", sort ? "yes" : "no");
}

const std::vector<game_config::server_info>& server_list()
{
	static std::vector<game_config::server_info> pref_servers;
	if(pref_servers.empty()) {
		std::vector<game_config::server_info> &game_servers = game_config::server_list;
		VALIDATE(game_servers.size() > 0, _("No server has been defined."));
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
	if(res.empty()) {
		return server_list().front().address;
	} else {
		return res;
	}
}

void set_network_host(const std::string& host)
{
	preferences::set("host", host);
}

const unsigned int get_ping_timeout()
{
	return lexical_cast_default<unsigned>(preferences::get("ping_timeout"), 0);
}

void set_ping_timeout(unsigned int timeout)
{
	network::ping_timeout = timeout;
	preferences::set("ping_timeout", lexical_cast<std::string>(timeout));
}

const std::string campaign_server()
{
	if(!preferences::get("campaign_server").empty()) {
		return preferences::get("campaign_server");
	} else {
		return "campaigns.wesnoth.org";
	}
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

bool turn_dialog()
{
	return utils::string_bool(preferences::get("turn_dialog"), false);
}

void set_turn_dialog(bool ison)
{
	preferences::set("turn_dialog", ison ? "yes" : "no");
}

bool show_combat()
{
	return utils::string_bool(preferences::get("show_combat"), true);
}

bool allow_observers()
{
	return utils::string_bool(preferences::get("allow_observers"), true);
}

void set_allow_observers(bool value)
{
	preferences::set("allow_observers", value ? "yes" : "no");
}

bool use_map_settings()
{
	return utils::string_bool(preferences::get("mp_use_map_settings"), true);
}

void set_use_map_settings(bool value)
{
	preferences::set("mp_use_map_settings", value ? "yes" : "no");
}

bool random_start_time()
{
	return settings::use_random_start_time(preferences::get("mp_random_start_time"));
}

void set_random_start_time(bool value)
{
	preferences::set("mp_random_start_time", value ? "yes" : "no");
}

bool fog()
{
	return settings::use_fog(preferences::get("mp_fog"));
}

void set_fog(bool value)
{
	preferences::set("mp_fog", value ? "yes" : "no");
}

bool shroud()
{
	return settings::use_shroud(preferences::get("mp_shroud"));
}

void set_shroud(bool value)
{
	preferences::set("mp_shroud", value ? "yes" : "no");
}

int turns()
{
	return settings::get_turns(preferences::get("mp_turns"));
}

void set_turns(int value)
{
	preferences::set("mp_turns", lexical_cast<std::string>(value));
}

bool skip_mp_replay()
{
	return utils::string_bool(preferences::get("skip_mp_replay"), false);
}

void set_skip_mp_replay(bool value)
{
	preferences::set("skip_mp_replay", value ? "yes" : "no");
}

bool countdown()
{
	return utils::string_bool(preferences::get("mp_countdown"), false);
}

void set_countdown(bool value)
{
	preferences::set("mp_countdown", value ? "yes" : "no");
}

int countdown_init_time()
{
	return lexical_cast_in_range<int>
		(preferences::get("mp_countdown_init_time"), 270, 0, 1500);
}

void set_countdown_init_time(int value)
{
	preferences::set("mp_countdown_init_time", lexical_cast<std::string>(value));
}

int countdown_reservoir_time()
{
	return lexical_cast_in_range<int>(
		preferences::get("mp_countdown_reservoir_time"), 330, 30, 1500);
}

void set_countdown_reservoir_time(int value)
{
	preferences::set("mp_countdown_reservoir_time", lexical_cast<std::string>(value));
}

int countdown_turn_bonus()
{
	return lexical_cast_in_range<int>(
		preferences::get("mp_countdown_turn_bonus"), 40, 0, 300);
}

void set_countdown_turn_bonus(int value)
{
	preferences::set("mp_countdown_turn_bonus", lexical_cast<std::string>(value));
}

int countdown_action_bonus()
{
	return lexical_cast_in_range<int>(
		preferences::get("mp_countdown_action_bonus"), 13, 0, 30);
}

void set_countdown_action_bonus(int value)
{
	preferences::set("mp_countdown_action_bonus", lexical_cast<std::string>(value));
}

int village_gold()
{
	return settings::get_village_gold(preferences::get("mp_village_gold"));
}

void set_village_gold(int value)
{
	preferences::set("mp_village_gold", lexical_cast<std::string>(value));
}

int xp_modifier()
{
	return settings::get_xp_modifier(preferences::get("mp_xp_modifier"));
}

void set_xp_modifier(int value)
{
	preferences::set("mp_xp_modifier", lexical_cast<std::string>(value));
}

int era()
{
	return lexical_cast_default<int>(preferences::get("mp_era"), 0);
}

void set_era(int value)
{
	preferences::set("mp_era", lexical_cast<std::string>(value));
}

int map()
{
	return lexical_cast_default<int>(preferences::get("mp_map"), 0);
}

void set_map(int value)
{
	preferences::set("mp_map", lexical_cast<std::string>(value));
}

bool show_ai_moves()
{
	return utils::string_bool(preferences::get("show_ai_moves"), true);
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
	return utils::string_bool(preferences::get("show_side_colours"), true);
}

void set_save_replays(bool value)
{
	preferences::set("save_replays", value ? "yes" : "no");
}

bool save_replays()
{
	return utils::string_bool(preferences::get("save_replays"), true);
}

void set_delete_saves(bool value)
{
	preferences::set("delete_saves", value ? "yes" : "no");
}

bool delete_saves()
{
	return utils::string_bool(preferences::get("delete_saves"), false);
}

void set_ask_delete_saves(bool value)
{
	preferences::set("ask_delete", value ? "yes" : "no");
}

bool ask_delete_saves()
{
	return utils::string_bool(preferences::get("ask_delete"), true);
}

int autosavemax()
{
	return lexical_cast_default<int>(preferences::get("auto_save_max"), 10);
}

void set_autosavemax(int value)
{
	preferences::set("auto_save_max", lexical_cast<std::string>(value));
}

std::string client_type()
{
	return preferences::get("client_type") == "ai" ? "ai" : "human";
}

std::string clock_format()
{
	if(preferences::get("clock_format").size())
		return preferences::get("clock_format");
	else
		preferences::set("clock_format", "%H:%M");
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
	return utils::string_bool(preferences::get("floating_labels"), true);
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
	return utils::string_bool(preferences::get("tip_of_day"), true);
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

bool flip_time()
{
	return utils::string_bool(preferences::get("flip_time"), false);
}

void set_flip_time(bool value)
{
	preferences::set("flip_time", value ? "yes" : "no");
}

bool upload_log()
{
	return utils::string_bool(preferences::get("upload_log"), false);
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
		preferences::set("upload_id",
			lexical_cast<std::string>(rand())
				 + lexical_cast<std::string>(SDL_GetTicks()));
	}
	return preferences::get("upload_id");
}

bool compress_saves()
{
	return utils::string_bool(preferences::get("compress_saves"), true);
}

std::string get_chat_timestamp(const time_t& t) {
	if (chat_timestamping()) {
		return lg::get_timestamp(t, clock_format()) + " ";
	}
	return "";
}

bool chat_timestamping() {
	return utils::string_bool(preferences::get("chat_timestamp"), false);
}

void set_chat_timestamping(bool value) {
	preferences::set("chat_timestamp", value ? "yes" : "no");
}

int chat_lines()
{
	return lexical_cast_default<int>(preferences::get("chat_lines"), 6);
}

void set_chat_lines(int lines)
{
	preferences::set("chat_lines", lexical_cast<std::string>(lines));
}

std::set<std::string> &encountered_units() {
	return encountered_units_set;
}

std::set<t_translation::t_terrain> &encountered_terrains() {
	return encountered_terrains_set;
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
	for (int map_x = 0; map_x < map.w(); map_x++) {
		for (int map_y = 0; map_y < map.h(); map_y++) {
			const t_translation::t_terrain t = map.get_terrain(gamemap::location(map_x, map_y));
			preferences::encountered_terrains().insert(t);
		}
	}
}

} // preferences namespace
