/*
   Copyright (C) 2013 by Andrius Silinskas <silinskas.andrius@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "multiplayer_create_engine.hpp"

#include "game_config_manager.hpp"
#include "filesystem.hpp"
#include "formula_string_utils.hpp"
#include "log.hpp"
#include "generators/map_create.hpp"
#include "map_exception.hpp"
#include "minimap.hpp"
#include "wml_separators.hpp"
#include "wml_exception.hpp"

#include <boost/foreach.hpp>
#include <sstream>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)

static lg::log_domain log_mp_create("mp/create");
#define DBG_MP LOG_STREAM(debug, log_mp_create)

namespace mp {

level::level(const config& data) :
	data_(data)
{
}

std::string level::description() const
{
	return data_["description"];
}

std::string level::name() const
{
	return data_["name"];
}

void level::set_data(const config& data)
{
	data_ = data;
}

const config& level::data() const
{
	return data_;
}

scenario::scenario(const config& data) :
	level(data),
	map_(),
	num_players_(0)
{
}

scenario::~scenario()
{
}

bool scenario::can_launch_game() const
{
	return map_.get() != NULL;
}

surface* scenario::create_image_surface(const SDL_Rect& image_rect)
{
	surface* minimap = NULL;

	if (map_.get() != NULL) {
		minimap = new surface(image::getMinimap(image_rect.w,
			image_rect.h, *map_, 0));
	}

	return minimap;
}

void scenario::set_metadata()
{
	const std::string& map_data = data_["map_data"];

	try {
		map_.reset(new gamemap(resources::config_manager->game_config(),
			map_data));
	} catch(incorrect_map_format_error& e) {
		ERR_CF << "map could not be loaded: " << e.message << '\n';
	} catch(twml_exception& e) {
		ERR_CF << "map could not be loaded: " << e.dev_message << '\n';
	}

	set_sides();
}

int scenario::num_players() const
{
	return num_players_;
}

std::string scenario::map_size() const
{
	std::stringstream map_size;
	map_size << map_.get()->w();
	map_size << utils::unicode_multiplication_sign;
	map_size << map_.get()->h();

	return map_size.str();
}

void scenario::set_sides()
{
	if (map_.get() != NULL) {
		// If there are less sides in the configuration than there are
		// starting positions, then generate the additional sides
		const int map_positions = map_->num_valid_starting_positions();

		for (int pos = data_.child_count("side");
			pos < map_positions; ++pos) {
			config& side = data_.add_child("side");
			side["side"] = pos + 1;
			side["team_name"] = pos + 1;
			side["canrecruit"] = true;
			side["controller"] = "human";
		}

		num_players_ = 0;
		BOOST_FOREACH(const config &scenario, data_.child_range("side")) {
			if (scenario["allow_player"].to_bool(true)) {
				++num_players_;
			}
		}
	}
}

user_map::user_map(const std::string& name) :
	scenario(config()),
	name_(name)
{
}

user_map::~user_map()
{
}

std::string user_map::description() const
{
	return _("User made map");
}

std::string user_map::name() const
{
	return name_;
}

campaign::campaign(const config& data) :
	level(data),
	image_label_()
{
}

campaign::~campaign()
{
}

bool campaign::can_launch_game() const
{
	return !data_.empty();
}

surface* campaign::create_image_surface(const SDL_Rect& image_rect)
{
	surface temp_image(
		image::get_image(image::locator(image_label_)));

	surface* campaign_image = new surface(scale_surface(temp_image,
		image_rect.w, image_rect.h));

	return campaign_image;
}

void campaign::set_metadata()
{
	image_label_ = data_["image"].str();
}

create_engine::create_engine(level::TYPE current_level_type,
	mp_game_settings& parameters, depcheck::manager& dependency_manager) :
	parameters_(parameters),
	dependency_manager_(dependency_manager),
	current_level_type_(current_level_type),
	current_level_index_(0),
	current_era_index_(0),
	current_mod_index_(0),
	scenarios_(),
	user_maps_(),
	campaigns_(),
	user_map_names_(),
	eras_(),
	mods_(),
	generator_(NULL)
{
	get_files_in_dir(get_user_data_dir() + "/editor/maps", &user_map_names_,
		NULL, FILE_NAME_ONLY);

	init_all_levels();
	init_all_eras();
	init_all_mods();

	parameters_.saved_game = false;
}

create_engine::~create_engine()
{
}

void create_engine::init_current_level_data()
{
	generator_.assign(NULL);

	config const* level = NULL;

	switch (current_level_type_) {
	case level::SCENARIO: {
		level = find_selected_level("multiplayer");

		break;
	}
	case level::USER_MAP: {
		if (const config &generic_multiplayer =
			resources::config_manager->game_config().child(
				"generic_multiplayer")) {
			config data = generic_multiplayer;
			data["map_data"] =
				read_map(user_map_names_[current_level_index_]);

			current_level().set_data(data);
		}

		break;
	}
	case level::CAMPAIGN: {
		level = find_selected_level("campaign");

		break;
	}
	} // end switch

	if (level != NULL) {
		current_level().set_data(*level);
	}
}

void create_engine::init_generated_level_data()
{
	config data = generator_->create_scenario(std::vector<std::string>());

	// Set the scenario to have placing of sides
	// based on the terrain they prefer
	data["modify_placing"] = "true";

	const std::string& description = current_level().data()["description"];
	data["description"] = description;

	current_level().set_data(data);
}

void create_engine::prepare_for_new_level()
{
	parameters_.scenario_data = current_level().data();
}

void create_engine::prepare_for_campaign(const std::string& difficulty)
{
	game_state state = game_state();
	state.classification().campaign_type = "multiplayer";

	if (difficulty != "") {
		state.classification().difficulty = difficulty;
	}

	state.classification().campaign_define =
		current_level().data()["define"].str();
	state.classification().campaign_xtra_defines =
		utils::split(current_level().data()["extra_defines"]);

	resources::config_manager->
		load_game_config_for_game(state.classification());

	current_level().set_data(
		resources::config_manager->game_config().find_child("multiplayer",
		"id", current_level().data()["first_scenario"]));
}

void create_engine::prepare_for_saved_game()
{
	parameters_.saved_game = true;
	parameters_.scenario_data.clear();
}

std::vector<std::string> create_engine::levels_menu_item_names(
	const level::TYPE type) const
{
	std::vector<std::string> menu_names;

	switch (type) {
	case level::SCENARIO: {
		BOOST_FOREACH(level_ptr level, scenarios_) {
			menu_names.push_back(level->name() + HELP_STRING_SEPARATOR +
				level->name());
		}
		break;
	}
	case level::USER_MAP: {
		BOOST_FOREACH(level_ptr level, user_maps_) {
			menu_names.push_back(level->name() + HELP_STRING_SEPARATOR +
				level->name());
		}
		break;
	}
	case level::CAMPAIGN: {
		BOOST_FOREACH(level_ptr level, campaigns_) {
			menu_names.push_back(level->name() + HELP_STRING_SEPARATOR +
				level->name());
		}
		break;
	}
	} // end switch

	return menu_names;
}

std::vector<std::string> create_engine::eras_menu_item_names() const
{
	std::vector<std::string> names;

	BOOST_FOREACH(era_mod_metadata era, eras_) {
		names.push_back(era.first);
	}

	return names;
}

std::vector<std::string> create_engine::mods_menu_item_names() const
{
	std::vector<std::string> names;

	BOOST_FOREACH(era_mod_metadata mod, mods_) {
		names.push_back(mod.first);
	}

	return names;
}

level& create_engine::current_level() const
{
	switch (current_level_type_) {
	case level::SCENARIO: {
		return *scenarios_[current_level_index_];
	}
	case level::USER_MAP: {
		return *user_maps_[current_level_index_];
	}
	case level::CAMPAIGN:
	default: {
		return *campaigns_[current_level_index_];
	}
	} // end switch
}

std::string create_engine::current_era_description() const
{
	return eras_[current_era_index_].second;
}

std::string create_engine::current_mod_description() const
{
	return mods_[current_mod_index_].second;
}

std::string create_engine::current_era_id() const
{
	config::const_child_itors era_list = resources::config_manager->
		game_config().child_range("era");
	for (int num = current_era_index_; num > 0; --num) {
		if (era_list.first == era_list.second) {
			throw config::error(_("Invalid era selected"));
		}
		++era_list.first;
	}

	return (*era_list.first)["id"].str();
}

void create_engine::set_current_level_type(const level::TYPE type)
{
	current_level_type_ = type;
}

level::TYPE create_engine::current_level_type() const
{
	return current_level_type_;
}

void create_engine::set_current_level_index(const size_t index)
{
	current_level_index_ = index;
}

void create_engine::set_current_era_index(const size_t index)
{
	current_era_index_ = index;
}

void create_engine::set_current_mod_index(const size_t index)
{
	current_mod_index_ = index;
}

size_t create_engine::current_level_index() const
{
	return current_level_index_;
}

size_t create_engine::user_maps_count() const
{
	return user_maps_.size();
}

bool create_engine::generator_assigned() const
{
	return generator_ != NULL;
}

void create_engine::generator_user_config(display& disp)
{
	generator_->user_config(disp);
}

void create_engine::init_all_levels()
{
	// User maps.
	for(size_t i = 0; i < user_map_names_.size(); i++)
	{
		user_map_ptr new_user_map(new user_map(user_map_names_[i]));
		user_maps_.push_back(new_user_map);

		// Since user maps are treated as scenarios,
		// some dependency info is required
		config depinfo;
		depinfo["id"] = user_map_names_[i];
		depinfo["name"] = user_map_names_[i];
		dependency_manager_.insert_element(depcheck::SCENARIO, depinfo, i);
	}

	// Stand-alone scenarios.
	BOOST_FOREACH(const config &data,
		resources::config_manager->game_config().child_range("multiplayer"))
	{
		if (data["allow_new_game"].to_bool(true))
		{
			std::string name = data["name"];

			scenario_ptr new_scenario(new scenario(data));
			scenarios_.push_back(new_scenario);
		}
	}

	// Campaigns.
	BOOST_FOREACH(const config &data,
		resources::config_manager->game_config().child_range("campaign"))
	{
		std::string name = data["name"];

		campaign_ptr new_campaign(new campaign(data));
		campaigns_.push_back(new_campaign);
	}
}

void create_engine::init_all_eras()
{
	BOOST_FOREACH(const config &era,
		resources::config_manager->game_config().child_range("era")) {
		eras_.push_back(std::make_pair(era["name"], era["description"]));
	}
}

void create_engine::init_all_mods()
{
	BOOST_FOREACH(const config &mod,
		resources::config_manager->game_config().child_range("modification")) {
		mods_.push_back(std::make_pair(mod["name"], mod["description"]));
	}
}

config const* create_engine::find_selected_level(const std::string& level_type)
{
	size_t index = current_level_index_;

	config::const_child_itors levels =
		resources::config_manager->game_config().child_range(level_type);

	for (; index > 0; --index) {
		if (levels.first == levels.second) {
			break;
		}
		++levels.first;
	}

	if (levels.first != levels.second)
	{
		const config &level = *levels.first;

		// If the map should be randomly generated.
		if (!level["map_generation"].empty()) {
			generator_.assign(create_map_generator(
				level["map_generation"],
				level.child("generator")));
		}

		return &level;
	}

	return NULL;
}

} // end namespace mp
