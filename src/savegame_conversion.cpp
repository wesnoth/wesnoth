/*
   Copyright (C) 2021 by demario
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "savegame_conversion.hpp"

#include "carryover.hpp"
#include "units/unit.hpp"
#include "units/make.hpp"
#include "units/types.hpp"
#include "units/attack_type.hpp"
#include "log.hpp"
#include "serialization/parser.hpp"
#include "game_version.hpp"

#include <iomanip>

static lg::log_domain log_enginerefac("enginerefac");
#define DBG_RG LOG_STREAM(debug, log_enginerefac)
#define LOG_RG LOG_STREAM(info, log_enginerefac)


namespace savegame {

config& replay_start_config(config& cfg)
{
	if(cfg.has_child("replay_start")) { return cfg.child("replay_start"); }
	else if(config& carryover_ = cfg.child("carryover_sides_start")) {
		// Copy from saved_game::expand_carryover()
		config& starting_point = cfg.child("scenario");
		carryover_info sides(carryover_);

		sides.transfer_to(starting_point);
		for(config& side_cfg : starting_point.child_range("side")) {
			sides.transfer_all_to(side_cfg);
		}

		carryover_ = sides.to_config();
	}
	return cfg.child("scenario");
}

std::string convert_version_to_suffix(version_info loaded_version)
{
	std::string suffix = "";

	if(loaded_version < version_info("1.13.0") && loaded_version >= version_info("1.11.0"))
		suffix = "_1_12";

	return suffix;
}

std::string get_era_suffix(version_info loaded_version)
{
	std::string suffix = convert_version_to_suffix(loaded_version);
	if(loaded_version < version_info("1.15.0") && loaded_version >= version_info("1.13.0"))
		suffix = "_unitless_1_14";

	return suffix;
}

std::string append_suffix_to_era_id(config& cfg)
{
 	std::string mp_era = "era_default";
 	std::ostringstream ss;

	version_info loaded_version(cfg["version"]);
	if(config& multiplayer = cfg.child("multiplayer")) { mp_era = multiplayer["mp_era"].str(); }
	else if(cfg.has_attribute("campaign_type") && cfg.has_child("replay_start")) {
		if(cfg["campaign_type"] == "scenario") { mp_era = "era_default"; }
	}
	{
		if(mp_era.find("RBY", 0) != std::string::npos) {
			mp_era = "era_default";
		}
 		ss << mp_era;
		if(mp_era == "era_default") {
			ss << get_era_suffix(loaded_version);
		} else if(mp_era == "ladder_era") {
 			// Pre-0.1.17 Ladder era maintained its own versioning for unit types
 			ss << "_0_17";
		}
	}
	return ss.str();
}

std::string append_suffix_to_unit_type(std::string unit_type, std::string suffix)
{
 	bool suffix_found, ladder_found, null_found;
 	std::string sep = "";
 	std::ostringstream ss;
	for(const std::string& ut : utils::split(unit_type, ',', utils::REMOVE_EMPTY))
	{
		ss << sep << ut;
		sep = ",";

		null_found = (ut.find("null", 0) != std::string::npos);
		suffix_found = (ut.find(suffix, 0) != std::string::npos);
		ladder_found = (ut.find("Ladder", 0) != std::string::npos);
		if(suffix_found || ladder_found || null_found) { continue; }

		ss << suffix;
	}
	return ss.str();
}

void update_start_unit_types(config& cfg)
{
	version_info loaded_version(cfg["version"]);
	config& replay_start = replay_start_config(cfg);
	if(!convert_version_to_suffix(loaded_version).empty())
	{
		for(config& side : replay_start.child_range("side"))
		{
 			// BFW 1.4, 1.6
			side["leader"] = append_suffix_to_unit_type(side["leader"].str(), loaded_version);
 			if (side.has_attribute("recruit"))
			{
				side["recruit"] = append_suffix_to_unit_type(side["recruit"].str(), loaded_version);
			}
 			if (side.has_attribute("random_leader"))
			{
				side["random_leader"] = append_suffix_to_unit_type(side["random_leader"].str(), loaded_version);
			}
 			if (side.has_attribute("type")) side["type"] = append_suffix_to_unit_type(side["type"].str(), loaded_version);

 			// BFW 1.8
 			if (side.has_attribute("previous_recruits"))
			{
				side["previous_recruits"] = append_suffix_to_unit_type(side["previous_recruits"].str(), loaded_version);
			}
			for(config& unit : side.child_range("unit"))
			{
				unit["type"] = append_suffix_to_unit_type(unit["type"].str(), loaded_version);
				unit["advances_to"] = append_suffix_to_unit_type(unit["advances_to"].str(), loaded_version);
			}
		}
 		if (config& variables = replay_start.child("variables")) {
 			if (config& side1 = variables.child("side1")) {
				side1["recruit"] = append_suffix_to_unit_type(side1["recruit"].str(), loaded_version);
			}
 			if (config& side2 = variables.child("side2")) {
				side2["recruit"] = append_suffix_to_unit_type(side2["recruit"].str(), loaded_version);
			}
		}
	}
}

void update_replay_unit_types(config& cfg)
{
	version_info loaded_version(cfg["version"]);
	if(convert_version_to_suffix(loaded_version).empty()) return;
	for(config& replay : cfg.child_range("replay"))
	{
	for(config& command : replay.child_range("command"))
	{
 		// BFW 1.12
 		if (config& recruit = command.child("recruit"))
		{
			recruit["type"] = append_suffix_to_unit_type(recruit["type"].str(), loaded_version);
		}
		if (config& attack = command.child("attack"))
		{
			if (attack.has_attribute("attacker_type"))
			{
				attack["attacker_type"] = append_suffix_to_unit_type(attack["attacker_type"].str(), loaded_version);
				attack["defender_type"] = append_suffix_to_unit_type(attack["defender_type"].str(), loaded_version);
			}
		}
	}
	}
}

void update_prestart_detect_factions(config& cfg)
{
	if(config& replay = replay_start_config(cfg))
	{
		for(config& event: replay.child_range("event"))
		{
			if(event["name"] == "prestart")
			{
				if(config& fire_event = event.child("fire_event"))
				{
					if(fire_event["name"] == "place_units")
					{
						fire_event["name"] = "place_units_replay_compatible";
					}
				}
			}
		}
	}
}

void update_pre_recruited_loyals(config& cfg)
{
	version_info loaded_version(cfg["version"]);
	if(convert_version_to_suffix(loaded_version).empty()) return;
	if(config& replay = replay_start_config(cfg))
	{
		for(config& event: replay.child_range("event"))
		{
			if(event["name"] == "place_units")
			{
				for(config& switch_: event.child_range("switch"))
				{
					for(config& case_: switch_.child_range("case"))
					{
						for(config& unit: case_.child_range("unit"))
						{
							unit["type"] = append_suffix_to_unit_type(unit["type"].str(), loaded_version);
						}
					}
					if(config& else_ = switch_.child("else"))
					{
						for(config& unit: else_.child_range("unit"))
						{
							unit["type"] = append_suffix_to_unit_type(unit["type"].str(), loaded_version);
						}
					}
				}
			}
		}
	}
}

//changes done during 1.13.0-dev
static void convert_old_saves_1_13_0(config& cfg)
{
	if(config& carryover_sides_start = cfg.child("carryover_sides_start"))
	{
		if(!carryover_sides_start.has_attribute("next_underlying_unit_id"))
		{
			carryover_sides_start["next_underlying_unit_id"] = cfg["next_underlying_unit_id"];
		}
	}
	if(cfg.child_or_empty("snapshot").empty())
	{
		cfg.clear_children("snapshot");
	}
	if(cfg.child_or_empty("replay_start").empty())
	{
		cfg.clear_children("replay_start");
	}
	if(config& snapshot = cfg.child("snapshot"))
	{
		//make [end_level] -> [end_level_data] since its alo called [end_level_data] in the carryover.
		if(config& end_level = cfg.child("end_level") )
		{
			snapshot.add_child("end_level_data", end_level);
			snapshot.clear_children("end_level");
		}
		//if we have a snapshot then we already applied carryover so there is no reason to keep this data.
		if(cfg.has_child("carryover_sides_start"))
		{
			cfg.clear_children("carryover_sides_start");
		}
	}
	if(!cfg.has_child("snapshot") && !cfg.has_child("replay_start"))
	{
		cfg.clear_children("carryover_sides");
	}
	//This code is needed because for example otherwise it won't find the (empty) era
	if(!cfg.has_child("multiplayer")) {
		cfg.add_child("multiplayer", config {
			"mp_era", "era_blank",
			"mp_use_map_settings", true,
		});
	}
}


//changes done during 1.13.0+dev
static void convert_old_saves_1_13_1(config& cfg)
{
	if(config& multiplayer = cfg.child("multiplayer")) {
		if(multiplayer["mp_era"] == "era_blank") {
			multiplayer["mp_era"] = "era_default";
		}
	}
	//This currently only fixes start-of-scenario saves.
	if(config& carryover_sides_start = cfg.child("carryover_sides_start"))
	{
		for(config& side : carryover_sides_start.child_range("side"))
		{
			for(config& unit : side.child_range("unit"))
			{
				if(config& modifications = unit.child("modifications"))
				{
					for(config& advancement : modifications.child_range("advance"))
					{
						modifications.add_child("advancement", advancement);
					}
					modifications.clear_children("advance");
				}
			}
		}
	}
	for(config& snapshot : cfg.child_range("snapshot")) {
		if (snapshot.has_attribute("used_items")) {
			config used_items;
			for(const std::string& item : utils::split(snapshot["used_items"])) {
				used_items[item] = true;
			}
			snapshot.remove_attribute("used_items");
			snapshot.add_child("used_items", used_items);
		}
	}
}

static void convert_old_saves_1_13_1_plus(config& cfg)
{
	unsigned int random_number_offset = 0;
	for(config& replay: cfg.child_range("replay"))
	{
		for(config& command : replay.child_range("command"))
		{
			if(command.has_child("recruit") || command.has_child("auto_shroud"))
			{
				command.clear_children("checkup");
			}
			if(config& recruit = command.child("recruit"))
			{
				random_number_offset = 0;
				std::string recruited_unit_race = unit_types.find(recruit["type"])->race_id();
				if(!recruited_unit_race.compare(/*pos=*/ 0, /*len=*/ 3, "orc")
					|| !recruited_unit_race.compare(/*pos=*/ 0, /*len=*/ 4, "ogre")
					|| !recruited_unit_race.compare(/*pos=*/ 0, /*len=*/ 4, "wolf")
					|| !recruited_unit_race.compare(/*pos=*/ 0, /*len=*/ 4, "wose")
					|| !recruited_unit_race.compare(/*pos=*/ 0, /*len=*/ 5, "dwarf")
					|| !recruited_unit_race.compare(/*pos=*/ 0, /*len=*/ 5, "troll")
					|| !recruited_unit_race.compare(/*pos=*/ 0, /*len=*/ 6, "goblin")
					|| !recruited_unit_race.compare(/*pos=*/ 0, /*len=*/ 6, "lizard"))
				{
					random_number_offset = 8;
				}
			}
			if(config& checkup = command.child("mp_checkup"))
			{
				if(checkup.has_attribute("random_calls")) { checkup["random_calls"] = checkup["random_calls"] + random_number_offset; random_number_offset = 0; }
			}
		}
	}
	if(config& replay = replay_start_config(cfg))
	{
		for(config& event: replay.child_range("event"))
		{
			if(event["name"] == "prestart")
			{
				if(config& lua = event.child("lua"))
				{
					std::string code = lua["code"];
					std::string original_path = "add-ons/Ladder_Era/lua/";
					std::string backward_support_path = "add-ons/Support_Past_Default_Era/lua/Ladder_Era_";
					std::size_t found = code.find(original_path, 0);
					if(found != std::string::npos)
					{
						code.replace(found, original_path.length(), backward_support_path);
						lua["code"] = code;
					}
				}
			}
		}
	}
}


