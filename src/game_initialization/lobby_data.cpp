/*
   Copyright (C) 2009 - 2018 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "game_initialization/lobby_data.hpp"

#include "config.hpp"
#include "preferences/credentials.hpp"
#include "preferences/game.hpp"
#include "gui/dialogs/campaign_difficulty.hpp"
#include "filesystem.hpp"
#include "font/pango/escape.hpp"
#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "game_config_manager.hpp"
#include "gettext.hpp"
#include "lexical_cast.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "map/exception.hpp"
#include "terrain/type_data.hpp"
#include "wml_exception.hpp"
#include "game_version.hpp"

#include <iterator>

#include <boost/algorithm/string.hpp>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)
static lg::log_domain log_engine("engine");
#define WRN_NG LOG_STREAM(warn, log_engine)

static lg::log_domain log_lobby("lobby");
#define DBG_LB LOG_STREAM(info, log_lobby)
#define LOG_LB LOG_STREAM(info, log_lobby)
#define ERR_LB LOG_STREAM(err, log_lobby)

namespace mp {

chat_message::chat_message(const std::time_t& timestamp,
						   const std::string& user,
						   const std::string& message)
	: timestamp(timestamp), user(user), message(message)
{
}

chat_session::chat_session() : history_()
{
}

void chat_session::add_message(const std::time_t& timestamp,
						   const std::string& user,
						   const std::string& message)
{
	history_.emplace_back(timestamp, user, message);
}


void chat_session::add_message(const std::string& user, const std::string& message)
{
	add_message(std::time(nullptr), user, message);
}

void chat_session::clear()
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

bool user_info::operator<(const user_info& b) const
{
	return relation < b.relation || (relation == b.relation && translation::icompare(name, b.name) < 0);
}

namespace
{
const std::string& spaced_em_dash()
{
	static const std::string res = " " + font::unicode_em_dash + " ";
	return res;
}

std::string make_game_type_marker(std::string text, bool color_for_missing)
{
	if(color_for_missing) {
		return formatter() << "<b><span color='#f00'>[" << text << "]</span></b> ";
	} else {
		return formatter() << "<b>[" << text << "]</b> ";
	}
}

} // end anon namespace

game_info::game_info(const config& game, const std::vector<std::string>& installed_addons)
	: id(game["id"])
	, map_data(game["map_data"])
	, name(font::escape_text(game["name"]))
	, scenario()
	, remote_scenario(false)
	, map_info()
	, map_size_info()
	, era()
	, gold(game["mp_village_gold"])
	, support(game["mp_village_support"])
	, xp(game["experience_modifier"].str() + "%")
	, vision()
	, status()
	, time_limit()
	, vacant_slots()
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
	, required_addons()
	, addons_outcome(SATISFIED)
{
	const config& game_config = game_config_manager::get()->game_config();

	// Parse the list of addons required to join this game.
	for(const config& addon : game.child_range("addon")) {
		if(addon.has_attribute("id")) {
			if(std::find(installed_addons.begin(), installed_addons.end(), addon["id"].str()) == installed_addons.end()) {
				required_addon r;
				r.addon_id = addon["id"].str();
				r.outcome = NEED_DOWNLOAD;

				// Use addon name if provided, else fall back on the addon id.
				if(addon.has_attribute("name")) {
					r.message = VGETTEXT("Missing addon: $name", {{"name", addon["name"].str()}});
				} else {
					r.message = VGETTEXT("Missing addon: $id", {{"id", addon["id"].str()}});
				}

				required_addons.push_back(std::move(r));

				if(addons_outcome == SATISFIED) {
					addons_outcome = NEED_DOWNLOAD;
				}
			}
		}
	}

	if(!game["mp_era"].empty()) {
		const config& era_cfg = game_config.find_child("era", "id", game["mp_era"]);
		const bool require = game["require_era"].to_bool(true);
		if(era_cfg) {
			era = era_cfg["name"].str();

			if(require) {
				ADDON_REQ result = check_addon_version_compatibility(era_cfg, game);
				addons_outcome = std::max(addons_outcome, result); // Elevate to most severe error level encountered so far
			}
		} else {
			have_era = !require;
			era = game["mp_era_name"].str();
			verified = false;

			if(!have_era) {
				addons_outcome = NEED_DOWNLOAD;
			}
		}
	} else {
		era = _("Unknown era");
		verified = false;
	}

	std::stringstream info_stream;
	info_stream << era;

	for(const config& cfg : game.child_range("modification")) {
		mod_info.emplace_back(cfg["name"].str(), true);
		info_stream << ' ' << mod_info.back().first;

		if(cfg["require_modification"].to_bool(false)) {
			if(const config& mod = game_config.find_child("modification", "id", cfg["id"])) {
				ADDON_REQ result = check_addon_version_compatibility(mod, game);
				addons_outcome = std::max(addons_outcome, result); // Elevate to most severe error level encountered so far
			} else {
				have_all_mods = false;
				mod_info.back().second = false;

				addons_outcome = NEED_DOWNLOAD;
			}
		}
	}

	std::sort(mod_info.begin(), mod_info.end(), [](const auto& lhs, const auto& rhs) {
		return translation::icompare(lhs.first, rhs.first) < 0;
	});

	info_stream << ' ';

	if(map_data.empty()) {
		map_data = filesystem::read_map(game["mp_scenario"]);
	}

	if(map_data.empty()) {
		info_stream << " — ??×??";
	} else {
		try {
			gamemap map(std::make_shared<terrain_type_data>(game_config), map_data);
			std::ostringstream msi;
			msi << map.w() << font::unicode_multiplication_sign << map.h();
			map_size_info = msi.str();
			info_stream << spaced_em_dash() << map_size_info;
		} catch(const incorrect_map_format_error&) {
			verified = false;
		} catch(const wml_exception& e) {
			ERR_CF << "map could not be loaded: " << e.dev_message << '\n';
			verified = false;
		}
	}

	info_stream << " ";

	//
	// Check scenarios and campaigns
	//
	if(!game["mp_scenario"].empty() && game["mp_campaign"].empty()) {
		// Check if it's a multiplayer scenario
		const config* level_cfg = &game_config.find_child("multiplayer", "id", game["mp_scenario"]);
		const bool require = game["require_scenario"].to_bool(false);

		// Check if it's a user map
		if(!*level_cfg) {
			level_cfg = &game_config.find_child("generic_multiplayer", "id", game["mp_scenario"]);
		}

		if(*level_cfg) {
			scenario = formatter() << make_game_type_marker(_("scenario_abbreviation^S"), false) << (*level_cfg)["name"].str();
			info_stream << scenario;

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
						info_stream << spaced_em_dash();
						info_stream << _("Remote scenario");
						verified = false;
					}
				}
			}

			if(require) {
				ADDON_REQ result = check_addon_version_compatibility((*level_cfg), game);
				addons_outcome = std::max(addons_outcome, result); // Elevate to most severe error level encountered so far
			}
		} else {
			if(require) {
				addons_outcome = std::max(addons_outcome, NEED_DOWNLOAD); // Elevate to most severe error level encountered so far
			}
			scenario = formatter() << make_game_type_marker(_("scenario_abbreviation^S"), true) << game["mp_scenario_name"].str();
			info_stream << scenario;
			verified = false;
		}
	} else if(!game["mp_campaign"].empty()) {
		if(const config& campaign_cfg = game_config.find_child("campaign", "id", game["mp_campaign"])) {
			std::stringstream campaign_text;
			campaign_text
				<< make_game_type_marker(_("campaign_abbreviation^C"), false)
				<< campaign_cfg["name"] << spaced_em_dash()
				<< game["mp_scenario_name"];

			// Difficulty
			config difficulties = gui2::dialogs::generate_difficulty_config(campaign_cfg);
			for(const config& difficulty : difficulties.child_range("difficulty")) {
				if(difficulty["define"] == game["difficulty_define"]) {
					campaign_text << spaced_em_dash() << difficulty["description"];

					break;
				}
			}

			scenario = campaign_text.str();
			info_stream << campaign_text.rdbuf();

			// TODO: should we have this?
			//if(game["require_scenario"].to_bool(false)) {
				ADDON_REQ result = check_addon_version_compatibility(campaign_cfg, game);
				addons_outcome = std::max(addons_outcome, result); // Elevate to most severe error level encountered so far
			//}
		} else {
			scenario = formatter() << make_game_type_marker(_("campaign_abbreviation^C"), true) << game["mp_campaign_name"].str();
			info_stream << scenario;
			verified = false;
		}
	} else {
		scenario = _("Unknown scenario");
		info_stream << scenario;
		verified = false;
	}

	// Remove any newlines that might have been in game titles
	boost::replace_all(scenario, "\n", " " + font::unicode_em_dash + " ");

	if(reloaded) {
		info_stream << spaced_em_dash();
		info_stream << _("Reloaded game");
		verified = false;
	}

	// These should always be present in the data the server sends, but may or may not be empty.
	// I'm just using child_or_empty here to preempt any cases where they might not be included.
	const config& s = game.child_or_empty("slot_data");
	const config& t = game.child_or_empty("turn_data");

	if(!s.empty()) {
		started = false;

		vacant_slots = s["vacant"].to_unsigned();

		if(vacant_slots > 0) {
			status = formatter() << _n("Vacant Slot:", "Vacant Slots:", vacant_slots) << " " << vacant_slots << "/" << s["max"];
		} else {
			status = _("mp_game_available_slots^Full");
		}
	}

	if(!t.empty()) {
		started = true;

		current_turn = t["current"].to_unsigned();
		const int max_turns = t["max"].to_int();

		if(max_turns > -1) {
			status = formatter() << _("Turn") << " " << t["current"] << "/" << max_turns;
		} else {
			status = formatter() << _("Turn") << " " << t["current"];
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
		vision = _("vision^none");
	}

	if(game["mp_countdown"].to_bool()) {
		time_limit = formatter()
			<< game["mp_countdown_init_time"].str() << "+"
			<< game["mp_countdown_turn_bonus"].str() << "/"
			<< game["mp_countdown_action_bonus"].str();
	} else {
		time_limit = _("time limit^none");
	}

	map_info = info_stream.str();
}

game_info::ADDON_REQ game_info::check_addon_version_compatibility(const config& local_item, const config& game)
{
	if(!local_item.has_attribute("addon_id") || !local_item.has_attribute("addon_version")) {
		return SATISFIED;
	}

	if(const config& game_req = game.find_child("addon", "id", local_item["addon_id"])) {
		required_addon r {local_item["addon_id"].str(), SATISFIED, ""};

		// Local version
		const version_info local_ver(local_item["addon_version"].str());
		version_info local_min_ver(local_item.has_attribute("addon_min_version") ? local_item["addon_min_version"] : local_item["addon_version"]);

		// If the UMC didn't specify last compatible version, assume no backwards compatibility.
		// Also apply some sanity checking regarding min version; if the min ver doesn't make sense, ignore it.
		local_min_ver = std::min(local_min_ver, local_ver);

		// Remote version
		const version_info remote_ver(game_req["version"].str());
		version_info remote_min_ver(game_req.has_attribute("min_version") ? game_req["min_version"] : game_req["version"]);

		remote_min_ver = std::min(remote_min_ver, remote_ver);

		// Check if the host is too out of date to play.
		if(local_min_ver > remote_ver) {
			DBG_LB << "r.outcome = CANNOT_SATISFY for item='" << local_item["id"]
				<< "' addon='" << local_item["addon_id"]
				<< "' addon_min_version='" << local_item["addon_min_version"]
				<< "' addon_min_version_parsed='" << local_min_ver.str()
				<< "' addon_version='" << local_item["addon_version"]
				<< "' remote_ver='" << remote_ver.str()
				<< "'\n";
			r.outcome = CANNOT_SATISFY;

			r.message = VGETTEXT("The host's version of <i>$addon</i> is incompatible. They have version <b>$host_ver</b> while you have version <b>$local_ver</b>.", {
				{"addon",     local_item["addon_title"].str()},
				{"host_ver",  remote_ver.str()},
				{"local_ver", local_ver.str()}
			});

			required_addons.push_back(r);
			return r.outcome;
		}

		// Check if our version is too out of date to play.
		if(remote_min_ver > local_ver) {
			r.outcome = NEED_DOWNLOAD;

			r.message = VGETTEXT("Your version of <i>$addon</i> is incompatible. You have version <b>$local_ver</b> while the host has version <b>$host_ver</b>.", {
				{"addon",     local_item["addon_title"].str()},
				{"host_ver",  remote_ver.str()},
				{"local_ver", local_ver.str()}
			});

			required_addons.push_back(r);
			return r.outcome;
		}
	}

	return SATISFIED;
}

bool game_info::can_join() const
{
	return !started && vacant_slots > 0;
}

bool game_info::can_observe() const
{
	return observers || preferences::is_authenticated();
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
	const std::string& s1 = name;
	const std::string& s2 = map_info;
	return std::search(s1.begin(), s1.end(), filter.begin(), filter.end(),
			chars_equal_insensitive) != s1.end()
	    || std::search(s2.begin(), s2.end(), filter.begin(), filter.end(),
			chars_equal_insensitive) != s2.end();
}

}
