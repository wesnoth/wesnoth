/*
   Copyright (C) 2009 - 2016 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "gui/dialogs/lobby/data.hpp"

#include "config.hpp"
#include "game_preferences.hpp"
#include "filesystem.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "map/exception.hpp"
#include "terrain/type_data.hpp"
#include "wml_exception.hpp"
#include "version.hpp"

#include <iterator>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)
static lg::log_domain log_engine("engine");
#define WRN_NG LOG_STREAM(warn, log_engine)

static lg::log_domain log_lobby("lobby");
#define DBG_LB LOG_STREAM(info, log_lobby)
#define LOG_LB LOG_STREAM(info, log_lobby)
#define ERR_LB LOG_STREAM(err, log_lobby)

chat_message::chat_message(const time_t& timestamp,
						   const std::string& user,
						   const std::string& message)
	: timestamp(timestamp), user(user), message(message)
{
}

chat_log::chat_log() : history_()
{
}

void chat_log::add_message(const time_t& timestamp,
						   const std::string& user,
						   const std::string& message)
{
	history_.push_back(chat_message(timestamp, user, message));
}


void chat_log::add_message(const std::string& user, const std::string& message)
{
	add_message(time(nullptr), user, message);
}

void chat_log::clear()
{
	history_.clear();
}

room_info::room_info(const std::string& name) : name_(name), members_(), log_()
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

void room_info::process_room_members(const config& data)
{
	members_.clear();
	for(const auto & m : data.child_range("member"))
	{
		members_.insert(m["name"]);
	}
}

user_info::user_info(const config& c)
	: name(c["name"])
	, game_id(c["game_id"])
	, relation(ME)
	, state(game_id == 0 ? LOBBY : GAME)
	, registered(c["registered"].to_bool())
	, observing(c["status"] == "observing")
{
	update_relation();
}

void user_info::update_state(int selected_game_id,
							 const room_info* current_room /*= nullptr*/)
{
	if(game_id != 0) {
		if(game_id == selected_game_id) {
			state = SEL_GAME;
		} else {
			state = GAME;
		}
	} else {
		if(current_room != nullptr && current_room->is_member(name)) {
			state = SEL_ROOM;
		} else {
			state = LOBBY;
		}
	}
	update_relation();
}

void user_info::update_relation()
{
	if(name == preferences::login()) {
		relation = ME;
	} else if(preferences::is_ignored(name)) {
		relation = IGNORED;
	} else if(preferences::is_friend(name)) {
		relation = FRIEND;
	} else {
		relation = NEUTRAL;
	}
}

namespace
{

std::string make_short_name(const std::string& long_name)
{
	if(long_name.empty())
		return "";
	std::string sh;
	bool had_space = true;
	for(size_t i = 1; i < long_name.size(); ++i) {
		if(long_name[i] == ' ') {
			had_space = true;
		} else if(had_space && long_name[i] != '?') {
			sh += long_name[i];
			had_space = false;
		}
	}
	return sh;
}

} // end anonymous namespace

// local_item is either an [era] or [modification] tag, something with addon_version and addon_id.
// (These are currently added at add-on loading time in the game_config_manager.)
// It is checked whether the local item's add-on version is required for this game, and if the
// versions are compatible. If it's not, a record is made in the req_list passed as argument.
static game_info::ADDON_REQ check_addon_version_compatibility(const config& local_item, const config& game, std::vector<game_info::required_addon>& req_list)
{
	if(!local_item.has_attribute("addon_id") || !local_item.has_attribute("addon_version")) {
		return game_info::SATISFIED;
	}

	if(const config& game_req = game.find_child("addon", "id", local_item["addon_id"])) {
		// Record object which we will potentially store for this check
		game_info::required_addon r;
		r.addon_id = local_item["addon_id"].str();

		const version_info local_ver(local_item["addon_version"].str());
		version_info local_min_ver(local_item.has_attribute("addon_min_version") ? local_item["addon_min_version"] : local_item["addon_version"]);
		// If UMC didn't specify last compatible version, assume no backwards compatibility.
		if(local_min_ver > local_ver) {
			// Some sanity checking regarding min version. If the min ver doens't make sense, ignore it.
			local_min_ver = local_ver;
		}

		const version_info remote_ver(game_req["version"].str());
		version_info remote_min_ver(game_req.has_attribute("min_version") ? game_req["min_version"] : game_req["version"]);
		if(remote_min_ver > remote_ver) {
			remote_min_ver = remote_ver;
		}

		// Check if the host is too out of date to play.
		if(local_min_ver > remote_ver) {
			r.outcome = game_info::CANNOT_SATISFY;

			utils::string_map symbols;
			symbols["addon"] = r.addon_id; // TODO: Figure out how to ask the add-on manager for the user-friendly name of this add-on.
			symbols["host_ver"] = remote_ver.str();
			symbols["local_ver"] = local_ver.str();
			r.message = vgettext("Host's version of $addon is too old: host's version $host_ver < your version $local_ver.", symbols);
			req_list.push_back(r);
			return r.outcome;
		}

		// Check if our version is too out of date to play.
		if(remote_min_ver > local_ver) {
			r.outcome = game_info::NEED_DOWNLOAD;

			utils::string_map symbols;
			symbols["addon"] = r.addon_id; // TODO: Figure out how to ask the add-on manager for the user-friendly name of this add-on.
			symbols["host_ver"] = remote_ver.str();
			symbols["local_ver"] = local_ver.str();
			r.message = vgettext("Your version of $addon is out of date: host's version $host_ver > your version $local_ver.", symbols);
			req_list.push_back(r);
			return r.outcome;
		}
	}

	return game_info::SATISFIED;
}

