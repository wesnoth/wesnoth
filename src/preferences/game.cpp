/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "game_board.hpp"
#include "game_display.hpp"
#include "preferences/game.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode_cast.hpp"
#include "settings.hpp"
#include "units/unit.hpp"
#include "units/map.hpp"
#include "wml_exception.hpp"

#include <cassert>
#ifdef _WIN32
#include <boost/range/iterator_range.hpp>
#ifdef INADDR_ANY
	#undef INADDR_ANY
#endif
#ifdef INADDR_BROADCAST
	#undef INADDR_BROADCAST
#endif
#ifdef INADDR_NONE
	#undef INADDR_NONE
#endif
#include <windows.h> //GetUserName
#endif


static lg::log_domain log_config("config");
#define ERR_CFG LOG_STREAM(err , log_config)

using acquaintances_map = std::map<std::string, preferences::acquaintance>;

namespace {

bool message_private_on = false;

bool haloes = true;

std::map<std::string, std::set<std::string> > completed_campaigns;
std::set<std::string> encountered_units_set;
std::set<t_translation::terrain_code> encountered_terrains_set;

std::map<std::string, std::vector<std::string> > history_map;

acquaintances_map acquaintances;

std::vector<std::string> mp_modifications;
bool mp_modifications_initialized = false;
std::vector<std::string> sp_modifications;
bool sp_modifications_initialized = false;

config option_values;
bool options_initialized = false;

bool authenticated = false;

const char WRAP_CHAR = '@';
const std::string EMPTY_WRAPPED_STRING = "@@";

std::string wrap_credentials_field_value(const std::string& value)
{
	return WRAP_CHAR + value + WRAP_CHAR;
}

std::string parse_wrapped_credentials_field(const std::string& raw)
{
	if(raw.empty() || raw == EMPTY_WRAPPED_STRING) {
		// empty (wrapped or not)
		return raw;
	} else if(raw.length() < 2 || raw[0] != WRAP_CHAR || raw[raw.length() - 1] != WRAP_CHAR ) {
		// malformed/not wrapped (shouldn't happen)
		ERR_CFG << "malformed user credentials (did you manually edit the preferences file?)" << std::endl;
		return raw;
	}

	return raw.substr(1, raw.length() - 2);
}

void initialize_modifications(bool mp = true)
{
	if (mp) {
		mp_modifications = utils::split(preferences::get("mp_modifications"), ',');
		mp_modifications_initialized = true;
	} else {
		sp_modifications = utils::split(preferences::get("sp_modifications"), ',');
		sp_modifications_initialized = true;
	}
}

} // anon namespace

