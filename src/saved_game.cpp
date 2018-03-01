/*
   Copyright (C) 2003 - 2018 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * 	Some information about savefiles:
 *
 *	A savefile can contain:
 *
 *  - General information (toplevel attributes, [multiplayer])
 *	  This is present in all savefiles
 *
 *  - [statistics]
 *    This is present in all savefiles but it's not handled by playcampaign/play_controller/saved_game.
 *    It's handled by savegame.cpp
 *
 *  - [snapshot]
 *    If a savegame was saved during a scenario this contains a snapshot of the game at the point when
 *    it was saved.
 *
 *  - [carryover_sides_start]
 *    At start-of-scenario saves this contains data from the previous scenario that was preserved.
 *
 *  - [carryover_sides]
 *    In savefile made during the game, this tag contains data from [carryover_sides_start] that was not
 *    used in the current scenario but should be saved for a next scenario
 *
 *  - [replay_start]
 *    A snapshot made very early to replay the game from.
 *
 *  - [replay]
 *    A record of game actions that was made between the creation of [replay_start] and [snapshot].
 *
 *
 * The following types of savegames are known:
 *
 * - Start of scenario savefiles
 *   These files only contain general information, statistics, and [carryover_sides_start]. When these
 *   saves are loaded, the scenario data is loaded form the game config using the next_scenario attribute
 *   from [carryover_sides_start].
 *
 * - Expanded Start of scenario savefiles
 *   Similar to normal Start-of-scenario savefiles, but the also contain a [scenario] that contains the
 *   scenario data. This type is only used internally and usually doesn't get written to the disk.
 *
 * - In-game savefile
 *   These files contain general information, statistics, [snapshot], [replay], [replay_start], [snapshot],
 *   and [carryover_sides]. These files don't contain a [carryover_sides_start] because both starting points
 *   ([replay_start] and [snapshot]) were made after [carryover_sides_start] was merged into the scenario.
 *
 * - Replay savefiles
 *   Like a in-game save made during linger mode, but without the [snapshot].
 */

#include "saved_game.hpp"

#include "carryover.hpp"
#include "config.hpp"
#include "cursor.hpp"
#include "formula/string_utils.hpp"
#include "game_config_manager.hpp"
#include "generators/map_create.hpp"
#include "log.hpp"
#include "random.hpp"
#include "serialization/binary_or_text.hpp"
#include "statistics.hpp"
#include "variable.hpp" // for config_variable_set
#include "variable_info.hpp"

#include <cassert>

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)

saved_game::saved_game()
	: has_carryover_expanded_(false)
	, carryover_(carryover_info().to_config())
	, replay_start_()
	, classification_()
	, mp_settings_()
	, starting_pos_type_(STARTINGPOS_NONE)
	, starting_pos_()
	, replay_data_()
{
}

saved_game::saved_game(config cfg)
	: has_carryover_expanded_(false)
	, carryover_()
	, replay_start_()
	, classification_(cfg)
	, mp_settings_()
	, starting_pos_type_(STARTINGPOS_NONE)
	, starting_pos_()
	, replay_data_()

{
	set_data(cfg);
}

saved_game::saved_game(const saved_game& state)
	: has_carryover_expanded_(state.has_carryover_expanded_)
	, carryover_(state.carryover_)
	, replay_start_(state.replay_start_)
	, classification_(state.classification_)
	, mp_settings_(state.mp_settings_)
	, starting_pos_type_(state.starting_pos_type_)
	, starting_pos_(state.starting_pos_)
	, replay_data_(state.replay_data_)
{
}

void saved_game::set_carryover_sides_start(config carryover_sides_start)
{
	carryover_.swap(carryover_sides_start);
	has_carryover_expanded_ = false;
}

void saved_game::set_random_seed()
{
	if(has_carryover_expanded_ || !carryover_["random_seed"].empty()) {
		return;
	}

	carryover_["random_seed"] = randomness::generator->get_random_int(0, INT_MAX);
	carryover_["random_calls"] = 0;
}

void saved_game::write_config(config_writer& out) const
{
	write_general_info(out);
	write_starting_pos(out);

	if(!this->replay_start_.empty()) {
		out.write_child("replay_start", replay_start_);
	}

	out.open_child("replay");
	replay_data_.write(out);

	out.close_child("replay");
	write_carryover(out);
}

