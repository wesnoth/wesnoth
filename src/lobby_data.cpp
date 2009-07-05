/* $Id$ */
/*
   Copyright (C) 2009 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "lobby_data.hpp"

#include "config.hpp"
#include "game_preferences.hpp"
#include "filesystem.hpp"
#include "foreach.hpp"
#include "formula_string_utils.hpp"
#include "gettext.hpp"
#include "network.hpp"
#include "log.hpp"
#include "map.hpp"
#include "map_exception.hpp"
#include "wml_exception.hpp"

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)

room_info::room_info(const std::string& name)
: name_(name), members_()
{
}

bool room_info::is_member(const std::string& user) const
{
	return members_.find(user) != members_.end();
}

void room_info::add_member(const std::string& user)
{
	members_.insert(user);
}

void room_info::remove_member(const std::string& user)
{
	members_.erase(user);
}

user_info::user_info()
	: name()
	, game_id()
	, location()
	, relation(ME)
	, state(LOBBY)
	, registered()
{
}

user_info::user_info(const config& c)
	: name(c["name"])
	, game_id(c["game_id"])
	, location(c["location"])
	, relation(ME)
	, state(c["available"] == "no" ? GAME : LOBBY)
	, registered(utils::string_bool(c["registered"]))
{
	if (name == preferences::login()) {
		relation = ME;
	} else if (preferences::is_ignored(name)) {
		relation = IGNORED;
	} else if (preferences::is_friend(name)) {
		relation = FRIEND;
	} else {
		relation = NEUTRAL;
	}
}

void user_info::update_state(const std::string& selected_game_id, const room_info* current_room /*= NULL*/)
{
	if (!game_id.empty() && game_id == selected_game_id) {
		state = SEL_GAME;
	} else if (state == LOBBY && current_room != NULL && current_room->is_member(name)) {
		state = SEL_ROOM;
	}
}

game_info::game_info() :
	mini_map(),
	id(),
	map_data(),
	name(),
	map_info(),
	map_info_size(),
	gold(),
	xp(),
	vision(),
	status(),
	time_limit(),
	vacant_slots(0),
	current_turn(0),
	reloaded(false),
	started(false),
	fog(false),
	shroud(false),
	observers(false),
	use_map_settings(false),
	verified(false),
	password_required(false),
	have_era(false)
{
}

namespace {

std::string make_short_name(const std::string long_name)
{
	if (long_name.empty()) return "";
	std::string sh;
	bool had_space = true;
	for (size_t i = 1; i < long_name.size(); ++i) {
		if (long_name[i] == ' ') {
			had_space = true;
		} else if (had_space && long_name[i] != '?') {
			sh += long_name[i];
			had_space = false;
		}
	}
	return sh;
}

} //end anonymous namespace

