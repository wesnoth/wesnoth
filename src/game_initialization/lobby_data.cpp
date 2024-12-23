/*
	Copyright (C) 2009 - 2024
	by Tomasz Sniatowski <kailoran@gmail.com>
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
#include "filesystem.hpp"
#include "font/pango/escape.hpp"
#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "game_config_manager.hpp"
#include "game_config_view.hpp"
#include "game_initialization/multiplayer.hpp"
#include "game_version.hpp"
#include "gettext.hpp"
#include "gui/dialogs/campaign_difficulty.hpp"
#include "log.hpp"
#include "map/exception.hpp"
#include "map/map.hpp"
#include "mp_game_settings.hpp"
#include "preferences/preferences.hpp"
#include "serialization/markup.hpp"
#include "wml_exception.hpp"


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

user_info::user_info(const config& c)
	: name(c["name"])
	, forum_id(c["forum_id"].to_int())
	, game_id(c["game_id"].to_int())
	, registered(c["registered"].to_bool())
	, observing(c["status"] == "observing")
	, moderator(c["moderator"].to_bool(false))
{
}

user_info::user_state user_info::get_state(int selected_game_id) const
{
	if(game_id == 0) {
		return user_state::LOBBY;
	} else if(game_id == selected_game_id) {
		return user_state::SEL_GAME;
	} else {
		return user_state::GAME;
	}
}

user_info::user_relation user_info::get_relation() const
{
	if(name == prefs::get().login()) {
		return user_relation::ME;
	} else if(prefs::get().is_ignored(name)) {
		return user_relation::IGNORED;
	} else if(prefs::get().is_friend(name)) {
		return user_relation::FRIEND;
	} else {
		return user_relation::NEUTRAL;
	}
}

bool user_info::operator<(const user_info& b) const
{
	const auto ar = get_relation();
	const auto br = b.get_relation();
	return ar < br || (ar == br && translation::icompare(name, b.name) < 0);
}

namespace
{
const std::string& spaced_em_dash()
{
	static const std::string res = " " + font::unicode_em_dash + " ";
	return res;
}

std::string make_game_type_marker(const std::string& text, bool color_for_missing)
{
	if(color_for_missing) {
		return markup::span_color("#f00", markup::bold("[", text, "] "));
	} else {
		return markup::bold("[", text, "] ");
	}
}

} // end anon namespace

game_info::game_info(const config& game, const std::vector<std::string>& installed_addons)
	: id(game["id"].to_int())
	, map_data(game["map_data"])
	, name(font::escape_text(game["name"].str()))
	, scenario()
	, type_marker()
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
	, reloaded(saved_game_mode::get_enum(game["savegame"].str()).value_or(saved_game_mode::type::no) != saved_game_mode::type::no)
	, started(false)
	, fog(game["mp_fog"].to_bool())
	, shroud(game["mp_shroud"].to_bool())
	, observers(game["observer"].to_bool(true))
	, shuffle_sides(game["shuffle_sides"].to_bool(true))
	, use_map_settings(game["mp_use_map_settings"].to_bool())
	, private_replay(game["private_replay"].to_bool())
	, verified(true)
	, password_required(game["password"].to_bool())
	, have_era(true)
	, have_all_mods(true)
	, has_friends(false)
	, has_ignored(false)
	, auto_hosted(game["auto_hosted"].to_bool())
	, display_status(disp_status::NEW)
	, required_addons()
	, addons_outcome(addon_req::SATISFIED)
{
	const game_config_view& game_config = game_config_manager::get()->game_config();

	// Parse the list of addons required to join this game.
	for(const config& addon : game.child_range("addon")) {
		if(addon.has_attribute("id") && addon["required"].to_bool(false)) {
			if(std::find(installed_addons.begin(), installed_addons.end(), addon["id"].str()) == installed_addons.end()) {
				required_addon r;
				r.addon_id = addon["id"].str();
				r.outcome = addon_req::NEED_DOWNLOAD;

				// Use addon name if provided, else fall back on the addon id.
				if(addon.has_attribute("name")) {
					r.message = VGETTEXT("Missing addon: $name", {{"name", addon["name"].str()}});
				} else {
					r.message = VGETTEXT("Missing addon: $id", {{"id", addon["id"].str()}});
				}

				required_addons.push_back(std::move(r));

				if(addons_outcome == addon_req::SATISFIED) {
					addons_outcome = addon_req::NEED_DOWNLOAD;
				}
			}
		}
	}

	if(!game["mp_era"].empty()) {
		auto era_cfg = game_config.find_child("era", "id", game["mp_era"]);
		const bool require = game["require_era"].to_bool(true);
		if(era_cfg) {
			era = era_cfg["name"].str();

			if(require) {
				addon_req result = check_addon_version_compatibility(*era_cfg, game);
				addons_outcome = std::max(addons_outcome, result); // Elevate to most severe error level encountered so far
			}
		} else {
			have_era = !require;
			era = game["mp_era_name"].str();
			verified = false;

			if(!have_era) {
				addons_outcome = addon_req::NEED_DOWNLOAD;
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

		if(cfg["require_modification"].to_bool(true)) {
			if(auto mod = game_config.find_child("modification", "id", cfg["id"])) {
				addon_req result = check_addon_version_compatibility(*mod, game);
				addons_outcome = std::max(addons_outcome, result); // Elevate to most severe error level encountered so far
			} else {
				have_all_mods = false;
				mod_info.back().second = false;

				addons_outcome = addon_req::NEED_DOWNLOAD;
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
			gamemap map(map_data);
			std::ostringstream msi;
			msi << map.w() << font::unicode_multiplication_sign << map.h();
			map_size_info = msi.str();
			info_stream << spaced_em_dash() << map_size_info;
		} catch(const incorrect_map_format_error&) {
			verified = false;
		} catch(const wml_exception& e) {
			ERR_CF << "map could not be loaded: " << e.dev_message;
			verified = false;
		}
	}

	info_stream << " ";

	//
	// Check scenarios and campaigns
	//
	if(!game["mp_scenario"].empty() && game["mp_campaign"].empty()) {
		// Check if it's a multiplayer scenario
		const config* level_cfg = game_config.find_child("multiplayer", "id", game["mp_scenario"]).ptr();
		const bool require = game["require_scenario"].to_bool(false);

		// Check if it's a user map
		if(!level_cfg) {
			level_cfg = game_config.find_child("generic_multiplayer", "id", game["mp_scenario"]).ptr();
		}

		if(level_cfg) {
			type_marker = make_game_type_marker(_("scenario_abbreviation^S"), false);
			scenario = (*level_cfg)["name"].str();
			info_stream << scenario;

			// Reloaded games do not match the original scenario hash, so it makes no sense
			// to test them, since they always would appear as remote scenarios
			if(!reloaded) {
				if(auto hashes = game_config.optional_child("multiplayer_hashes")) {
					std::string hash = game["hash"];
					bool hash_found = false;
					for(const auto & i : hashes->attribute_range()) {
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
				addon_req result = check_addon_version_compatibility((*level_cfg), game);
				addons_outcome = std::max(addons_outcome, result); // Elevate to most severe error level encountered so far
			}
		} else {
			if(require) {
				addons_outcome = std::max(addons_outcome, addon_req::NEED_DOWNLOAD); // Elevate to most severe error level encountered so far
			}
			type_marker = make_game_type_marker(_("scenario_abbreviation^S"), true);
			scenario = game["mp_scenario_name"].str();
			info_stream << scenario;
			verified = false;
		}
	} else if(!game["mp_campaign"].empty()) {
		if(auto campaign_cfg = game_config.find_child("campaign", "id", game["mp_campaign"])) {
			type_marker = make_game_type_marker(_("campaign_abbreviation^C"), false);

			std::stringstream campaign_text;
			campaign_text
				<< campaign_cfg["name"] << spaced_em_dash()
				<< game["mp_scenario_name"];

			// Difficulty
			config difficulties = gui2::dialogs::generate_difficulty_config(*campaign_cfg);
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
				addon_req result = check_addon_version_compatibility(*campaign_cfg, game);
				addons_outcome = std::max(addons_outcome, result); // Elevate to most severe error level encountered so far
			//}
		} else {
			type_marker = make_game_type_marker(_("campaign_abbreviation^C"), true);
			scenario = game["mp_campaign_name"].str();
			info_stream << scenario;
			verified = false;
		}
	} else {
		scenario = _("Unknown scenario");
		info_stream << scenario;
		verified = false;
	}

	// Remove any newlines that might have been in game names (the player-set ones)
	// No idea how this could happen, but I've seen it (vultraz, 2020-10-26)
	boost::erase_all(name, "\n");

	// Remove any newlines that might have been in game titles (scenario/campaign name, etc.)
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

game_info::addon_req game_info::check_addon_version_compatibility(const config& local_item, const config& game)
{
	if(!local_item.has_attribute("addon_id") || !local_item.has_attribute("addon_version")) {
		return addon_req::SATISFIED;
	}

	if(auto game_req = game.find_child("addon", "id", local_item["addon_id"])) {
		if(!game_req["required"].to_bool(false)) {
			return addon_req::SATISFIED;
		}

		required_addon r{local_item["addon_id"].str(), addon_req::SATISFIED, ""};

		// Local version
		const version_info local_ver(local_item["addon_version"].str());
		version_info local_min_ver(local_item.has_attribute("addon_min_version") ? local_item["addon_min_version"] : local_item["addon_version"]);

		// If the UMC didn't specify last compatible version, assume no backwards compatibility.
		// Also apply some sanity checking regarding min version; if the min ver doesn't make sense, ignore it.
		local_min_ver = std::min(local_min_ver, local_ver);

		// Remote version
		const version_info remote_ver(game_req["version"].str());
		version_info remote_min_ver(game_req->has_attribute("min_version") ? game_req["min_version"] : game_req["version"]);

		remote_min_ver = std::min(remote_min_ver, remote_ver);

		// Check if the host is too out of date to play.
		if(local_min_ver > remote_ver) {
			DBG_LB << "r.outcome = CANNOT_SATISFY for item='" << local_item["id"]
				<< "' addon='" << local_item["addon_id"]
				<< "' addon_min_version='" << local_item["addon_min_version"]
				<< "' addon_min_version_parsed='" << local_min_ver.str()
				<< "' addon_version='" << local_item["addon_version"]
				<< "' remote_ver='" << remote_ver.str()
				<< "'";
			r.outcome = addon_req::CANNOT_SATISFY;

			r.message = VGETTEXT("The host’s version of <i>$addon</i> is incompatible. They have version <b>$host_ver</b> while you have version <b>$local_ver</b>.", {
				{"addon",     local_item["addon_title"].str()},
				{"host_ver",  remote_ver.str()},
				{"local_ver", local_ver.str()}
			});

			required_addons.push_back(r);
			return r.outcome;
		}

		// Check if our version is too out of date to play.
		if(remote_min_ver > local_ver) {
			r.outcome = addon_req::NEED_DOWNLOAD;

			r.message = VGETTEXT("Your version of <i>$addon</i> is incompatible. You have version <b>$local_ver</b> while the host has version <b>$host_ver</b>.", {
				{"addon",     local_item["addon_title"].str()},
				{"host_ver",  remote_ver.str()},
				{"local_ver", local_ver.str()}
			});

			required_addons.push_back(r);
			return r.outcome;
		}
	}

	return addon_req::SATISFIED;
}

bool game_info::can_join() const
{
	return !started && vacant_slots > 0;
}

bool game_info::can_observe() const
{
	return observers || mp::logged_in_as_moderator();
}

const char* game_info::display_status_string() const
{
	switch(display_status) {
		case game_info::disp_status::CLEAN:
			return "clean";
		case game_info::disp_status::NEW:
			return "new";
		case game_info::disp_status::DELETED:
			return "deleted";
		case game_info::disp_status::UPDATED:
			return "updated";
		default:
			ERR_CF << "BAD display_status " << static_cast<int>(display_status) << " in game " << id;
			return "?";
	}
}

bool game_info::match_string_filter(const std::string& filter) const
{
	const std::string& s1 = name;
	const std::string& s2 = map_info;
	return translation::ci_search(s1, filter) || translation::ci_search(s2, filter);
}

}