void saved_game::write_starting_pos(config_writer& out) const
{
	if(starting_pos_type_ == STARTINGPOS_SNAPSHOT) {
		out.write_child("snapshot", starting_pos_);
	} else if(starting_pos_type_ == STARTINGPOS_SCENARIO) {
		out.write_child("scenario", starting_pos_);
	}
}

void saved_game::write_carryover(config_writer& out) const
{
	assert(not_corrupt());
	out.write_child(has_carryover_expanded_ ? "carryover_sides" : "carryover_sides_start", carryover_);
}

void saved_game::write_general_info(config_writer& out) const
{
	out.write(classification_.to_config());
	out.write_child("multiplayer", mp_settings_.to_config());
}

void saved_game::set_defaults()
{
	const bool is_loaded_game = this->starting_pos_type_ != STARTINGPOS_SCENARIO;
	const bool is_multiplayer_tag = classification().get_tagname() == "multiplayer";

	static const std::vector<std::string> team_defaults {
		"carryover_percentage",
		"carryover_add",
	};

	for(config& side : starting_pos_.child_range("side")) {
		// Set save_id default value directly after loading to its default to prevent different default behaviour in
		// mp_connect code and sp code.
		if(side["save_id"].empty()) {
			side["save_id"] = side["id"];
		}

		if(!is_multiplayer_tag && side["side_name"].blank()) {
			side["side_name"] = side["name"];
		}

		if(!is_loaded_game && !side["current_player"].empty()) {
			ERR_NG << "Removed invalid 'current_player' attribute from [side] while loading a scenario. Consider using "
					  "'side_name' instead\n";

			side["current_player"] = config::attribute_value();
		}

		// Set some team specific values to their defaults specified in scenario
		for(const std::string& att_name : team_defaults) {
			const config::attribute_value* scenario_value = starting_pos_.get(att_name);
			config::attribute_value& team_value = side[att_name];

			if(scenario_value && team_value.empty()) {
				team_value = *scenario_value;
			}
		}
	}
}

void saved_game::expand_scenario()
{
	if(this->starting_pos_type_ == STARTINGPOS_NONE && !has_carryover_expanded_) {
		game_config_manager::get()->load_game_config_for_game(this->classification());

		const config& game_config = game_config_manager::get()->game_config();
		const config& scenario =
			game_config.find_child(classification().get_tagname(), "id", carryover_["next_scenario"]);

		if(scenario) {
			this->starting_pos_type_ = STARTINGPOS_SCENARIO;
			this->starting_pos_ = scenario;

			// A hash has to be generated using an unmodified scenario data.
			mp_settings_.hash = scenario.hash();

			// Add addon_id information if it exists.
			if(!scenario["addon_id"].empty() && scenario["require_scenario"].to_bool(false)) {
				config required_scenario;

				required_scenario["id"] = scenario["addon_id"];
				required_scenario["name"] = scenario["addon_title"];
				required_scenario["version"] = scenario["addon_version"];
				required_scenario["min_version"] = scenario["addon_min_version"];

				mp_settings_.update_addon_requirements(required_scenario);
			}

			update_label();
			set_defaults();
		} else {
			this->starting_pos_type_ = STARTINGPOS_INVALID;
			this->starting_pos_.clear();
		}
	}
}

namespace
{
bool variable_to_bool(const config& vars, const std::string& expression)
{
	std::string res = utils::interpolate_variables_into_string(expression, config_variable_set(vars));
	return res == "true" || res == "yes" || res == "1";
}

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

} // end anon namespace

void saved_game::load_mod(const std::string& type, const std::string& id)
{
	if(const config& cfg = game_config_manager::get()->game_config().find_child(type, "id", id)) {
		// Note the addon_id if this mod is required to play the game in mp.
		std::string require_attr = "require_" + type;

		// By default, eras have "require_era = true", and mods have "require_modification = false".
		bool require_default = (type == "era");

		if(!cfg["addon_id"].empty() && cfg[require_attr].to_bool(require_default)) {
			config required_mod;

			required_mod["id"] = cfg["addon_id"];
			required_mod["name"] = cfg["addon_title"];
			required_mod["version"] = cfg["addon_version"];
			required_mod["min_version"] = cfg["addon_min_version"];

			mp_settings_.update_addon_requirements(required_mod);
		}

		// Copy events
		for(const config& modevent : cfg.child_range("event")) {
			if(modevent["enable_if"].empty()
				|| variable_to_bool(carryover_.child_or_empty("variables"), modevent["enable_if"])
			) {
				this->starting_pos_.add_child("event", modevent);
			}
		}

		// Copy lua
		for(const config& modlua : cfg.child_range("lua")) {
			this->starting_pos_.add_child("lua", modlua);
		}

		// Copy load_resource
		for(const config& load_resource : cfg.child_range("load_resource")) {
			this->starting_pos_.add_child("load_resource", load_resource);
		}
	} else {
		// TODO: A user message instead?
		ERR_NG << "Couldn't find [" << type << "] with id=" << id << std::endl;
	}
}