namespace preferences {


manager::manager() :
	base()
{
	set_music_volume(music_volume());
	set_sound_volume(sound_volume());

	set_show_haloes(preferences::get("show_haloes", true));
	if (!preferences::get("remember_timer_settings", false)) {
		preferences::erase("mp_countdown_init_time");
		preferences::erase("mp_countdown_reservoir_time");
		preferences::erase("mp_countdown_turn_bonus");
		preferences::erase("mp_countdown_action_bonus");
	}

	/*
	completed_campaigns = "A,B,C"
	[completed_campaigns]
		[campaign]
			name = "A"
			difficulty_levels = "EASY,MEDIUM"
		[/campaign]
	[/completed_campaigns]
	*/
	for (const std::string &c : utils::split(preferences::get("completed_campaigns"))) {
		completed_campaigns[c]; // create the elements
	}
	if (const config &ccc = preferences::get_child("completed_campaigns")) {
		for (const config &cc : ccc.child_range("campaign")) {
			std::set<std::string> &d = completed_campaigns[cc["name"]];
			std::vector<std::string> nd = utils::split(cc["difficulty_levels"]);
			std::copy(nd.begin(), nd.end(), std::inserter(d, d.begin()));
		}
	}

	const std::vector<std::string> v (utils::split(preferences::get("encountered_units")));
	encountered_units_set.insert(v.begin(), v.end());

	const t_translation::ter_list terrain (t_translation::read_list(preferences::get("encountered_terrain_list")));
	encountered_terrains_set.insert(terrain.begin(), terrain.end());

	if (const config &history = preferences::get_child("history"))
	{
/* Structure of the history
	[history]
		[history_id]
			[line]
				message = foobar
			[/line]
*/
		for (const config::any_child &h : history.all_children_range())
		{
			for (const config &l : h.cfg.child_range("line")) {
				history_map[h.key].push_back(l["message"]);
			}
		}
	}

	//network::ping_timeout = get_ping_timeout();
}

manager::~manager()
{
	config campaigns;
	typedef const std::pair<std::string, std::set<std::string> > cc_elem;
	for (cc_elem &elem : completed_campaigns) {
		config cmp;
		cmp["name"] = elem.first;
		cmp["difficulty_levels"] = utils::join(elem.second);
		campaigns.add_child("campaign", cmp);
	}
	preferences::set_child("completed_campaigns", campaigns);
	std::vector<std::string> v (encountered_units_set.begin(), encountered_units_set.end());
	preferences::set("encountered_units", utils::join(v));
	t_translation::ter_list terrain (encountered_terrains_set.begin(), encountered_terrains_set.end());
	preferences::set("encountered_terrain_list", t_translation::write_list(terrain));

/* Structure of the history
	[history]
		[history_id]
			[line]
				message = foobar
			[/line]
*/
	config history;
	typedef std::pair<std::string, std::vector<std::string> > hack;
	for (const hack& history_id : history_map) {

		config history_id_cfg; // [history_id]
		for (const std::string& line : history_id.second) {
			config cfg; // [line]

			cfg["message"] = line;
			history_id_cfg.add_child("line", cfg);
		}

		history.add_child(history_id.first, history_id_cfg);
	}
	preferences::set_child("history", history);

	history_map.clear();
	encountered_units_set.clear();
	encountered_terrains_set.clear();
}

bool is_authenticated() {
	return authenticated;
}

void parse_admin_authentication(const std::string& sender, const std::string& message) {
	if(sender != "server") return;
	if(message.compare(0, 43, "You are now recognized as an administrator.") == 0) {
		authenticated = true;
	} else if(message.compare(0, 50, "You are no longer recognized as an administrator.") == 0) {
		authenticated = false;
	}
}

admin_authentication_reset::admin_authentication_reset()
{
}

admin_authentication_reset::~admin_authentication_reset()
{
	authenticated = false;
}

static void load_acquaintances() {
	if(acquaintances.empty()) {
		for (const config &acfg : preferences::get_prefs()->child_range("acquaintance")) {
			acquaintance ac = acquaintance(acfg);
			acquaintances[ac.get_nick()] = ac;
		}
	}
}

static void save_acquaintances()
{
	config *cfg = preferences::get_prefs();
	cfg->clear_children("acquaintance");

	for(std::map<std::string, acquaintance>::iterator i = acquaintances.begin();
			i != acquaintances.end(); ++i)
	{
		config& item = cfg->add_child("acquaintance");
		i->second.save(item);
	}
}

const std::map<std::string, acquaintance> & get_acquaintances() {
	load_acquaintances();
	return acquaintances;
}

//returns acquaintances in the form nick => notes where the status = filter
std::map<std::string, std::string> get_acquaintances_nice(const std::string& filter) {
	load_acquaintances();
	std::map<std::string, std::string> ac_nice;

	for(std::map<std::string, acquaintance>::iterator i = acquaintances.begin(); i != acquaintances.end(); ++i)
	{
		if(i->second.get_status() == filter) {
			ac_nice[i->second.get_nick()] = i->second.get_notes();
		}
	}

	return ac_nice;
}

preferences::acquaintance* add_acquaintance(const std::string& nick, const std::string& mode, const std::string& notes)
{
	if(!utils::isvalid_wildcard(nick)) {
		return nullptr;
	}

	preferences::acquaintance new_entry(nick, mode, notes);

	acquaintances_map::iterator iter;
	bool success;

	std::tie(iter, success) = acquaintances.emplace(nick, new_entry);

	if(!success) {
		iter->second = new_entry;
	}

	save_acquaintances();

	return &iter->second;
}

bool remove_acquaintance(const std::string& nick) {
	std::map<std::string, acquaintance>::iterator i = acquaintances.find(nick);

	//nick might include the notes, depending on how we're removing
	if(i == acquaintances.end()) {
		size_t pos = nick.find_first_of(' ');

		if(pos != std::string::npos) {
			i = acquaintances.find(nick.substr(0, pos));
		}
	}

	if(i == acquaintances.end()) {
		return false;
	}

	acquaintances.erase(i);
	save_acquaintances();

	return true;
}

bool is_friend(const std::string& nick)
{
	load_acquaintances();
	const std::map<std::string, acquaintance
			>::const_iterator it = acquaintances.find(nick);

	if(it == acquaintances.end()) {
		return false;
	} else {
		return it->second.get_status() == "friend";
	}
}

bool is_ignored(const std::string& nick)
{
	load_acquaintances();
	const std::map<std::string, acquaintance
			>::const_iterator it = acquaintances.find(nick);

	if(it == acquaintances.end()) {
		return false;
	} else {
		return it->second.get_status() == "ignore";
	}
}

void add_completed_campaign(const std::string &campaign_id, const std::string &difficulty_level) {
	completed_campaigns[campaign_id].insert(difficulty_level);
}

bool is_campaign_completed(const std::string& campaign_id) {
	return completed_campaigns.count(campaign_id) != 0;
}

bool is_campaign_completed(const std::string& campaign_id, const std::string &difficulty_level) {
	std::map<std::string, std::set<std::string> >::iterator it = completed_campaigns.find(campaign_id);
	return it == completed_campaigns.end() ? false : it->second.count(difficulty_level) != 0;
}

bool parse_should_show_lobby_join(const std::string &sender, const std::string &message)
{
	// If it's actually not a lobby join or leave message return true (show it).
	if (sender != "server") return true;
	std::string::size_type pos = message.find(" has logged into the lobby");
	if (pos == std::string::npos){
		pos = message.find(" has disconnected");
		if (pos == std::string::npos) return true;
	}
	int lj = lobby_joins();
	if (lj == SHOW_NONE) return false;
	if (lj == SHOW_ALL) return true;
	return is_friend(message.substr(0, pos));
}

int lobby_joins()
{
	std::string pref = preferences::get("lobby_joins");
	if (pref == "friends") {
		return SHOW_FRIENDS;
	} else if (pref == "all") {
		return SHOW_ALL;
	} else if (pref == "none") {
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

const std::vector<game_config::server_info>& server_list()
{
	static std::vector<game_config::server_info> pref_servers;
	if(pref_servers.empty()) {
		std::vector<game_config::server_info> &game_servers = game_config::server_list;
		VALIDATE(!game_servers.empty(), _("No server has been defined."));
		pref_servers.insert(pref_servers.begin(), game_servers.begin(), game_servers.end());
		for(const config &server : get_prefs()->child_range("server")) {
			game_config::server_info sinf;
			sinf.name = server["name"].str();
			sinf.address = server["address"].str();
			pref_servers.push_back(sinf);
		}
	}
	return pref_servers;
}

std::string network_host()
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

unsigned int get_ping_timeout()
{
	return lexical_cast_default<unsigned>(preferences::get("ping_timeout"), 0);
}

std::string campaign_server()
{
	if(!preferences::get("campaign_server").empty()) {
		return preferences::get("campaign_server");
	} else {
		return "add-ons.wesnoth.org";
	}
}

void set_campaign_server(const std::string& host)
{
	preferences::set("campaign_server", host);
}

bool wrap_password()
{
	const bool have_old_password_format =
		(!preferences::have_setting("password_is_wrapped")) && preferences::have_setting("password");
	return have_old_password_format ? false : preferences::get("password_is_wrapped", true);
}

void set_wrap_password(bool wrap)
{
	preferences::set("password_is_wrapped", wrap);
}

bool wrap_login()
{
	const bool have_old_login_format =
		(!preferences::have_setting("login_is_wrapped")) && preferences::have_setting("login");
	return have_old_login_format ? false : preferences::get("login_is_wrapped", true);
}

void set_wrap_login(bool wrap)
{
	preferences::set("login_is_wrapped", wrap);
}

static std::string get_system_username()
{
	std::string res;
#ifdef _WIN32
	wchar_t buffer[300];
	DWORD size = 300;
	if(GetUserNameW(buffer,&size)) {
		//size includes a terminating null character.
		assert(size > 0);
		res = unicode_cast<utf8::string>(boost::iterator_range<wchar_t*>(buffer, buffer + size - 1));
	}
#else
	if(char* const login = getenv("USER")) {
		res = login;
	}
#endif
	return res;
}

std::string login()
{
	const std::string res = preferences::get("login");
	if(res.empty() || res == EMPTY_WRAPPED_STRING) {
		const std::string& login = get_system_username();
		if(!login.empty()) {
			return login;
		}

		if(res.empty()) {
			return _("player");
		}
	}

	if(!wrap_login()) {
		return res;
	} else {
		return parse_wrapped_credentials_field(res);
	}
}

void set_login(const std::string& username)
{
	set_wrap_login(true);
	preferences::set("login", wrap_credentials_field_value(username));
}

namespace prv {
	std::string password;
}

std::string password()
{
	if(remember_password()) {
		const std::string saved_pass = preferences::get("password");
		if(!wrap_password()) {
			return saved_pass;
		} else {
			return parse_wrapped_credentials_field(saved_pass);
		}
	} else {
		return prv::password;
	}
}

void set_password(const std::string& password)
{
	prv::password = password;
	if(remember_password()) {
		set_wrap_password(true);
		preferences::set("password", wrap_credentials_field_value(password));
	}
}

bool remember_password()
{
	return preferences::get("remember_password", false);
}

void set_remember_password(bool remember)
{
	preferences::set("remember_password", remember);

	if(!remember) {
		preferences::set("password", "");
	}
}

bool turn_dialog()
{
	return preferences::get("turn_dialog", false);
}

void set_turn_dialog(bool ison)
{
	preferences::set("turn_dialog", ison);
}

bool enable_whiteboard_mode_on_start()
{
	return preferences::get("enable_planning_mode_on_start", false);
}

void set_enable_whiteboard_mode_on_start(bool value)
{
	preferences::set("enable_planning_mode_on_start", value);
}

bool hide_whiteboard()
{
	return preferences::get("hide_whiteboard", false);
}

void set_hide_whiteboard(bool value)
{
	preferences::set("hide_whiteboard", value);
}

bool show_combat()
{
	return preferences::get("show_combat", true);
}

bool allow_observers()
{
	return preferences::get("allow_observers", true);
}

void set_allow_observers(bool value)
{
	preferences::set("allow_observers", value);
}

bool registered_users_only()
{
	return preferences::get("registered_users_only", false);
}

void set_registered_users_only(bool value)
{
	preferences::set("registered_users_only", value);
}

bool shuffle_sides()
{
	return preferences::get("shuffle_sides", false);
}

void set_shuffle_sides(bool value)
{
	preferences::set("shuffle_sides", value);
}

std::string random_faction_mode(){
	return preferences::get("random_faction_mode");
}

void set_random_faction_mode(const std::string & value) {
	preferences::set("random_faction_mode", value);
}

bool use_map_settings()
{
	return preferences::get("mp_use_map_settings", true);
}

void set_use_map_settings(bool value)
{
	preferences::set("mp_use_map_settings", value);
}

int mp_server_warning_disabled()
{
	return lexical_cast_default<int>(preferences::get("mp_server_warning_disabled"), 0);
}

void set_mp_server_warning_disabled(int value)
{
	preferences::set("mp_server_warning_disabled", value);
}

void set_mp_server_program_name(const std::string& path)
{
	if (path.empty())
	{
		preferences::clear("mp_server_program_name");
	}
	else
	{
		preferences::set("mp_server_program_name", path);
	}
}

std::string get_mp_server_program_name()
{
	return preferences::get("mp_server_program_name");
}

bool random_start_time()
{
	return preferences::get("mp_random_start_time", true);
}

void set_random_start_time(bool value)
{
	preferences::set("mp_random_start_time", value);
}

bool fog()
{
	return preferences::get("mp_fog", true);
}

void set_fog(bool value)
{
	preferences::set("mp_fog", value);
}

bool shroud()
{
	return preferences::get("mp_shroud", false);
}

void set_shroud(bool value)
{
	preferences::set("mp_shroud", value);
}

int turns()
{
	return settings::get_turns(preferences::get("mp_turns"));
}

void set_turns(int value)
{
	preferences::set("mp_turns", value);
}

const config& options()
{
	if (options_initialized) {
		return option_values;
	}

	if (!preferences::get_child("options")) {
		// It may be an invalid config, which would cause problems in
		// multiplayer_create, so let's replace it with an empty but valid
		// config
		option_values = config();
	} else {
		option_values = preferences::get_child("options");
	}

	options_initialized = true;

	return option_values;
}

void set_options(const config& values)
{
	preferences::set_child("options", values);
	options_initialized = false;
}

bool skip_mp_replay()
{
	return preferences::get("skip_mp_replay", false);
}

void set_skip_mp_replay(bool value)
{
	preferences::set("skip_mp_replay", value);
}

bool blindfold_replay()
{
	return preferences::get("blindfold_replay", false);
}

void set_blindfold_replay(bool value)
{
	preferences::set("blindfold_replay", value);
}

bool countdown()
{
	return preferences::get("mp_countdown", false);
}

void set_countdown(bool value)
{
	preferences::set("mp_countdown", value);
}

int countdown_init_time()
{
	return utils::clamp<int>(
		lexical_cast_default<int>(preferences::get("mp_countdown_init_time"), 270), 0, 1500);
}

void set_countdown_init_time(int value)
{
	preferences::set("mp_countdown_init_time", value);
}

int countdown_reservoir_time()
{
	return utils::clamp<int>(
		lexical_cast_default<int>(preferences::get("mp_countdown_reservoir_time"), 330), 30, 1500);
}

void set_countdown_reservoir_time(int value)
{
	preferences::set("mp_countdown_reservoir_time", value);
}

int countdown_turn_bonus()
{
	return utils::clamp<int>(
		lexical_cast_default<int>(preferences::get("mp_countdown_turn_bonus"), 60), 0, 300);
}

void set_countdown_turn_bonus(int value)
{
	preferences::set("mp_countdown_turn_bonus", value);
}

int countdown_action_bonus()
{
	return utils::clamp<int>(
		lexical_cast_default<int>(preferences::get("mp_countdown_action_bonus"), 13), 0, 30);
}

void set_countdown_action_bonus(int value)
{
	preferences::set("mp_countdown_action_bonus", value);
}

int village_gold()
{
	return settings::get_village_gold(preferences::get("mp_village_gold"));
}

void set_village_gold(int value)
{
	preferences::set("mp_village_gold", value);
}

int village_support()
{
	return settings::get_village_support(preferences::get("mp_village_support"));
}

void set_village_support(int value)
{
	preferences::set("mp_village_support", std::to_string(value));
}

int xp_modifier()
{
	return settings::get_xp_modifier(preferences::get("mp_xp_modifier"));
}

void set_xp_modifier(int value)
{
	preferences::set("mp_xp_modifier", value);
}

std::string era()
{
	return preferences::get("mp_era");
}

void set_era(const std::string& value)
{
	preferences::set("mp_era", value);
}

std::string level()
{
	return preferences::get("mp_level");
}

void set_level(const std::string& value)
{
	preferences::set("mp_level", value);
}

int level_type()
{
	return lexical_cast_default<int>(preferences::get("mp_level_type"), 0);
}

void set_level_type(int value)
{
	preferences::set("mp_level_type", value);
}

const std::vector<std::string>& modifications(bool mp)
{
	if ((!mp_modifications_initialized && mp) || (!sp_modifications_initialized && !mp))
		initialize_modifications(mp);

	return mp ? mp_modifications : sp_modifications;
}

void set_modifications(const std::vector<std::string>& value, bool mp)
{
	if (mp) {
		preferences::set("mp_modifications", utils::join(value, ","));
		mp_modifications_initialized = false;
	} else {
		preferences::set("sp_modifications", utils::join(value, ","));
		sp_modifications_initialized = false;
	}

}

bool skip_ai_moves()
{
	return preferences::get("skip_ai_moves", false);
}

void set_skip_ai_moves(bool value)
{
	preferences::set("skip_ai_moves", value);
}

void set_show_side_colors(bool value)
{
	preferences::set("show_side_colors", value);
}

bool show_side_colors()
{
	return preferences::get("show_side_colors", true);
}

void set_save_replays(bool value)
{
	preferences::set("save_replays", value);
}

bool save_replays()
{
	return preferences::get("save_replays", true);
}

void set_delete_saves(bool value)
{
	preferences::set("delete_saves", value);
}

bool delete_saves()
{
	return preferences::get("delete_saves", false);
}

void set_ask_delete_saves(bool value)
{
	preferences::set("ask_delete", value);
}

bool ask_delete_saves()
{
	return preferences::get("ask_delete", true);
}

void set_interrupt_when_ally_sighted(bool value)
{
	preferences::set("ally_sighted_interrupts", value);
}

bool interrupt_when_ally_sighted()
{
	return preferences::get("ally_sighted_interrupts", true);
}

int autosavemax()
{
	return lexical_cast_default<int>(preferences::get("auto_save_max"), 10);
}

void set_autosavemax(int value)
{
	preferences::set("auto_save_max", value);
}

std::string theme()
{
	if(CVideo::get_singleton().non_interactive()) {
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
	return preferences::get("floating_labels", true);
}

void set_show_floating_labels(bool value)
{
	preferences::set("floating_labels", value);
}

bool message_private()
{
	return message_private_on;
}

void set_message_private(bool value)
{
	message_private_on = value;
}

bool show_haloes()
{
	return haloes;
}

void set_show_haloes(bool value)
{
	haloes = value;
	preferences::set("show_haloes", value);
}

compression::format save_compression_format()
{
	const std::string& choice =
		preferences::get("compress_saves");

	// "yes" was used in 1.11.7 and earlier; the compress_saves
	// option used to be a toggle for gzip in those versions.
	if(choice.empty() || choice == "gzip" || choice == "yes") {
		return compression::GZIP;
	} else if(choice == "bzip2") {
		return compression::BZIP2;
	} else if(choice == "none" || choice == "no") { // see above
		return compression::NONE;
	} /*else*/

	// In case the preferences file was created by a later version
	// supporting some algorithm we don't; although why would anyone
	// playing a game need more algorithms, really...
	return compression::GZIP;
}

std::string get_chat_timestamp(const time_t& t) {
	if (chat_timestamping()) {
		if(preferences::use_twelve_hour_clock_format() == false) {
			return lg::get_timestamp(t, _("[%H:%M]")) + " ";
		}
		else {
			return lg::get_timestamp(t, _("[%I:%M %p]")) + " ";
		}
	}
	return "";
}

bool chat_timestamping() {
	return preferences::get("chat_timestamp", false);
}

void set_chat_timestamping(bool value) {
	preferences::set("chat_timestamp", value);
}

int chat_lines()
{
	return lexical_cast_default<int>(preferences::get("chat_lines"), 6);
}

void set_chat_lines(int lines)
{
	preferences::set("chat_lines", lines);
}

void set_chat_message_aging(const int aging)
{
	preferences::set("chat_message_aging", aging);
}

int chat_message_aging()
{
	return lexical_cast_default<int>(preferences::get("chat_message_aging"), 20);
}

bool show_all_units_in_help() {
	return preferences::get("show_all_units_in_help", false);
}

void set_show_all_units_in_help(bool value) {
	preferences::set("show_all_units_in_help", value);
}

std::set<std::string> &encountered_units() {
	return encountered_units_set;
}

std::set<t_translation::terrain_code> &encountered_terrains() {
	return encountered_terrains_set;
}

std::string custom_command() {
	return preferences::get("custom_command");
}

void set_custom_command(const std::string& command) {
	preferences::set("custom_command", command);
}

/**
 * Returns a pointer to the history vector associated with given id
 * making a new one if it doesn't exist.
 *
 * @todo FIXME only used for gui2. Could be used for the above histories.
 */
std::vector<std::string>* get_history(const std::string& id) {
	return &history_map[id];
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


void encounter_recruitable_units(const std::vector<team>& teams){
	for (std::vector<team>::const_iterator help_team_it = teams.begin();
		help_team_it != teams.end(); ++help_team_it) {
		help_team_it->log_recruitable();
		encountered_units_set.insert(help_team_it->recruits().begin(), help_team_it->recruits().end());
	}
}

void encounter_start_units(const unit_map& units){
	for (unit_map::const_iterator help_unit_it = units.begin();
		 help_unit_it != units.end(); ++help_unit_it) {
		encountered_units_set.insert(help_unit_it->type_id());
	}
}

static void encounter_recallable_units(const std::vector<team>& teams){
	for (const team& t : teams) {
		for (const unit_const_ptr & u : t.recall_list()) {
			encountered_units_set.insert(u->type_id());
		}
	}
}

void encounter_map_terrain(const gamemap& map)
{
	map.for_each_loc([&](const map_location& loc) {
		const t_translation::terrain_code terrain = map.get_terrain(loc);
		preferences::encountered_terrains().insert(terrain);
		for (t_translation::terrain_code t : map.underlying_union_terrain(loc)) {
			preferences::encountered_terrains().insert(t);
		}
	});
}

void encounter_all_content(const game_board & gameboard_) {
	preferences::encounter_recruitable_units(gameboard_.teams());
	preferences::encounter_start_units(gameboard_.units());
	preferences::encounter_recallable_units(gameboard_.teams());
	preferences::encounter_map_terrain(gameboard_.map());
}

void acquaintance::load_from_config(const config& cfg)
{
	nick_ = cfg["nick"].str();
	status_ = cfg["status"].str();
	notes_ = cfg["notes"].str();
}

void acquaintance::save(config& item)
{
	item["nick"] = nick_;
	item["status"] = status_;
	item["notes"] = notes_;
}

} // preferences namespace
