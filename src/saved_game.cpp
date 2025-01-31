/*
	Copyright (C) 2003 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
 *    This is present in all savefiles.
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
#include "side_controller.hpp"
#include "utils/general.hpp"
#include "team.hpp" // for team::attributes, team::variables
#include "variable.hpp" // for config_variable_set
#include "variable_info.hpp"

#include <cassert>
#include <iomanip>

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)

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

bool is_illegal_file_char(char c)
{
	return c == '/' || c == '\\' || c == ':' || (c >= 0x00 && c < 0x20)
#ifdef _WIN32
	|| c == '?' || c == '|' || c == '<' || c == '>' || c == '*' || c == '"'
#endif
	;
}

} // end anon namespace

saved_game::saved_game()
	: has_carryover_expanded_(false)
	, carryover_(carryover_info().to_config())
	, replay_start_()
	, classification_()
	, mp_settings_()
	, starting_point_type_(starting_point::NONE)
	, starting_point_()
	, replay_data_()
	, statistics_()
	, skip_story_(false)
{
}

saved_game::saved_game(config cfg)
	: has_carryover_expanded_(false)
	, carryover_()
	, replay_start_()
	, classification_(cfg)
	, mp_settings_()
	, starting_point_type_(starting_point::NONE)
	, starting_point_()
	, replay_data_()
	, statistics_()
	, skip_story_(false)
{
	set_data(cfg);
}

saved_game::saved_game(const saved_game& state)
	: has_carryover_expanded_(state.has_carryover_expanded_)
	, carryover_(state.carryover_)
	, replay_start_(state.replay_start_)
	, classification_(state.classification_)
	, mp_settings_(state.mp_settings_)
	, starting_point_type_(state.starting_point_type_)
	, starting_point_(state.starting_point_)
	, replay_data_(state.replay_data_)
	, statistics_(state.statistics_)
	, skip_story_(state.skip_story_)
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

	std::stringstream stream;
	stream << std::setfill('0') << std::setw(8) << std::hex << randomness::generator->get_random_int(0, std::numeric_limits<int>::max());
	carryover_["random_seed"] = stream.str();
	carryover_["random_calls"] = 0;
}

void saved_game::write_config(config_writer& out) const
{
	write_general_info(out);
	write_starting_point(out);

	if(!replay_start_.empty()) {
		out.write_child("replay_start", replay_start_);
	}

	out.open_child("replay");
	replay_data_.write(out);
	out.close_child("replay");
	write_carryover(out);
}

void saved_game::write_starting_point(config_writer& out) const
{
	if(starting_point_type_ == starting_point::SNAPSHOT) {
		out.write_child("snapshot", starting_point_);
	} else if(starting_point_type_ == starting_point::SCENARIO) {
		out.write_child("scenario", starting_point_);
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
	out.open_child("statistics");
	statistics().write(out);
	out.close_child("statistics");
}

void saved_game::set_defaults()
{
	const bool is_loaded_game = starting_point_type_ != starting_point::SCENARIO;
	const bool is_multiplayer_tag = classification().get_tagname() == "multiplayer";
	const game_config_view& game_config = game_config_manager::get()->game_config();

	static const std::vector<std::string> team_defaults {
		"carryover_percentage",
		"carryover_add",
	};

	if(auto campaign = game_config.find_child("campaign", "id", classification_.campaign)) {
		// FIXME: The mp code could use `require_scenario` to check whether we have the addon in question installed.
		//        But since [scenario]s are usually hidden behind `#ifdef CAMPAIGN_DEFINE` it would not be able to find them.
		//        Investigate how this should actually work.
		bool require_campaign = campaign["require_campaign"].to_bool(true);
		starting_point_["require_scenario"] = require_campaign;
	}

	for(config& side : starting_point_.child_range("side")) {
		// Set save_id default value directly after loading to its default to prevent different default behaviour in
		// mp_connect code and sp code.

		if(side["no_leader"].to_bool()) {
			side["leader_lock"] = true;
			side.remove_attribute("type");
		}

		if(side["save_id"].empty()) {
			side["save_id"] = side["id"];
		}
		if(side["save_id"].empty()) {
			side["save_id"] = side.child_or_empty("leader")["id"];
		}

		// If this side tag describes the leader of the side, convert it into a [leader] tag here, by doing this here,
		// all code that follows, no longer has to hande the possibility of leader information directly in [side].

		// If this side tag describes the leader of the side
		if(!side["type"].empty() && side["type"] != "null") {
			auto temp = config{};

			for(const std::string& tag : team::tags) {
				temp.append_children_by_move(side, tag);
			}
			for(const std::string& attr : team::attributes) {
				if(side.has_attribute(attr)) {
					temp[attr] = side[attr];
					side.remove_attribute(attr);
				}
			}
			temp["side"] = side["side"];
			temp.swap(side);
			temp.swap(side.add_child_at("leader", config(), 0));
		}

		if(!is_multiplayer_tag) {
			if(side["name"].blank()) {
				side["name"] = side.child_or_empty("leader")["name"];
			}
			if(side["side_name"].blank()) {
				side["side_name"] = side["name"];
			}
		}

		if(!is_loaded_game && !side["current_player"].empty()) {
			ERR_NG << "Removed invalid 'current_player' attribute from [side] while loading a scenario. Consider using "
					  "'side_name' instead";

			side["current_player"] = config::attribute_value();
		}

		// Set some team specific values to their defaults specified in scenario
		for(const std::string& att_name : team_defaults) {
			const config::attribute_value* scenario_value = starting_point_.get(att_name);
			config::attribute_value& team_value = side[att_name];

			if(scenario_value && team_value.empty()) {
				team_value = *scenario_value;
			}
		}
	}
}

void saved_game::expand_scenario()
{
	if(starting_point_type_ == starting_point::NONE && !has_carryover_expanded_) {
		game_config_manager::get()->load_game_config_for_game(classification(), carryover_["next_scenario"]);

		const game_config_view& game_config = game_config_manager::get()->game_config();
		auto scenario =
			game_config.find_child(classification().get_tagname(), "id", carryover_["next_scenario"]);

		if(scenario) {
			starting_point_type_ = starting_point::SCENARIO;
			starting_point_ = *scenario;

			// A hash has to be generated using an unmodified scenario data.
			mp_settings_.hash = scenario->hash();

			check_require_scenario();

			update_label();
			set_defaults();
		} else {
			ERR_NG << "Couldn't find [" << classification().get_tagname() << "] with id=" << carryover_["next_scenario"];
			starting_point_type_ = starting_point::INVALID;
			starting_point_.clear();
		}
	}
}

void saved_game::check_require_scenario()
{
	const std::string version_default = starting_point_["addon_id"].empty() ? game_config::wesnoth_version.str() : "";
	config scenario;
	scenario["id"] = starting_point_["addon_id"].str("mainline");
	scenario["name"] = starting_point_["addon_title"].str("mainline");
	scenario["version"] = starting_point_["addon_version"].str(version_default);
	scenario["min_version"] = starting_point_["addon_min_version"];
	scenario["required"] = starting_point_["require_scenario"].to_bool(false);
	config& content = scenario.add_child("content");
	content["id"] = starting_point_["id"];
	content["name"] = starting_point_["name"];
	// TODO: would it be better if this used the actual tagname ([multiplayer]/[scenario]) instead of always using [scenario]?
	content["type"] = "scenario";

	mp_settings_.update_addon_requirements(scenario);
}

// "non scenario" at the time of writing this meaning any era, campaign, mods, or resources (see expand_mp_events() below).
void saved_game::load_non_scenario(const std::string& type, const std::string& id, size_t pos)
{
	if(auto cfg = game_config_manager::get()->game_config().find_child(type, "id", id)) {
		// Note the addon_id if this mod is required to play the game in mp.
		std::string require_attr = "require_" + type;

		// anything with no addon_id is from mainline, and therefore isn't required in the sense that all players already have it
		const std::string version_default = cfg["addon_id"].empty() ? game_config::wesnoth_version.str() : "";
		config non_scenario;
		// if there's no addon_id, then this isn't an add-on
		non_scenario["id"] = cfg["addon_id"].str("mainline");
		non_scenario["name"] = cfg["addon_title"].str("mainline");
		non_scenario["version"] = cfg["addon_version"].str(version_default);
		non_scenario["min_version"] = cfg["addon_min_version"];
		non_scenario["required"] = cfg[require_attr].to_bool(!cfg["addon_id"].empty());
		config& content = non_scenario.add_child("content");
		content["id"] = id;
		content["name"] = cfg["addon_title"].str(cfg["name"].str(""));
		content["type"] = type;

		mp_settings_.update_addon_requirements(non_scenario);

		// Copy events
		for(const config& modevent : cfg->child_range("event")) {
			if(modevent["enable_if"].empty()
				|| variable_to_bool(carryover_.child_or_empty("variables"), modevent["enable_if"])
			) {
				starting_point_.add_child_at_total("event", modevent, pos++);
			}
		}

		// Copy lua
		for(const config& modlua : cfg->child_range("lua")) {
			starting_point_.add_child_at_total("lua", modlua, pos++);
		}

		// Copy modify_unit_type
		for(const config& modlua : cfg->child_range("modify_unit_type")) {
			starting_point_.add_child_at_total("modify_unit_type", modlua, pos++);
		}

		// Copy load_resource
		for(const config& load_resource : cfg->child_range("load_resource")) {
			starting_point_.add_child_at_total("load_resource", load_resource, pos++);
		}
	} else {
		// TODO: A user message instead?
		ERR_NG << "Couldn't find [" << type << "] with id=" << id;
	}
}

// Gets the ids of the mp_era and modifications which were set to be active, then fetches these configs from the
// game_config and copies their [event] and [lua] to the starting_point_.
// At this time, also collect the addon_id attributes which appeared in them and put this list in the addon_ids
// attribute of the mp_settings.
void saved_game::expand_mp_events()
{
	expand_scenario();

	if(starting_point_type_ == starting_point::SCENARIO && !starting_point_["has_mod_events"].to_bool(false)) {
		std::vector<modevents_entry> mods;
		std::set<std::string> loaded_resources;

		std::transform(classification_.active_mods.begin(), classification_.active_mods.end(), std::back_inserter(mods),
			[](const std::string& id) { return modevents_entry("modification", id); }
		);

		// We don't want the error message below if there is no era (= if this is a sp game).
		if(!classification_.era_id.empty()) {
			mods.emplace_back("era", classification_.era_id);
		}

		if(!classification_.campaign.empty()) {
			mods.emplace_back("campaign", classification_.campaign);
		}

		for(modevents_entry& mod : mods) {
			load_non_scenario(mod.type, mod.id, starting_point_.all_children_count());
		}
		mods.clear();

		while(starting_point_.has_child("load_resource")) {
			assert(starting_point_.child_count("load_resource") > 0);
			std::string id = starting_point_.mandatory_child("load_resource")["id"];
			size_t pos = starting_point_.find_total_first_of("load_resource");
			starting_point_.remove_child("load_resource", 0);
			if(loaded_resources.find(id) == loaded_resources.end()) {
				loaded_resources.insert(id);
				load_non_scenario("resource", id, pos);
			}
		}
		starting_point_["has_mod_events"] = true;
		starting_point_["loaded_resources"] = utils::join(loaded_resources);
	}
}

void saved_game::expand_mp_options()
{
	if(starting_point_type_ == starting_point::SCENARIO && !has_carryover_expanded_) {
		std::vector<modevents_entry> mods;

		std::transform(classification_.active_mods.begin(), classification_.active_mods.end(), std::back_inserter(mods),
			[](const std::string& id) { return modevents_entry("modification", id); }
		);

		mods.emplace_back("era", classification_.era_id);
		mods.emplace_back("multiplayer", get_scenario_id());
		mods.emplace_back("campaign", classification().campaign);

		config& variables = carryover_.child_or_add("variables");

		for(modevents_entry& mod : mods) {
			if(auto cfg = mp_settings().options.find_child(mod.type, "id", mod.id)) {
				for(const config& option : cfg->child_range("option")) {
					try {
						variable_access_create(option["id"], variables).as_scalar() = option["value"];
					} catch(const invalid_variablename_exception&) {
						ERR_NG << "variable " << option["id"] << "cannot be set to " << option["value"];
					}
				}
			} else {
				LOG_NG << "Couldn't find [" << mod.type << "] with id=" << mod.id << " for [option]s";
			}
		}
	}
}

static void inherit_scenario(config& scenario, config& map_scen)
{
	config& map_scenario = map_scen.has_child("multiplayer") ? map_scen.mandatory_child("multiplayer") : (map_scen.has_child("scenario") ? map_scen.mandatory_child("scenario") : map_scen);
	config sides;
	sides.splice_children(map_scenario, "side");
	scenario.append_children(map_scenario);
	scenario.inherit_attributes(map_scenario);
	for(config& side_from : sides.child_range("side")) {
		auto side_to = scenario.find_child("side", "side", side_from["side"]);
		if(side_to) {
			side_to->inherit_attributes(side_from);
			side_to->append_children(side_from);
		} else {
			scenario.add_child("side", side_from);
		}
	}
}

void saved_game::expand_map_file(config& scenario)
{
	if(!scenario["include_file"].empty()) {
		std::string include_data = filesystem::read_scenario(scenario["include_file"]);
		if(!include_data.empty()) {
			config include_data_cfg;
			read(include_data_cfg, include_data);
			inherit_scenario(scenario, include_data_cfg);
		}
		// this method gets called two additional times, so without this you end up calling inherit_scenario() three times total
		// this is equivalent to the below check for map_data being empty
		scenario["include_file"] = "";
	}

	if(scenario["map_data"].empty() && !scenario["map_file"].empty()) {
		std::string map_data = filesystem::read_map(scenario["map_file"]);
		if(map_data.find("map_data") != std::string::npos) {
			// we have a scenario, generated by the editor
			deprecated_message("map_file cfg", DEP_LEVEL::FOR_REMOVAL, "1.19", "Providing a .cfg file to the map_file attribute is deprecated. Use map_file for .map files and include_file for .cfg files.");
			config map_data_cfg;
			read(map_data_cfg, map_data);
			inherit_scenario(scenario, map_data_cfg);
		} else {
			// we have an plain map_data file
			scenario["map_data"] = map_data;
		}
	}
}

void saved_game::expand_random_scenario()
{
	expand_scenario();

	if(starting_point_type_ == starting_point::SCENARIO) {
		// If the entire scenario should be randomly generated
		if(!starting_point_["scenario_generation"].empty()) {
			LOG_NG << "randomly generating scenario...";
			const cursor::setter cursor_setter(cursor::WAIT);

			config scenario_new =
				random_generate_scenario(starting_point_["scenario_generation"], starting_point_.mandatory_child("generator"), &carryover_.child_or_empty("variables"));

			post_scenario_generation(starting_point_, scenario_new);
			starting_point_ = std::move(scenario_new);

			update_label();
			set_defaults();
		}

		// If no map_data is provided, try to load the specified file directly
		expand_map_file(starting_point_);
		// If the map should be randomly generated
		// We don’t want that we accidentally to this twice so we check for starting_point_["map_data"].empty()
		if(starting_point_["map_data"].empty() && !starting_point_["map_generation"].empty()) {
			LOG_NG << "randomly generating map...";
			const cursor::setter cursor_setter(cursor::WAIT);

			starting_point_["map_data"] =
				random_generate_map(starting_point_["map_generation"], starting_point_.mandatory_child("generator"), &carryover_.child_or_empty("variables"));
		}
	}
}

void saved_game::post_scenario_generation(const config& old_scenario, config& generated_scenario)
{
	static const std::vector<std::string> attributes_to_copy {
		"id",
		"addon_id",
		"addon_title",
		"addon_version",
		"addon_min_version",
		"require_scenario",
	};

	// TODO: should we add "description" to this list?
	// TODO: in theory it is possible that whether the scenario is required depends on the generated scenario, so maybe remove require_scenario from this list.

	for(const auto& str : attributes_to_copy) {
		generated_scenario[str] = old_scenario[str];
	}

	// Preserve "story" from the scenario toplevel.
	// Note that it does not delete [story] tags in generated_scenario, so you can still have your story
	// dependent on the generated scenario.
	for(const config& story : old_scenario.child_range("story")) {
		generated_scenario.add_child("story", story);
	}
}


void saved_game::expand_carryover()
{
	expand_scenario();
	if(starting_point_type_ == starting_point::SCENARIO && !has_carryover_expanded_) {
		carryover_info sides(carryover_);

		sides.transfer_to(get_starting_point());
		for(config& side_cfg : get_starting_point().child_range("side")) {
			sides.transfer_all_to(side_cfg);
		}

		carryover_ = sides.to_config();
		statistics().new_scenario(get_starting_point()["name"]);
		has_carryover_expanded_ = true;
	}
}

bool saved_game::valid() const
{
	return starting_point_type_ != starting_point::INVALID;
}

config& saved_game::set_snapshot(config snapshot)
{
	starting_point_type_ = starting_point::SNAPSHOT;
	starting_point_.swap(snapshot);

	return starting_point_;
}

void saved_game::set_scenario(config scenario)
{
	starting_point_type_ = starting_point::SCENARIO;
	starting_point_.swap(scenario);

	has_carryover_expanded_ = false;

	update_label();
}

void saved_game::remove_snapshot()
{
	starting_point_type_ = starting_point::NONE;
	starting_point_.clear();
}

config& saved_game::get_starting_point()
{
	return starting_point_;
}

const config& saved_game::get_replay_starting_point()
{
	if(!replay_start_.empty()) {
		return replay_start_;
	}

	if(!has_carryover_expanded_) {
		// Try to load the scenario form game config or from [scenario] if there is no [replay_start]
		expand_scenario();
		expand_carryover();
	}

	if(starting_point_type_ == starting_point::SCENARIO) {
		return starting_point_;
	}

	throw config::error("No replay_start found");
}

void saved_game::convert_to_start_save()
{
	assert(starting_point_type_ == starting_point::SNAPSHOT);

	carryover_info sides(starting_point_, true);

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

	if(!replay_start_.empty()) {
		r.add_child("replay_start", replay_start_);
	}

	replay_data_.write(r.add_child("replay"));

	if(starting_point_type_ == starting_point::SNAPSHOT) {
		r.add_child("snapshot", starting_point_);
	} else if(starting_point_type_ == starting_point::SCENARIO) {
		r.add_child("scenario", starting_point_);
	}

	r.add_child(has_carryover_expanded_ ? "carryover_sides" : "carryover_sides_start", carryover_);
	r.add_child("multiplayer", mp_settings_.to_config());
	r.add_child("statistics", statistics_.to_config());

	return r;
}

std::string saved_game::get_scenario_id() const
{
	std::string scenario_id;

	if(starting_point_type_ == starting_point::SNAPSHOT || starting_point_type_ == starting_point::SCENARIO) {
		scenario_id = starting_point_["id"].str();
	} else if(!has_carryover_expanded_) {
		scenario_id = carryover_["next_scenario"].str();
	} else if(!replay_start_.empty()) {
		scenario_id = replay_start_["id"].str();
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
	std::string& label = classification().label;

	if(classification().abbrev.empty()) {
		label = starting_point_["name"].str();
	} else {
		label = classification().abbrev + "-" + starting_point_["name"];
	}

	utils::erase_if(label, is_illegal_file_char);
	std::replace(label.begin(), label.end(), '_', ' ');
}

void saved_game::cancel_orders()
{
	for(config& side : starting_point_.child_range("side")) {
		// for humans "goto_x/y" is used for multi-turn-moves
		// for the ai "goto_x/y" is a way for wml to order the ai to move a unit to a certain place.
		// we want to cancel human order but not to break wml.
		if(side["controller"] != side_controller::human) {
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
	for(config& side : starting_point_.child_range("side")) {
		side.remove_attribute("is_local");
	}
}

saved_game& saved_game::operator=(saved_game&& other)
{
	swap(other);
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
	starting_point_.swap(other.starting_point_);

	std::swap(starting_point_type_, other.starting_point_type_);
}

void saved_game::set_data(config& cfg)
{
	log_scope("read_game");

	if(auto caryover_sides = cfg.optional_child("carryover_sides")) {
		carryover_.swap(*caryover_sides);
		has_carryover_expanded_ = true;
	} else if(auto caryover_sides_start = cfg.optional_child("carryover_sides_start")) {
		carryover_.swap(*caryover_sides_start);
		has_carryover_expanded_ = false;
	} else {
		carryover_.clear();
		has_carryover_expanded_ = false;
	}

	if(auto replay_start = cfg.optional_child("replay_start")) {
		replay_start_.swap(*replay_start);
	} else {
		replay_start_.clear();
	}

	replay_data_ = replay_recorder_base();

	// Serversided replays can contain multiple [replay]
	for(config& replay : cfg.child_range("replay")) {
		replay_data_.append_config(replay);
	}

	replay_data_.set_to_end();

	if(auto snapshot = cfg.optional_child("snapshot")) {
		starting_point_type_ = starting_point::SNAPSHOT;
		starting_point_.swap(*snapshot);
	} else if(auto scenario = cfg.optional_child("scenario")) {
		starting_point_type_ = starting_point::SCENARIO;
		starting_point_.swap(*scenario);
	} else {
		starting_point_type_ = starting_point::NONE;
		starting_point_.clear();
	}

	LOG_NG << "scenario: '" << carryover_["next_scenario"].str() << "'";

	if(auto stats = cfg.optional_child("statistics")) {
		statistics_.read(*stats);
	}

	classification_ = game_classification{ cfg };
	mp_settings_ = { cfg.child_or_empty("multiplayer") };

	cfg.clear();
}

void saved_game::clear()
{
	carryover_.clear();
	classification_ = {};
	has_carryover_expanded_ = false;
	mp_settings_ = {};
	replay_data_.swap({});
	replay_start_.clear();
	starting_point_.clear();
	starting_point_type_ = starting_point::NONE;
	statistics_ = statistics_record::campaign_stats_t();
}

void swap(saved_game& lhs, saved_game& rhs)
{
	lhs.swap(rhs);
}
