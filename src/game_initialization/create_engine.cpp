/*
   Copyright (C) 2013 - 2018 by Andrius Silinskas <silinskas.andrius@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "game_initialization/create_engine.hpp"

#include "filesystem.hpp"
#include "game_config_manager.hpp"
#include "preferences/credentials.hpp"
#include "preferences/game.hpp"
#include "generators/map_create.hpp"
#include "gui/dialogs/campaign_difficulty.hpp"
#include "hash.hpp"
#include "image.hpp"
#include "log.hpp"
#include "map/exception.hpp"
#include "map/map.hpp"
#include "minimap.hpp"
#include "saved_game.hpp"
#include "wml_exception.hpp"

#include "serialization/preprocessor.hpp"
#include "serialization/parser.hpp"
#include "serialization/string_utils.hpp"

#include <sstream>
#include <cctype>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)

static lg::log_domain log_mp_create_engine("mp/create/engine");
#define WRN_MP LOG_STREAM(warn, log_mp_create_engine)
#define DBG_MP LOG_STREAM(debug, log_mp_create_engine)

namespace ng {

level::level(const config& data)
	: data_(data)
{
}

scenario::scenario(const config& data)
	: level(data)
	, map_()
	, map_hash_()
	, num_players_(0)
{
	set_metadata();
}

bool scenario::can_launch_game() const
{
	return map_.get() != nullptr;
}

void scenario::set_metadata()
{
	const std::string& map_data = data_["map_data"];

	try {
		map_.reset(new gamemap(game_config_manager::get()->terrain_types(),
			map_data));
	} catch(incorrect_map_format_error& e) {
		data_["description"] = _("Map could not be loaded: ") + e.message;

		ERR_CF << "map could not be loaded: " << e.message << '\n';
	} catch(wml_exception& e) {
		data_["description"] = _("Map could not be loaded.");

		ERR_CF << "map could not be loaded: " << e.dev_message << '\n';
	}

	set_sides();
}

std::string scenario::map_size() const
{
	std::stringstream map_size;

	if(map_.get() != nullptr) {
		map_size << map_->w();
		map_size << font::unicode_multiplication_sign;
		map_size << map_->h();
	} else {
		map_size << _("not available.");
	}

	return map_size.str();
}

void scenario::set_sides()
{
	if(map_.get() != nullptr) {
		// If there are fewer sides in the configuration than there are
		// starting positions, then generate the additional sides
		const int map_positions = map_->num_valid_starting_positions();

		if(data_.child_count("side") == 0) {
			for(int pos = 0; pos < map_positions; ++pos) {
				config& side = data_.add_child("side");
				side["side"] = pos + 1;
				side["team_name"] = "Team " + std::to_string(pos + 1);
				side["canrecruit"] = true;
				side["controller"] = "human";
			}
		}

		num_players_ = 0;
		for(const config& scenario : data_.child_range("side")) {
			if(scenario["allow_player"].to_bool(true)) {
				++num_players_;
			}
		}
	}
}

user_map::user_map(const config& data, const std::string& name, gamemap* map)
	: scenario(data)
	, name_(name)
{
	if(map != nullptr) {
		map_.reset(new gamemap(*map));
	}

	set_metadata();
}

void user_map::set_metadata()
{
	set_sides();
}

std::string user_map::description() const
{
	if(!data_["description"].empty()) {
		return data_["description"];
	}

	// map error message
	return _("Custom map.");
}

random_map::random_map(const config& data)
	: scenario(data)
	, generator_data_()
	, generate_whole_scenario_(data_.has_attribute("scenario_generation"))
	, generator_name_(generate_whole_scenario_ ? data_["scenario_generation"] : data_["map_generation"])
{
	if(!data.has_child("generator")) {
		data_.clear();
		generator_data_.clear();
		data_["description"] = "Error: Random map found with missing generator information. Scenario should have a [generator] child.";
		data_["error_message"] = "missing [generator] tag";
	} else {
		generator_data_ = data.child("generator");
	}

	if(!data.has_attribute("scenario_generation") && !data.has_attribute("map_generation")) {
		data_.clear();
		generator_data_.clear();
		data_["description"] = "Error: Random map found with missing generator information. Scenario should have a [generator] child.";
		data_["error_message"] = "couldn't find 'scenario_generation' or 'map_generation' attribute";
	}
}

map_generator* random_map::create_map_generator() const
{
	return ::create_map_generator(generator_name(), generator_data());
}

campaign::campaign(const config& data)
	: level(data)
	, id_(data["id"])
	, allow_era_choice_(level::allow_era_choice())
	, image_label_()
	, min_players_(2)
	, max_players_(2)
{
	if(data.has_attribute("start_year")) {
		dates_.first = irdya_date::read_date(data["start_year"]);
		if(data.has_attribute("end_year")) {
			dates_.second = irdya_date::read_date(data["end_year"]);
		} else {
			dates_.second = dates_.first;
		}
	} else if(data.has_attribute("year")) {
		dates_.first = dates_.second = irdya_date::read_date(data["year"]);
	}
	set_metadata();
}

bool campaign::can_launch_game() const
{
	return !data_.empty();
}

void campaign::set_metadata()
{
	image_label_ = data_["image"].str();

	int min = data_["min_players"].to_int(2);
	int max = data_["max_players"].to_int(2);

	min_players_ = max_players_ =  min;

	if(max > min) {
		max_players_ = max;
	}
}

void campaign::mark_if_completed()
{
	data_["completed"] = preferences::is_campaign_completed(data_["id"]);

	for(auto& cfg : data_.child_range("difficulty")) {
		cfg["completed_at"] = preferences::is_campaign_completed(data_["id"], cfg["define"]);
	}
}

create_engine::create_engine(saved_game& state)
	: current_level_type_()
	, current_level_index_(0)
	, current_era_index_(0)
	, level_name_filter_()
	, player_count_filter_(1)
	, type_map_()
	, user_map_names_()
	, user_scenario_names_()
	, eras_()
	, mods_()
	, state_(state)
	, dependency_manager_(nullptr)
	, generator_(nullptr)
	, selected_campaign_difficulty_()
	, game_config_(game_config_manager::get()->game_config())
{
	// Set up the type map. Do this first!
	type_map_.emplace(level::TYPE::SCENARIO, type_list());
	type_map_.emplace(level::TYPE::USER_MAP, type_list());
	type_map_.emplace(level::TYPE::USER_SCENARIO, type_list());
	type_map_.emplace(level::TYPE::CAMPAIGN, type_list());
	type_map_.emplace(level::TYPE::SP_CAMPAIGN, type_list());
	type_map_.emplace(level::TYPE::RANDOM_MAP, type_list());

	DBG_MP << "restoring game config\n";

	// Restore game config for multiplayer.
	game_classification::CAMPAIGN_TYPE type = state_.classification().campaign_type;

	state_.clear();
	state_.classification().campaign_type = type;

	game_config_manager::get()->load_game_config_for_create(type == game_classification::CAMPAIGN_TYPE::MULTIPLAYER);

	// Initialize dependency_manager_ after refreshing game config.
	dependency_manager_.reset(new depcheck::manager(
		game_config_, type == game_classification::CAMPAIGN_TYPE::MULTIPLAYER));

	// TODO: the editor dir is already configurable, is the preferences value
	filesystem::get_files_in_dir(filesystem::get_user_data_dir() + "/editor/maps", &user_map_names_,
		nullptr, filesystem::FILE_NAME_ONLY);

	filesystem::get_files_in_dir(filesystem::get_user_data_dir() + "/editor/scenarios", &user_scenario_names_,
		nullptr, filesystem::FILE_NAME_ONLY);

	DBG_MP << "initializing all levels, eras and mods\n";

	init_all_levels();
	init_extras(ERA);
	init_extras(MOD);

	state_.mp_settings().saved_game = false;

	for(const std::string& str : preferences::modifications(state_.classification().campaign_type == game_classification::CAMPAIGN_TYPE::MULTIPLAYER)) {
		if(game_config_.find_child("modification", "id", str)) {
			state_.mp_settings().active_mods.push_back(str);
		}
	}

	if(state_.classification().campaign_type == game_classification::CAMPAIGN_TYPE::MULTIPLAYER) {
		dependency_manager_->try_modifications(state_.mp_settings().active_mods, true);
	}

	reset_level_filters();
}

void create_engine::init_generated_level_data()
{
	DBG_MP << "initializing generated level data\n";

	//DBG_MP << "current data:\n";
	//DBG_MP << current_level().data().debug();

	random_map * cur_lev = dynamic_cast<random_map *> (&current_level());

	if(!cur_lev) {
		WRN_MP << "Tried to initialized generated level data on a level that wasn't a random map\n";
		return;
	}

	try {
		if(!cur_lev->generate_whole_scenario())
		{
			DBG_MP << "** replacing map ** \n";

			config data = cur_lev->data();

			data["map_data"] = generator_->create_map();

			cur_lev->set_data(data);

		} else { //scenario generation

			DBG_MP << "** replacing scenario ** \n";

			config data = generator_->create_scenario();

			// Set the scenario to have placing of sides
			// based on the terrain they prefer
			if(!data.has_attribute("modify_placing")) {
				data["modify_placing"] = true;
			}

			const std::string& description = cur_lev->data()["description"];
			data["description"] = description;
			// TODO: should we also carryover [story] from the outer scenario as we do in saved_game.cpp
			data["id"] = cur_lev->data()["id"];

			cur_lev->set_data(data);
		}
	} catch (mapgen_exception & e) {
		config data = cur_lev->data();

		data["error_message"] = e.what();

		cur_lev->set_data(data);
	}

	//DBG_MP << "final data:\n";
	//DBG_MP << current_level().data().debug();
}

bool create_engine::current_level_has_side_data()
{
	//
	// We exclude campaigns from this check since they require preprocessing in order to check
	// their side data. Since this function is used by the MP Create screen to verify side data
	// before proceeding to Staging, this should cover most cases of false positives. It does,
	// however, leave open the possibility of scenarios that require preprocessing before their
	// side data is accessible, but that's an unlikely occurrence.
	//
	if(is_campaign()) {
		return true;
	}

	return current_level().data().has_child("side");
}

void create_engine::prepare_for_new_level()
{
	DBG_MP << "preparing mp_game_settings for new level\n";
	state_.expand_scenario();
	state_.expand_random_scenario();
}

void create_engine::prepare_for_era_and_mods()
{
	state_.classification().era_define = game_config_.find_child("era", "id", get_parameters().mp_era)["define"].str();
	for(const std::string& mod_id : get_parameters().active_mods) {
		state_.classification().mod_defines.push_back(game_config_.find_child("modification", "id", mod_id)["define"].str());
	}
}

void create_engine::prepare_for_scenario()
{
	DBG_MP << "preparing data for scenario by reloading game config\n";

	state_.classification().scenario_define = current_level().data()["define"].str();

	state_.set_carryover_sides_start(
		config {"next_scenario", current_level().data()["id"]}
	);
}

void create_engine::prepare_for_campaign(const std::string& difficulty)
{
	DBG_MP << "preparing data for campaign by reloading game config\n";

	if(!difficulty.empty()) {
		state_.classification().difficulty = difficulty;
	} else if(!selected_campaign_difficulty_.empty()) {
		state_.classification().difficulty = selected_campaign_difficulty_;
	}

	config& current_level_data = current_level().data();

	state_.classification().campaign = current_level_data["id"].str();
	state_.classification().campaign_name = current_level_data["name"].str();
	state_.classification().abbrev = current_level_data["abbrev"].str();

	state_.classification().end_text = current_level_data["end_text"].str();
	state_.classification().end_text_duration = current_level_data["end_text_duration"];

	state_.classification().campaign_define = current_level_data["define"].str();
	state_.classification().campaign_xtra_defines =
		utils::split(current_level_data["extra_defines"]);

	state_.set_carryover_sides_start(
		config {"next_scenario", current_level_data["first_scenario"]}
	);
}

std::string create_engine::select_campaign_difficulty(int set_value)
{
	// Verify the existence of difficulties
	std::vector<std::string> difficulties;

	for(const config& d : current_level().data().child_range("difficulty")) {
		difficulties.push_back(d["define"]);
	}

	if(difficulties.empty()) {
		difficulties = utils::split(current_level().data()["difficulties"]);
	}

	// No difficulties found. Exit
	if(difficulties.empty()) {
		return "";
	}

	// One difficulty found. Use it
	if(difficulties.size() == 1) {
		return difficulties[0];
	}

	// A specific difficulty value was passed
	// Use a minimalistic interface to get the specified define
	if(set_value != -1) {
		if(set_value > static_cast<int>(difficulties.size())) {
			std::cerr << "incorrect difficulty number: [" <<
				set_value << "]. maximum is [" << difficulties.size() << "].\n";
			return "FAIL";
		} else if(set_value < 1) {
			std::cerr << "incorrect difficulty number: [" <<
				set_value << "]. minimum is [1].\n";
			return "FAIL";
		} else {
			return difficulties[set_value - 1];
		}
	}

	// If not, let the user pick one from the prompt
	// We don't pass the difficulties vector here because additional data is required
	// to constrict the dialog
	gui2::dialogs::campaign_difficulty dlg(current_level().data());
	dlg.show();

	selected_campaign_difficulty_ = dlg.selected_difficulty();

	return selected_campaign_difficulty_;
}

void create_engine::prepare_for_saved_game()
{
	DBG_MP << "preparing mp_game_settings for saved game\n";

	game_config_manager::get()->load_game_config_for_game(state_.classification());

	// The save might be a start-of-scenario save so make sure we have the scenario data loaded.
	state_.expand_scenario();
	state_.mp_settings().saved_game = true;
}

void create_engine::prepare_for_other()
{
	DBG_MP << "prepare_for_other\n";
	state_.set_scenario(current_level().data());
	state_.mp_settings().hash = current_level().data().hash();
}

void create_engine::apply_level_filter(const std::string& name)
{
	level_name_filter_ = name;
	apply_level_filters();
}

void create_engine::apply_level_filter(int players)
{
	player_count_filter_ = players;
	apply_level_filters();
}

void create_engine::reset_level_filters()
{
	for(auto& type : type_map_) {
		type.second.reset_filter();
	}

	level_name_filter_ = "";
}

level& create_engine::current_level() const
{
	return *type_map_.at(current_level_type_.v).games[current_level_index_];
}

const create_engine::extras_metadata& create_engine::current_era() const
{
	return *get_const_extras_by_type(ERA)[current_era_index_];
}

void create_engine::set_current_level(const size_t index)
{
	try {
		current_level_index_ = type_map_.at(current_level_type_.v).games_filtered.at(index);
	} catch (std::out_of_range&) {
		current_level_index_ = 0u;
	}

	if(current_level_type_ == level::TYPE::RANDOM_MAP) {
		random_map* current_random_map = dynamic_cast<random_map*>(&current_level());

		// If dynamic cast has failed then we somehow have gotten all the pointers mixed together.
		assert(current_random_map);

		generator_.reset(current_random_map->create_map_generator());
	} else {
		generator_.reset(nullptr);
	}

	if(state_.classification().campaign_type == game_classification::CAMPAIGN_TYPE::MULTIPLAYER) {
		dependency_manager_->try_scenario(current_level().id());
	}
}

void create_engine::set_current_era_index(const size_t index, bool force)
{
	current_era_index_ = index;

	dependency_manager_->try_era_by_index(index, force);
}

bool create_engine::toggle_mod(int index, bool force)
{
	force |= state_.classification().campaign_type != game_classification::CAMPAIGN_TYPE::MULTIPLAYER;

	bool is_active = dependency_manager_->is_modification_active(index);
	dependency_manager_->try_modification_by_index(index, !is_active, force);

	state_.mp_settings().active_mods = dependency_manager_->get_modifications();

	return !is_active;
}

bool create_engine::generator_assigned() const
{
	return generator_ != nullptr;
}

bool create_engine::generator_has_settings() const
{
	return generator_->allow_user_config();
}

void create_engine::generator_user_config()
{
	generator_->user_config();
}

std::pair<level::TYPE, int> create_engine::find_level_by_id(const std::string& id) const
{
	for(const auto& type : type_map_) {
		int i = 0;

		for(const auto game : type.second.games) {
			if(game->id() == id) {
				return {type.first, i};
			}

			i++;
		}
	}

	return {level::TYPE::SP_CAMPAIGN, -1};
}

int create_engine::find_extra_by_id(const MP_EXTRA extra_type, const std::string& id) const
{
	int i = 0;
	for(extras_metadata_ptr extra : get_const_extras_by_type(extra_type)) {
		if(extra->id == id) {
			return i;
		}
		i++;
	}

	return -1;
}

void create_engine::init_active_mods()
{
	state_.mp_settings().active_mods = dependency_manager_->get_modifications();
}

std::vector<std::string>& create_engine::active_mods()
{
	return state_.mp_settings().active_mods;
}

std::vector<create_engine::extras_metadata_ptr> create_engine::active_mods_data()
{
	const std::vector<extras_metadata_ptr>& mods = get_const_extras_by_type(MP_EXTRA::MOD);

	std::vector<extras_metadata_ptr> data_vec;
	std::copy_if(mods.begin(), mods.end(), std::back_inserter(data_vec), [this](extras_metadata_ptr mod) {
		return dependency_manager_->is_modification_active(mod->id);
	});

	return data_vec;
}

const config& create_engine::curent_era_cfg() const
{
	int era_index = current_level().allow_era_choice() ? current_era_index_ : 0;
	return *eras_[era_index]->cfg;
}

const mp_game_settings& create_engine::get_parameters()
{
	DBG_MP << "getting parameter values" << std::endl;

	int era_index = current_level().allow_era_choice() ? current_era_index_ : 0;
	state_.mp_settings().mp_era = eras_[era_index]->id;
	state_.mp_settings().mp_era_name = eras_[era_index]->name;

	return state_.mp_settings();
}

void create_engine::init_all_levels()
{
	if(const config& generic_multiplayer = game_config_.child("generic_multiplayer")) {
		config gen_mp_data = generic_multiplayer;

		// User maps.
		int dep_index_offset = 0;
		for(size_t i = 0; i < user_map_names_.size(); i++)
		{
			config user_map_data = gen_mp_data;
			user_map_data["map_data"] = filesystem::read_map(user_map_names_[i]);

			// Check if a file is actually a map.
			// Note that invalid maps should be displayed in order to
			// show error messages in the GUI.
			bool add_map = true;
			std::unique_ptr<gamemap> map;
			try {
				map.reset(new gamemap(game_config_manager::get()->terrain_types(), user_map_data["map_data"]));
			} catch (incorrect_map_format_error& e) {
				user_map_data["description"] = _("Map could not be loaded: ") + e.message;

				ERR_CF << "map could not be loaded: " << e.message << '\n';
			} catch (wml_exception&) {
				add_map = false;
				dep_index_offset++;
			}

			if(add_map) {
				type_map_[level::TYPE::USER_MAP].games.emplace_back(new user_map(user_map_data, user_map_names_[i], map.get()));

				// Since user maps are treated as scenarios, some dependency info is required
				config depinfo;
				depinfo["id"] = user_map_names_[i];
				depinfo["name"] = user_map_names_[i];
				dependency_manager_->insert_element(depcheck::SCENARIO, depinfo, i - dep_index_offset);
			}
		}

		// User made scenarios.
		dep_index_offset = 0;
		for(size_t i = 0; i < user_scenario_names_.size(); i++)
		{
			config data;
			try {
				read(data, *preprocess_file(filesystem::get_user_data_dir() + "/editor/scenarios/" + user_scenario_names_[i]));
			} catch (config::error & e) {
				ERR_CF << "Caught a config error while parsing user made (editor) scenarios:\n" << e.message << std::endl;
				ERR_CF << "Skipping file: " << (filesystem::get_user_data_dir() + "/editor/scenarios/" + user_scenario_names_[i]) << std::endl;
				continue;
			}

			scenario_ptr new_scenario(new scenario(data));
			if(new_scenario->id().empty()) continue;

			type_map_[level::TYPE::USER_SCENARIO].games.push_back(std::move(new_scenario));

			// Since user scenarios are treated as scenarios, some dependency info is required
			config depinfo;
			depinfo["id"] = data["id"];
			depinfo["name"] = data["name"];
			dependency_manager_->insert_element(depcheck::SCENARIO, depinfo, i - dep_index_offset++);
		}
	}

	// Stand-alone scenarios.
	for(const config& data : game_config_.child_range("multiplayer"))
	{
		if(!data["allow_new_game"].to_bool(true))
			continue;

		if(!data["campaign_id"].empty())
			continue;

		if(data.has_attribute("map_generation") || data.has_attribute("scenario_generation")) {
			type_map_[level::TYPE::RANDOM_MAP].games.emplace_back(new random_map(data));
		} else {
			type_map_[level::TYPE::SCENARIO].games.emplace_back(new scenario(data));
		}
	}

	// Campaigns.
	for(const config& data : game_config_.child_range("campaign"))
	{
		const std::string& type = data["type"];
		bool mp = state_.classification().campaign_type == game_classification::CAMPAIGN_TYPE::MULTIPLAYER;

		if(type == "mp" || (type == "hybrid" && mp)) {
			type_map_[level::TYPE::CAMPAIGN].games.emplace_back(new campaign(data));
		}

		if(type == "sp" || type.empty() || (type == "hybrid" && !mp)) {
			campaign_ptr new_sp_campaign(new campaign(data));
			new_sp_campaign->mark_if_completed();

			type_map_[level::TYPE::SP_CAMPAIGN].games.push_back(std::move(new_sp_campaign));
		}
	}

	auto& sp_campaigns = type_map_[level::TYPE::SP_CAMPAIGN].games;

	// Sort sp campaigns by rank.
	std::stable_sort(sp_campaigns.begin(), sp_campaigns.end(),
        [](const create_engine::level_ptr& a, const create_engine::level_ptr& b) {
			return a->data()["rank"].to_int(1000) < b->data()["rank"].to_int(1000);
		}
    );
}

void create_engine::init_extras(const MP_EXTRA extra_type)
{
	std::vector<extras_metadata_ptr>& extras = get_extras_by_type(extra_type);
	const std::string extra_name = (extra_type == ERA) ? "era" : "modification";

	ng::depcheck::component_availability default_availabilty = (extra_type == ERA)
		? ng::depcheck::component_availability::MP
		: ng::depcheck::component_availability::HYBRID;

	std::set<std::string> found_ids;
	for(const config& extra : game_config_.child_range(extra_name))
	{
		ng::depcheck::component_availability type = extra["type"].to_enum(default_availabilty);
		bool mp = state_.classification().campaign_type == game_classification::CAMPAIGN_TYPE::MULTIPLAYER;

		if((type != ng::depcheck::component_availability::MP || mp) && (type != ng::depcheck::component_availability::SP || !mp) )
		{
			if(found_ids.insert(extra["id"]).second) {
				extras_metadata_ptr new_extras_metadata(new extras_metadata());
				new_extras_metadata->id = extra["id"].str();
				new_extras_metadata->name = extra["name"].str();
				new_extras_metadata->description = extra["description"].str();
				new_extras_metadata->cfg = &extra;

				extras.push_back(std::move(new_extras_metadata));
			}
			else {
				//TODO: use a more visible error message.
				ERR_CF << "found " << extra_name << " with id=" << extra["id"] << " twice\n";
			}
		}
	}
}

void create_engine::apply_level_filters()
{
	for(auto& type : type_map_) {
		type.second.apply_filter(player_count_filter_, level_name_filter_);
	}
}

std::vector<create_engine::level_ptr> create_engine::get_levels_by_type_unfiltered(level::TYPE type) const
{
	std::vector<level_ptr> levels;
	for(const level_ptr lvl : type_map_.at(type.v).games) {
		levels.push_back(lvl);
	}

	return levels;
}

std::vector<create_engine::level_ptr> create_engine::get_levels_by_type(level::TYPE type) const
{
    auto& g_list = type_map_.at(type.v);

	std::vector<level_ptr> levels;
	for(size_t level : g_list.games_filtered) {
		levels.push_back(g_list.games[level]);
	}

	return levels;
}

std::vector<size_t> create_engine::get_filtered_level_indices(level::TYPE type) const
{
	return type_map_.at(type.v).games_filtered;
}

const std::vector<create_engine::extras_metadata_ptr>&
	create_engine::get_const_extras_by_type(const MP_EXTRA extra_type) const
{
	return (extra_type == ERA) ? eras_ : mods_;
}

std::vector<create_engine::extras_metadata_ptr>&
	create_engine::get_extras_by_type(const MP_EXTRA extra_type)
{
	return (extra_type == ERA) ? eras_ : mods_;
}

saved_game& create_engine::get_state()
{
	return state_;
}

} // end namespace ng
