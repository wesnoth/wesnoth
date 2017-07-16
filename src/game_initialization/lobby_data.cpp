/*
   Copyright (C) 2009 - 2017 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "lexical_cast.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "map/exception.hpp"
#include "terrain/type_data.hpp"
#include "utils/general.hpp"
#include "wml_exception.hpp"
#include "version.hpp"

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

chat_message::chat_message(const time_t& timestamp,
						   const std::string& user,
						   const std::string& message)
	: timestamp(timestamp), user(user), message(message)
{
}

chat_session::chat_session() : history_()
{
}

void chat_session::add_message(const time_t& timestamp,
						   const std::string& user,
						   const std::string& message)
{
	history_.emplace_back(timestamp, user, message);
}


void chat_session::add_message(const std::string& user, const std::string& message)
{
	add_message(time(nullptr), user, message);
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

// Returns an abbreviated form of the provided string - ie, 'Ageless Era' should become 'AE'
static std::string make_short_name(const std::string& long_name)
{
	if(long_name.empty()) {
		return "";
	}

	size_t pos = 0;

	std::stringstream ss;
	ss << long_name[pos];

	while(pos < long_name.size()) {
		pos = long_name.find(' ', pos + 1);

		if(pos <= long_name.size() - 2) {
			ss << long_name[pos + 1];
		}
	}

	return ss.str();
}

game_info::game_info(const config& game, const config& game_config, const std::vector<std::string>& installed_addons)
	: id(game["id"])
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
	, required_addons()
	, addons_outcome(SATISFIED)
{
	const auto parse_requirements = [&](const config& c, const std::string& id_key) {
		if(c.has_attribute(id_key)) {
			if(std::find(installed_addons.begin(), installed_addons.end(), c[id_key].str()) == installed_addons.end()) {
				required_addon r;
				r.addon_id = c[id_key].str();
				r.outcome = NEED_DOWNLOAD;

				r.message = vgettext("Missing addon: $id", {{"id", c[id_key].str()}});
				required_addons.push_back(r);
				if(addons_outcome == SATISFIED) {
					addons_outcome = NEED_DOWNLOAD;
				}
			}
		}
	};

	for(const config& addon : game.child_range("addon")) {
		parse_requirements(addon, "id");
	}

	/*
	 * Modifications have a different format than addons. The id and addon_id are keys sent by the
	 * server, so we have to parse them separately here and add them to the required_addons vector.
	 */
	for(const config& mod : game.child_range("modification")) {
		parse_requirements(mod, "addon_id");
	}

	std::string turn = game["turn"];
	if(!game["mp_era"].empty()) {
		const config& era_cfg = game_config.find_child("era", "id", game["mp_era"]);
		if(era_cfg) {
			era = era_cfg["name"].str();
			era_short = era_cfg["short_name"].str();
			if(era_short.empty()) {
				era_short = make_short_name(era);
			}

			ADDON_REQ result = check_addon_version_compatibility(era_cfg, game);
			addons_outcome = std::max(addons_outcome, result); // Elevate to most severe error level encountered so far
		} else {
			have_era = !game["require_era"].to_bool(true);
			era = vgettext("Unknown era: $era_id", {{"era_id", game["mp_era_addon_id"].str()}});
			era_short = make_short_name(era);
			verified = false;

			addons_outcome = NEED_DOWNLOAD;
		}
	} else {
		era = _("Unknown era");
		era_short = "??";
		verified = false;
	}

	std::stringstream info_stream;
	info_stream << era;

	if(!game.child_or_empty("modification").empty()) {
		for(const config& cfg : game.child_range("modification")) {
			if(const config& mod = game_config.find_child("modification", "id", cfg["id"])) {
				mod_info += (mod_info.empty() ? "" : ", ") + mod["name"].str();

				if(cfg["require_modification"].to_bool(false)) {
					ADDON_REQ result = check_addon_version_compatibility(mod, game);
					addons_outcome = std::max(addons_outcome, result); // Elevate to most severe error level encountered so far
				}
			} else {
				mod_info += (mod_info.empty() ? "" : ", ") + cfg["id"].str();

				if(cfg["require_modification"].to_bool(false)) {
					have_all_mods = false;
					mod_info += " " + _("(missing)");

					addons_outcome = NEED_DOWNLOAD;
				}
			}
		}
	}

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
			info_stream << " — " + map_size_info;
		} catch(incorrect_map_format_error& e) {
			ERR_CF << "illegal map: " << e.message << std::endl;
			verified = false;
		} catch(wml_exception& e) {
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

		// Check if it's a user map
		if(!*level_cfg) {
			level_cfg = &game_config.find_child("generic_multiplayer", "id", game["mp_scenario"]);
		}

		if(*level_cfg) {
			scenario = formatter() << "<b>" << _("(S)") << "</b>" << " " << (*level_cfg)["name"].str();
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
						info_stream << " — ";
						info_stream << _("Remote scenario");
						verified = false;
					}
				}
			}

			if((*level_cfg)["require_scenario"].to_bool(false)) {
				ADDON_REQ result = check_addon_version_compatibility((*level_cfg), game);
				addons_outcome = std::max(addons_outcome, result); // Elevate to most severe error level encountered so far
			}
		} else {
			scenario = vgettext("Unknown scenario: $scenario_id", {{"scenario_id", game["mp_scenario_name"].str()}});
			info_stream << scenario;
			verified = false;
		}
	} else if(!game["mp_campaign"].empty()) {
		if(const config& level_cfg = game_config.find_child("campaign", "id", game["mp_campaign"])) {
			std::stringstream campaign_text;
			campaign_text
				<< "<b>" << _("(C)") << "</b>" << " "
				<< level_cfg["name"] << " — "
				<< game["mp_scenario_name"];

			// Difficulty
			config difficulties = gui2::dialogs::generate_difficulty_config(level_cfg);
			for(const config& difficulty : difficulties.child_range("difficulty")) {
				if(difficulty["define"] == game["difficulty_define"]) {
					campaign_text << " — " << difficulty["description"];

					break;
				}
			}

			scenario = campaign_text.str();
			info_stream << campaign_text.rdbuf();

			// TODO: should we have this?
			//if(game["require_scenario"].to_bool(false)) {
				ADDON_REQ result = check_addon_version_compatibility(level_cfg, game);
				addons_outcome = std::max(addons_outcome, result); // Elevate to most severe error level encountered so far
			//}
		} else {
			scenario = vgettext("Unknown campaign: $campaign_id", {{"campaign_id", game["mp_campaign"].str()}});
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
		info_stream << " — ";
		info_stream << _("Reloaded game");
		verified = false;
	}

	if(!turn.empty()) {
		started = true;
		int index = turn.find_first_of('/');
		if(index > -1) {
			const std::string current_turn_string = turn.substr(0, index);
			current_turn = lexical_cast<unsigned int>(current_turn_string);
		}
		status = _("Turn") + " " + turn;
	} else {
		started = false;
		if(vacant_slots > 0) {
			status = _n("Vacant Slot:", "Vacant Slots:", vacant_slots) + " " + game["slots"];
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
		time_limit = formatter()
			<< game["mp_countdown_init_time"].str() << "+"
			<< game["mp_countdown_turn_bonus"].str() << "/"
			<< game["mp_countdown_action_bonus"].str();
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
		// Also apply some sanity checking regarding min version; if the min ver doens't make sense, ignore it.
		local_min_ver = std::min(local_min_ver, local_ver);

		// Remote version
		const version_info remote_ver(game_req["version"].str());
		version_info remote_min_ver(game_req.has_attribute("min_version") ? game_req["min_version"] : game_req["version"]);

		remote_min_ver = std::min(remote_min_ver, remote_ver);

		// Check if the host is too out of date to play.
		if(local_min_ver > remote_ver) {
			r.outcome = CANNOT_SATISFY;

			// TODO: Figure out how to ask the add-on manager for the user-friendly name of this add-on.
			r.message = vgettext("The host's version of <i>$addon</i> is incompatible. They have version <b>$host_ver</b> while you have version <b>$local_ver</b>.", {
				{"addon",     r.addon_id},
				{"host_ver",  remote_ver.str()},
				{"local_ver", local_ver.str()}
			});

			required_addons.push_back(r);
			return r.outcome;
		}

		// Check if our version is too out of date to play.
		if(remote_min_ver > local_ver) {
			r.outcome = NEED_DOWNLOAD;

			// TODO: Figure out how to ask the add-on manager for the user-friendly name of this add-on.
			r.message = vgettext("Your version of <i>$addon</i> is incompatible. You have version <b>$local_ver</b> while the host has version <b>$host_ver</b>.", {
				{"addon", r.addon_id},
				{"host_ver", remote_ver.str()},
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

}