// Gets the ids of the mp_era and modifications which were set to be active, then fetches these configs from the
// game_config and copies their [event] and [lua] to the starting_pos_.
// At this time, also collect the addon_id attributes which appeared in them and put this list in the addon_ids
// attribute of the mp_settings.
void saved_game::expand_mp_events()
{
	expand_scenario();

	if(this->starting_pos_type_ == STARTINGPOS_SCENARIO && !this->starting_pos_["has_mod_events"].to_bool(false)) {
		std::vector<modevents_entry> mods;
		std::set<std::string> loaded_resources;

		std::transform(mp_settings_.active_mods.begin(), mp_settings_.active_mods.end(), std::back_inserter(mods),
			[](const std::string& id) { return modevents_entry("modification", id); }
		);

		// We don't want the error message below if there is no era (= if this is a sp game).
		if(!mp_settings_.mp_era .empty()) {
			mods.emplace_back("era", mp_settings_.mp_era);
		}

		if(!classification_.campaign.empty()) {
			mods.emplace_back("campaign", classification_.campaign);
		}

		// In the first iteration mod contains no [resource]s in all other iterations, mods contains only [resource]s.
		do {
			for(modevents_entry& mod : mods) {
				load_mod(mod.type, mod.id);
			}

			mods.clear();

			for(const config& cfg : starting_pos_.child_range("load_resource")) {
				if(loaded_resources.find(cfg["id"].str()) == loaded_resources.end()) {
					mods.emplace_back("resource", cfg["id"].str());

					loaded_resources.insert(cfg["id"].str());
				}
			}

			starting_pos_.clear_children("load_resource");
		} while(!mods.empty());

		this->starting_pos_["has_mod_events"] = true;
	}
}

void saved_game::expand_mp_options()
{
	if(starting_pos_type_ == STARTINGPOS_SCENARIO && !has_carryover_expanded_) {
		std::vector<modevents_entry> mods;

		std::transform(mp_settings_.active_mods.begin(), mp_settings_.active_mods.end(), std::back_inserter(mods),
			[](const std::string& id) { return modevents_entry("modification", id); }
		);

		mods.emplace_back("era", mp_settings_.mp_era);
		mods.emplace_back("multiplayer", get_scenario_id());
		mods.emplace_back("campaign", classification().campaign);

		config& variables = carryover_.child_or_add("variables");

		for(modevents_entry& mod : mods) {
			if(const config& cfg = this->mp_settings().options.find_child(mod.type, "id", mod.id)) {
				// Parse old [option] tag range syntax.
				for(const config& option : cfg.child_range("option")) {
					try {
						variable_access_create(option["id"], variables).as_scalar() = option["value"];
					} catch(const invalid_variablename_exception&) {
						ERR_NG << "variable " << option["id"] << "cannot be set to " << option["value"] << std::endl;
					}
				}

				// Parse new [options] id = value syntax.
				for(const auto& option : cfg.child_or_empty("options").attribute_range()) {
					try {
						variable_access_create(option.first, variables).as_scalar() = option.second;
					} catch(const invalid_variablename_exception&) {
						ERR_NG << "variable " << option.first << "cannot be set to " << option.second << std::endl;
					}
				}
			} else {
				LOG_NG << "Couldn't find [" << mod.type << "] with id=" << mod.id << " for [option]s" << std::endl;
			}
		}
	}
}