game_info::game_info(const config& game, const config& game_config, const std::vector<std::string>& installed_addons)
	: mini_map()
	, id(game["id"])
	, map_data(game["map_data"])
	, name(game["name"])
	, scenario()
	, remote_scenario(false)
	, map_info()
	, map_size_info()
	, era()
	, era_short()
	, gold(game["mp_village_gold"])
	, support(game["mp_village_support"])
	, xp(game["experience_modifier"].str() + "%")
	, vision()
	, status()
	, time_limit()
	, vacant_slots(lexical_cast_default<int>(game["slots"], 0)) // Can't use to_int() here.
	, current_turn(0)
	, reloaded(game["savegame"].to_bool())
	, started(false)
	, fog(game["mp_fog"].to_bool())
	, shroud(game["mp_shroud"].to_bool())
	, observers(game["observer"].to_bool(true))
	, shuffle_sides(game["shuffle_sides"].to_bool(true))
	, use_map_settings(game["mp_use_map_settings"].to_bool())
	, registered_users_only(game["registered_users_only"].to_bool())
	, verified(true)
	, password_required(game["password"].to_bool())
	, have_era(true)
	, have_all_mods(true)
	, has_friends(false)
	, has_ignored(false)
	, display_status(NEW)
{
	for(const config& addon : game.child_range("addon")) {
		if(addon.has_attribute("id")) {
			if(std::find(installed_addons.begin(), installed_addons.end(), addon["id"].str()) == installed_addons.end()) {
				required_addon r;
				r.addon_id = addon["id"].str();
				r.outcome = NEED_DOWNLOAD;

				utils::string_map symbols;
				symbols["id"] = addon["id"].str();
				r.message = vgettext("Missing addon: $id", symbols);
				addons.push_back(r);
				if(addons_outcome == SATISFIED) {
					addons_outcome = NEED_DOWNLOAD;
				}
			}
		}
	}

	std::string turn = game["turn"];
	if(!game["mp_era"].empty()) {
		const config& era_cfg = game_config.find_child("era", "id", game["mp_era"]);
		utils::string_map symbols;
		symbols["era_id"] = game["mp_era"];
		if(era_cfg) {
			era = era_cfg["name"].str();
			era_short = era_cfg["short_name"].str();
			if(era_short.empty()) {
				era_short = make_short_name(era);
			}

			ADDON_REQ result = check_addon_version_compatibility(era_cfg, game, addons);
			addons_outcome = std::max(addons_outcome, result); // Elevate to most severe error level encountered so far
		} else {
			have_era = !game["require_era"].to_bool(true);
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

	if(!game.child_or_empty("modification").empty()) {
		for(const config &cfg : game.child_range("modification")) {
			if(cfg["require_modification"].to_bool(false)) {
				const config &mod = game_config.find_child("modification", "id", cfg["id"]);
				if(!mod) {
					have_all_mods = false;
					break;
				}
				ADDON_REQ result = check_addon_version_compatibility(mod, game, addons);
				addons_outcome = std::max(addons_outcome, result); //elevate to most severe error level encountered so far
			}
		}
	}

	if(map_data.empty()) {
		map_data = filesystem::read_map(game["mp_scenario"]);
	}

	if(map_data.empty()) {
		map_info += " — ??×??";
	} else {
		try {
			gamemap map(std::make_shared<terrain_type_data>(game_config), map_data);
			// mini_map = image::getMinimap(minimap_size_, minimap_size_, map,
			// 0);
			std::ostringstream msi;
			msi << map.w() << utils::unicode_multiplication_sign << map.h();
			map_size_info = msi.str();
			map_info += " — " + map_size_info;
		} catch(incorrect_map_format_error& e) {
			ERR_CF << "illegal map: " << e.message << std::endl;
			verified = false;
		} catch(twml_exception& e) {
			ERR_CF << "map could not be loaded: " << e.dev_message << '\n';
			verified = false;
		}
	}
	map_info += " ";
	if(!game["mp_scenario"].empty()) {
		// Check if it's a multiplayer scenario
		const config* level_cfg = &game_config.find_child("multiplayer", "id", game["mp_scenario"]);
		if(!*level_cfg) {
			// Check if it's a user map
			level_cfg = &game_config.find_child("generic_multiplayer", "id", game["mp_scenario"]);
		}
		if(*level_cfg) {
			scenario = (*level_cfg)["name"].str();
			map_info += scenario;
			// Reloaded games do not match the original scenario hash, so it makes no sense
			// to test them, since they always would appear as remote scenarios
			if(!reloaded) {
				if(const config& hashes = game_config.child("multiplayer_hashes")) {
					std::string hash = game["hash"];
					bool hash_found = false;
					for(const auto & i : hashes.attribute_range()) {
						if(i.first == game["mp_scenario"] && i.second == hash) {
							hash_found = true;
							break;
						}
					}

					if(!hash_found) {
						remote_scenario = true;
						map_info += " — ";
						map_info += _("Remote scenario");
						verified = false;
					}
				}
			}

			if((*level_cfg)["require_scenario"].to_bool(false)) {
				ADDON_REQ result = check_addon_version_compatibility((*level_cfg), game, addons);
				addons_outcome = std::max(addons_outcome, result); //elevate to most severe error level encountered so far
			}
		} else {
			utils::string_map symbols;
			symbols["scenario_id"] = game["mp_scenario"];
			scenario = vgettext("Unknown scenario: $scenario_id", symbols);
			map_info += scenario;
			verified = false;
		}
	} else {
		scenario = _("Unknown scenario");
		map_info += scenario;
		verified = false;
	}

	if(reloaded) {
		map_info += " — ";
		map_info += _("Reloaded game");
		verified = false;
	}

	if(!turn.empty()) {
		started = true;
		int index = turn.find_first_of('/');
		if(index > -1) {
			const std::string current_turn_string = turn.substr(0, index);
			current_turn = lexical_cast<unsigned int>(current_turn_string);
		}
		status = _("Turn ") + turn;
	} else {
		started = false;
		if(vacant_slots > 0) {
			status = std::string(_n("Vacant Slot:", "Vacant Slots:", vacant_slots)) + " " + game["slots"];
		}
	}

	if(fog) {
		vision = _("Fog");
		if(shroud) {
			vision += "/";
			vision += _("Shroud");
		}
	} else if(shroud) {
		vision = _("Shroud");
	} else {
		vision = _("none");
	}

	if(game["mp_countdown"].to_bool()) {
		time_limit = game["mp_countdown_init_time"].str() + "+"
					 + game["mp_countdown_turn_bonus"].str() + "/"
					 + game["mp_countdown_action_bonus"].str();
	} else {
		time_limit = "";
	}
}

bool game_info::can_join() const
{
	return have_era && have_all_mods && !started && vacant_slots > 0;
}

bool game_info::can_observe() const
{
	return (have_era && have_all_mods && observers) || preferences::is_authenticated();
}

const char* game_info::display_status_string() const
{
	switch(display_status) {
		case game_info::CLEAN:
			return "clean";
		case game_info::NEW:
			return "new";
		case game_info::DELETED:
			return "deleted";
		case game_info::UPDATED:
			return "updated";
		default:
			ERR_CF << "BAD display_status " << display_status << " in game " << id << "\n";
			return "?";
	}
}

bool game_info::match_string_filter(const std::string& filter) const
{
	const std::string& s1 = map_info;
	const std::string& s2 = name;
	return std::search(s1.begin(), s1.end(), filter.begin(), filter.end(),
			chars_equal_insensitive) != s1.end()
	    || std::search(s2.begin(), s2.end(), filter.begin(), filter.end(),
			chars_equal_insensitive) != s2.end();
}
