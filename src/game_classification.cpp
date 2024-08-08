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

#include "game_classification.hpp"

#include "config.hpp"
#include "log.hpp"
#include "preferences/preferences.hpp"
#include "serialization/string_utils.hpp"
#include "game_version.hpp"
#include "game_config_manager.hpp"

#include <list>

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)

/** The default difficulty setting for campaigns. */
const std::string DEFAULT_DIFFICULTY("NORMAL");

game_classification::game_classification(const config& cfg)
	: label(cfg["label"])
	, version(cfg["version"])
	, type(campaign_type::get_enum(cfg["campaign_type"].str()).value_or(campaign_type::type::scenario))
	, campaign_define(cfg["campaign_define"])
	, campaign_xtra_defines(utils::split(cfg["campaign_extra_defines"]))
	, scenario_define(cfg["scenario_define"])
	, era_define(cfg["era_define"])
	, mod_defines(utils::split(cfg["mod_defines"]))
	, active_mods(utils::split(cfg["active_mods"]))
	, era_id(cfg["era_id"])
	, campaign(cfg["campaign"])
	, campaign_name(cfg["campaign_name"])
	, abbrev(cfg["abbrev"])
	, end_credits(cfg["end_credits"].to_bool(true))
	, end_text(cfg["end_text"])
	, end_text_duration(std::clamp<unsigned>(cfg["end_text_duration"].to_unsigned(0), 0, 5000))
	, difficulty(cfg["difficulty"].empty() ? DEFAULT_DIFFICULTY : cfg["difficulty"].str())
	, random_mode(cfg["random_mode"])
	, oos_debug(cfg["oos_debug"].to_bool(false))
{
}

config game_classification::to_config() const
{
	config cfg;
	cfg["label"] = label;
	cfg["version"] = game_config::wesnoth_version.str();
	cfg["campaign_type"] = campaign_type::get_string(type);
	cfg["campaign_define"] = campaign_define;
	cfg["campaign_extra_defines"] = utils::join(campaign_xtra_defines);
	cfg["scenario_define"] = scenario_define;
	cfg["era_define"] = era_define;
	cfg["mod_defines"] = utils::join(mod_defines);
	cfg["active_mods"] = utils::join(active_mods);
	cfg["era_id"] = era_id;
	cfg["campaign"] = campaign;
	cfg["campaign_name"] = campaign_name;
	cfg["abbrev"] = abbrev;
	cfg["end_credits"] = end_credits;
	cfg["end_text"] = end_text;
	cfg["end_text_duration"] = std::to_string(end_text_duration);
	cfg["difficulty"] = difficulty;
	cfg["random_mode"] = random_mode;
	cfg["oos_debug"] = oos_debug;
	cfg["core"] = prefs::get().core();

	return cfg;
}

std::string game_classification::get_tagname() const
{
	if(is_multiplayer()) {
		return campaign.empty() ? campaign_type::multiplayer : campaign_type::scenario;
	}

	if(is_tutorial()) {
		return campaign_type::scenario;
	}

	return campaign_type::get_string(type);
}

namespace
{
// helper objects for saved_game::expand_mp_events()
struct modevents_entry
{
	modevents_entry(const std::string& _type, const std::string& _id)
		: type(_type)
		, id(_id)
	{
	}

	std::string type;
	std::string id;
};
}

std::set<std::string> game_classification::active_addons(const std::string& scenario_id) const
{
	//FIXME: this doesn't include mods from the current scenario.
	std::list<modevents_entry> mods;
	std::set<std::string> loaded_resources;
	std::set<std::string> res;

	for(const auto& mod : active_mods) {
		mods.emplace_back("modification", mod);
	}

	// We don't want the error message below if there is no era (= if this is a sp game).
	if(!era_id.empty()) {
		mods.emplace_back(get_tagname(), scenario_id);
	}

	if(!era_id.empty()) {
		mods.emplace_back("era", era_id);
	}

	if(!campaign.empty()) {
		mods.emplace_back("campaign", campaign);
	}
	while(!mods.empty()) {

		const modevents_entry& current = mods.front();
		if(current.type == "resource") {
			if(!loaded_resources.insert(current.id).second) {
				mods.pop_front();
				continue;
			}
		}
		if(auto cfg = game_config_manager::get()->game_config().find_child(current.type, "id", current.id)) {
			if(!cfg["addon_id"].empty()) {
				res.insert(cfg["addon_id"]);
			}
			for (const config& load_res : cfg->child_range("load_resource")) {
				mods.emplace_back("resource", load_res["id"].str());
			}
		} else {
			ERR_NG << "Unable to find config for content " << current.id << " of type " << current.type;
		}
		mods.pop_front( );
	}

	DBG_NG << "Active content for game set to:";
	for(const std::string& mod : res) {
		DBG_NG << mod;
	}

	return res;
}