void saved_game::expand_random_scenario()
{
	expand_scenario();

	if(this->starting_pos_type_ == STARTINGPOS_SCENARIO) {
		// If the entire scenario should be randomly generated
		if(!starting_pos_["scenario_generation"].empty()) {
			LOG_NG << "randomly generating scenario...\n";
			const cursor::setter cursor_setter(cursor::WAIT);

			config scenario_new =
				random_generate_scenario(starting_pos_["scenario_generation"], starting_pos_.child("generator"));

			// Preserve "story" form the scenario toplevel.
			for(config& story : starting_pos_.child_range("story")) {
				scenario_new.add_child("story", story);
			}

			scenario_new["id"] = starting_pos_["id"];
			starting_pos_ = scenario_new;

			update_label();
			set_defaults();
		}

		// If no map_data is provided, try to load the specified file directly
		if(starting_pos_["map_data"].empty() && !starting_pos_["map_file"].empty()) {
			starting_pos_["map_data"] = filesystem::read_map(starting_pos_["map_file"]);
		}

		// If the map should be randomly generated
		// We don’t want that we accidentally to this twice so we check for starting_pos_["map_data"].empty()
		if(starting_pos_["map_data"].empty() && !starting_pos_["map_generation"].empty()) {
			LOG_NG << "randomly generating map...\n";
			const cursor::setter cursor_setter(cursor::WAIT);

			starting_pos_["map_data"] =
				random_generate_map(starting_pos_["map_generation"], starting_pos_.child("generator"));
		}
	}
}

void saved_game::expand_carryover()
{
	expand_scenario();
	if(this->starting_pos_type_ == STARTINGPOS_SCENARIO && !has_carryover_expanded_) {
		carryover_info sides(carryover_);

		sides.transfer_to(get_starting_pos());
		for(config& side_cfg : get_starting_pos().child_range("side")) {
			sides.transfer_all_to(side_cfg);
		}

		carryover_ = sides.to_config();
		has_carryover_expanded_ = true;
	}
}

bool saved_game::valid() const
{
	return this->starting_pos_type_ != STARTINGPOS_INVALID;
}

config& saved_game::set_snapshot(config snapshot)
{
	this->starting_pos_type_ = STARTINGPOS_SNAPSHOT;
	this->starting_pos_.swap(snapshot);

	return this->starting_pos_;
}

void saved_game::set_scenario(config scenario)
{
	this->starting_pos_type_ = STARTINGPOS_SCENARIO;
	this->starting_pos_.swap(scenario);

	has_carryover_expanded_ = false;

	update_label();
}

void saved_game::remove_snapshot()
{
	this->starting_pos_type_ = STARTINGPOS_NONE;
	this->starting_pos_.clear();
}

config& saved_game::get_starting_pos()
{
	return starting_pos_;
}

const config& saved_game::get_replay_starting_pos()
{
	if(!replay_start_.empty()) {
		return replay_start_;
	}

	if(!has_carryover_expanded_) {
		// Try to load the scenario form game config or from [scenario] if there is no [replay_start]
		expand_scenario();
		expand_carryover();
	}

	if(starting_pos_type_ == STARTINGPOS_SCENARIO) {
		return starting_pos_;
	}

	return this->replay_start_.child("some_non_existet_invalid");
}

void saved_game::convert_to_start_save()
{
	assert(starting_pos_type_ == STARTINGPOS_SNAPSHOT);

	carryover_info sides(starting_pos_, true);

	sides.merge_old_carryover(carryover_info(carryover_));
	sides.rng().rotate_random();

	carryover_ = sides.to_config();

	has_carryover_expanded_ = false;

	replay_data_ = replay_recorder_base();
	replay_start_.clear();

	remove_snapshot();
}

config saved_game::to_config() const
{
	// TODO: remove this code duplication with write_... functions.
	config r = classification_.to_config();

	if(!this->replay_start_.empty()) {
		r.add_child("replay_start", replay_start_);
	}

	replay_data_.write(r.add_child("replay"));

	if(starting_pos_type_ == STARTINGPOS_SNAPSHOT) {
		r.add_child("snapshot", starting_pos_);
	} else if(starting_pos_type_ == STARTINGPOS_SCENARIO) {
		r.add_child("scenario", starting_pos_);
	}

	r.add_child(has_carryover_expanded_ ? "carryover_sides" : "carryover_sides_start", carryover_);
	r.add_child("multiplayer", mp_settings_.to_config());

	return r;
}