game_info::game_info(const config& game, const config& game_config)
: mini_map()
, id(game["id"])
, map_data(game["map_data"])
, name(game["name"])
, map_info()
, map_info_size()
, era()
, era_short()
, gold(game["mp_village_gold"])
, xp(game["experience_modifier"] + "%")
, vision()
, status()
, time_limit()
, vacant_slots(lexical_cast_default<size_t>(game["slots"], 0))
, current_turn(0)
, reloaded(game["savegame"] == "yes")
, started(false)
, fog(false)
, shroud(false)
, observers(game["observer"] != "no")
, use_map_settings(game["mp_use_map_settings"] == "yes")
, verified(false)
, password_required(game["password"] == "yes")
, have_era(true)
{
	std::string turn = game["turn"];
	std::string slots = game["slots"];
	if (!game["mp_era"].empty())
	{
		const config &era_cfg = game_config.find_child("era", "id", game["mp_era"]);
		utils::string_map symbols;
		symbols["era_id"] = game["mp_era"];
		if (era_cfg) {
			era = era_cfg["name"];
			era_short = era_cfg["short_name"];
			if (era_short.empty()) {
				era_short = make_short_name(era);
			}
		} else {
			have_era = (game["require_era"] == "no");
			era = vgettext("Unknown era: $era_id", symbols);
			era_short = "?" + make_short_name(era);
			verified = false;
		}
	} else {
		era = _("Unknown era");
		era_short = "??";
		verified = false;
	}
	map_info = era;

	if (map_data.empty()) {
		map_data = read_map(game["map"]);
	}

	if (map_data.empty()) {
		map_info += " - ??x??";
	} else {
		try {
			gamemap map(game_config, map_data);
			//mini_map = image::getMinimap(minimap_size_, minimap_size_, map, 0);
			map_info_size = lexical_cast_default<std::string, int>(map.w(), "??")
				+ std::string("x") + lexical_cast_default<std::string, int>(map.h(), "??");
			map_info += " - " + map_info_size;
		} catch (incorrect_map_format_exception &e) {
			ERR_CF << "illegal map: " << e.msg_ << "\n";
			verified = false;
		} catch (twml_exception& e) {
			ERR_CF <<  "map could not be loaded: " << e.dev_message << '\n';
			verified = false;
		}
	}
	map_info += " ";
	if (!game["mp_scenario"].empty())
	{
		// check if it's a multiplayer scenario
		const config *level_cfg = &game_config.find_child("multiplayer", "id", game["mp_scenario"]);
		if (!*level_cfg) {
			// check if it's a user map
			level_cfg = &game_config.find_child("generic_multiplayer", "id", game["mp_scenario"]);
		}
		if (*level_cfg) {
			map_info += level_cfg->get_attribute("name");
			// reloaded games do not match the original scenario hash,
			// so it makes no sense to test them, they always would appear
			// as remote scenarios
			if (!reloaded) {
				if (const config& hashes = game_config.child("multiplayer_hashes")) {
					std::string hash = game["hash"];
					bool hash_found = false;
					foreach (const config::attribute &i, hashes.attribute_range()) {
						if (i.first == game["mp_scenario"] && i.second == hash) {
							hash_found = true;
							break;
						}
					}
					if(!hash_found) {
						map_info += " - ";
						map_info += _("Remote scenario");
						verified = false;
					}
				}
			}
		} else {
			utils::string_map symbols;
			symbols["scenario_id"] = game["mp_scenario"];
			map_info += vgettext("Unknown scenario: $scenario_id", symbols);
			verified = false;
		}
	} else {
		map_info += _("Unknown scenario");
		verified = false;
	}
	if (reloaded) {
		map_info += " - ";
		map_info += _("Reloaded game");
		verified = false;
	}

	if (!turn.empty()) {
		started = true;
		int index = turn.find_first_of('/');
		if (index > -1){
			const std::string current_turn_string = turn.substr(0, index);
			current_turn = lexical_cast<unsigned int>(current_turn_string);
		}
		status = _("Turn ") + turn;
	} else {
		started = false;
		if (vacant_slots > 0) {
			status = std::string(_n("Vacant Slot:", "Vacant Slots:",
					vacant_slots)) + " " + game["slots"];
			if (password_required) {
				status += std::string(" (") + std::string(_("Password Required")) + ")";
			}
		}
	}

	if (game["mp_fog"] == "yes") {
		vision = _("Fog");
		fog = true;
		if (game["mp_shroud"] == "yes") {
			vision += "/";
			vision += _("Shroud");
			shroud = true;
		} else {
			shroud = false;
		}
	} else if (game["mp_shroud"] == "yes") {
		vision = _("Shroud");
		fog = false;
		shroud = true;
	} else {
		vision = _("none");
		fog = false;
		shroud = false;
	}
	if (game["mp_countdown"] == "yes" ) {
		time_limit =   game["mp_countdown_init_time"] + " / +"
		             + game["mp_countdown_turn_bonus"] + " "
		             + game["mp_countdown_action_bonus"];
	} else {
		time_limit = "";
	}
}

lobby_info::lobby_info(const config& game_config)
: game_config_(game_config), gamelist_(), gamelist_initialized_(false)
, rooms_(), games_(), users_()
{
}

void lobby_info::process_gamelist(const config &data)
{
	gamelist_ = data;
	gamelist_initialized_ = true;
	parse_gamelist();
}

bool lobby_info::process_gamelist_diff(const config &data)
{
	if (!gamelist_initialized_) return false;
	try {
		gamelist_.apply_diff(data);
	} catch(config::error& e) {
		ERR_CF << "Error while applying the gamelist diff: '"
			<< e.message << "' Getting a new gamelist.\n";
		network::send_data(config("refresh_lobby"), 0, true);
		return false;
	}
	parse_gamelist();
	return true;
}

void lobby_info::parse_gamelist()
{
	users_.clear();
	foreach (const config& c, gamelist_.child_range("user")) {
		users_.push_back(user_info(c));
	}
	games_.clear();
	foreach (const config& c, gamelist_.child("gamelist").child_range("game")) {
		games_.push_back(game_info(c, game_config_));
	}
}