void convert_earlier_version_saves(config& cfg)
{
	if(config& multiplayer = cfg.child("multiplayer")) {
		if(multiplayer["mp_era"] == "era_khalifate") {
			return;
		}
	}
	version_info loaded_version(cfg["version"]);
	// '<= version_info("1.13.0")' doesn't work
	//because version_info cannot handle 1.13.0-dev versions correctly.
	if(loaded_version < version_info("1.13.1"))
	{
		convert_old_saves_1_13_0(cfg);
	}
	if(loaded_version <= version_info("1.13.1"))
	{
		convert_old_saves_1_13_1(cfg);
	}
	if(loaded_version < version_info("1.15.0"))
	{
		convert_old_saves_1_13_1_plus(cfg);
	}

	if(!convert_version_to_suffix(loaded_version).empty())
	{
		update_start_unit_types(cfg);
		update_replay_unit_types(cfg);

		update_prestart_detect_factions(cfg);
		update_pre_recruited_loyals(cfg);
		if(config& multiplayer = cfg.child("multiplayer")) {
			multiplayer["mp_era"] = append_suffix_to_era_id(cfg);
		}
	}
	// To initialize {class replay: public rand_rng::rng} with replay version
	// The replay version needs to be passed to rng for RNG backward support
	if (config& replay = cfg.child("replay"))
	{
		replay["version"] = cfg["version"];
	}
}

}