std::string saved_game::get_scenario_id()
{
	std::string scenario_id;

	if(this->starting_pos_type_ == STARTINGPOS_SNAPSHOT || this->starting_pos_type_ == STARTINGPOS_SCENARIO) {
		scenario_id = starting_pos_["id"].str();
	} else if(!has_carryover_expanded_) {
		scenario_id = carryover_["next_scenario"].str();
	} else {
		assert(!"cannot figure out scenario_id");
		throw "assertion ignored";
	}

	return scenario_id == "null" ? "" : scenario_id;
}

bool saved_game::not_corrupt() const
{
	return true;
}

void saved_game::update_label()
{
	if(classification().abbrev.empty()) {
		classification().label = starting_pos_["name"].str();
	} else {
		classification().label = classification().abbrev + "-" + starting_pos_["name"];
	}
}

void saved_game::cancel_orders()
{
	for(config& side : this->starting_pos_.child_range("side")) {
		// for humans "goto_x/y" is used for multi-turn-moves
		// for the ai "goto_x/y" is a way for wml to order the ai to move a unit to a certain place.
		// we want to cancel human order but not to break wml.
		if(side["controller"] != "human" && side["controller"] != "network") {
			continue;
		}

		for(config& unit : side.child_range("unit")) {
			unit["goto_x"] = -999;
			unit["goto_y"] = -999;
		}
	}
}

void saved_game::unify_controllers()
{
	for(config& side : this->starting_pos_.child_range("side")) {
		
		side.remove_attribute["is_local"];
		//TODO: the old code below is probably not needed anymore
		if(side["controller"] == "network") {
			side["controller"] = "human";
		}

		if(side["controller"] == "network_ai") {
			side["controller"] = "ai";
		}
	}
}

saved_game& saved_game::operator=(saved_game&& other)
{
	this->swap(other);
	return *this;
}

void saved_game::swap(saved_game& other)
{
	carryover_.swap(other.carryover_);

	std::swap(classification_, other.classification_);
	std::swap(has_carryover_expanded_, other.has_carryover_expanded_);
	std::swap(mp_settings_, other.mp_settings_);

	replay_data_.swap(other.replay_data_);
	replay_start_.swap(other.replay_start_);
	starting_pos_.swap(other.starting_pos_);

	std::swap(starting_pos_type_, other.starting_pos_type_);
}

void saved_game::set_data(config& cfg)
{
	log_scope("read_game");

	if(config& caryover_sides = cfg.child("carryover_sides")) {
		carryover_.swap(caryover_sides);
		has_carryover_expanded_ = true;
	} else if(config& caryover_sides_start = cfg.child("carryover_sides_start")) {
		carryover_.swap(caryover_sides_start);
		has_carryover_expanded_ = false;
	} else {
		carryover_.clear();
		has_carryover_expanded_ = false;
	}

	if(config& replay_start = cfg.child("replay_start")) {
		replay_start_.swap(replay_start);
	} else {
		replay_start_.clear();
	}

	replay_data_ = replay_recorder_base();

	// Serversided replays can contain multiple [replay]
	for(config& replay : cfg.child_range("replay")) {
		replay_data_.append_config(replay);
	}

	replay_data_.set_to_end();

	if(config& snapshot = cfg.child("snapshot")) {
		this->starting_pos_type_ = STARTINGPOS_SNAPSHOT;
		this->starting_pos_.swap(snapshot);
	} else if(config& scenario = cfg.child("scenario")) {
		this->starting_pos_type_ = STARTINGPOS_SCENARIO;
		this->starting_pos_.swap(scenario);
	} else {
		this->starting_pos_type_ = STARTINGPOS_NONE;
		this->starting_pos_.clear();
	}

	LOG_NG << "scenario: '" << carryover_["next_scenario"].str() << "'\n";

	if(const config& stats = cfg.child("statistics")) {
		statistics::fresh_stats();
		statistics::read_stats(stats);
	}

	classification_ = game_classification(cfg);
	mp_settings_ = mp_game_settings(cfg.child_or_empty("multiplayer"));

	cfg.clear();
}

void saved_game::clear()
{
	carryover_.clear();
	classification_ = game_classification();
	has_carryover_expanded_ = false;
	mp_settings_ = mp_game_settings();
	replay_data_.swap(replay_recorder_base());
	replay_start_.clear();
	starting_pos_.clear();
	starting_pos_type_ = STARTINGPOS_NONE;
}

void swap(saved_game& lhs, saved_game& rhs)
{
	lhs.swap(rhs);
}
