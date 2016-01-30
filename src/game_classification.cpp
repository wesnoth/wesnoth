/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"
#include "game_classification.hpp"
#include "game_config.hpp"
#include "util.hpp"
#include "serialization/string_utils.hpp"
#include "log.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)

/// The default difficulty setting for campaigns.
const std::string DEFAULT_DIFFICULTY("NORMAL");

game_classification::game_classification():
	savegame_config(),
	label(),
	version(),
	campaign_type(),
	campaign_define(),
	campaign_xtra_defines(),
	scenario_define(),
	era_define(),
	mod_defines(),
	campaign(),
	abbrev(),
	end_credits(true),
	end_text(),
	end_text_duration(),
	difficulty(DEFAULT_DIFFICULTY),
	random_mode(""),
	oos_debug(false)
	{}

game_classification::game_classification(const config& cfg):
	savegame_config(),
	label(cfg["label"]),
	version(cfg["version"]),
	campaign_type(cfg["campaign_type"].to_enum<game_classification::CAMPAIGN_TYPE>(game_classification::CAMPAIGN_TYPE::SCENARIO)),
	campaign_define(cfg["campaign_define"]),
	campaign_xtra_defines(utils::split(cfg["campaign_extra_defines"])),
	scenario_define(cfg["scenario_define"]),
	era_define(cfg["era_define"]),
	mod_defines(utils::split(cfg["mod_defines"])),
	campaign(cfg["campaign"]),
	abbrev(cfg["abbrev"]),
	end_credits(cfg["end_credits"].to_bool(true)),
	end_text(cfg["end_text"]),
	end_text_duration(cfg["end_text_duration"]),
	difficulty(cfg["difficulty"].empty() ? DEFAULT_DIFFICULTY : cfg["difficulty"].str()),
	random_mode(cfg["random_mode"]),
	oos_debug(cfg["oos_debug"].to_bool(false))
	{}

game_classification::game_classification(const game_classification& gc):
	savegame_config(),
	label(gc.label),
	version(gc.version),
	campaign_type(gc.campaign_type),
	campaign_define(gc.campaign_define),
	campaign_xtra_defines(gc.campaign_xtra_defines),
	scenario_define(gc.scenario_define),
	era_define(gc.era_define),
	mod_defines(gc.mod_defines),
	campaign(gc.campaign),
	abbrev(gc.abbrev),
	end_credits(gc.end_credits),
	end_text(gc.end_text),
	end_text_duration(gc.end_text_duration),
	difficulty(gc.difficulty),
	random_mode(gc.random_mode),
	oos_debug(gc.oos_debug)
{
}

config game_classification::to_config() const
{
	config cfg;

	cfg["label"] = label;
	cfg["version"] = game_config::version;
	cfg["campaign_type"] = lexical_cast<std::string> (campaign_type);
	cfg["campaign_define"] = campaign_define;
	cfg["campaign_extra_defines"] = utils::join(campaign_xtra_defines);
	cfg["scenario_define"] = scenario_define;
	cfg["era_define"] = era_define;
	cfg["mod_defines"] = utils::join(mod_defines);
	cfg["campaign"] = campaign;
	cfg["abbrev"] = abbrev;
	cfg["end_credits"] = end_credits;
	cfg["end_text"] = end_text;
	cfg["end_text_duration"] = str_cast<unsigned int>(end_text_duration);
	cfg["difficulty"] = difficulty;
	cfg["random_mode"] = random_mode;
	cfg["oos_debug"] = oos_debug;
	return cfg;
}

std::string game_classification::get_tagname() const
{
	if(this->campaign_type == CAMPAIGN_TYPE::MULTIPLAYER) {
		return this->campaign.empty() ? "multiplayer" : "scenario";
	}
	else {
		return this->campaign_type.to_string();
	}
}

bool game_classification::is_normal_mp_game() const
{
	return this->campaign_type == CAMPAIGN_TYPE::MULTIPLAYER && this->campaign.empty();
}
