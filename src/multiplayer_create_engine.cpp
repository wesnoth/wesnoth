/*
   Copyright (C) 2013 - 2014 by Andrius Silinskas <silinskas.andrius@gmail.com>
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
#include "game_display.hpp"
#include "game_preferences.hpp"
#include "filesystem.hpp"
#include "formula_string_utils.hpp"
#include "log.hpp"
#include "generators/map_create.hpp"
#include "map_exception.hpp"
#include "minimap.hpp"
#include "resources.hpp"
#include "saved_game.hpp"
#include "wml_separators.hpp"
#include "wml_exception.hpp"

#include "serialization/preprocessor.hpp"
#include "serialization/parser.hpp"

#include <boost/foreach.hpp>
#include <sstream>
#include <cctype>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)

static lg::log_domain log_mp_create_engine("mp/create/engine");
#define DBG_MP LOG_STREAM(debug, log_mp_create_engine)

namespace {
bool contains_ignore_case(const std::string& str1, const std::string& str2)
{
	if (str2.size() > str1.size()) {
		return false;
	}

	for (size_t i = 0; i<str1.size() - str2.size()+1; i++) {
		bool ok = true;
		for (size_t j = 0; j<str2.size(); j++) {
			if (std::tolower(str1[i+j]) != std::tolower(str2[j])) {
				ok = false;
				break;
			}
		}

		if (ok) {
			return true;
		}
	}

	return false;
}
}

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

std::string level::id() const
{
	return data_["id"];
}

bool level::allow_era_choice() const
{
	return data_["allow_era_choice"].to_bool(true);
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
		data_["description"] = _("Map could not be loaded: ") + e.message;

		ERR_CF << "map could not be loaded: " << e.message << '\n';
	} catch(twml_exception& e) {
		data_["description"] = _("Map could not be loaded.");

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

	if (map_.get() != NULL) {
		map_size << map_.get()->w();
		map_size << utils::unicode_multiplication_sign;
		map_size << map_.get()->h();
	} else {
		map_size << _("not available.");
	}

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

user_map::user_map(const config& data, const std::string& name, gamemap* map) :
	scenario(data),
	name_(name)
{
	if (map != NULL) {
		map_.reset(new gamemap(*map));
	}
}

user_map::~user_map()
{
}

void user_map::set_metadata()
{
	set_sides();
}

std::string user_map::description() const
{
	if (data_["description"].empty()) {
		return _("User made map");
	} else { // map error message
		return data_["description"];
	}
}

std::string user_map::name() const
{
	return name_;
}

std::string user_map::id() const
{
	return name_;
}

random_map::random_map(const config& generator_data) :
	scenario(config()),
	generator_data_(generator_data)
{
}

random_map::~random_map()
{
}

const config& random_map::generator_data() const
{
	return generator_data_;
}

std::string random_map::name() const
{
	return generator_data_["name"];
}

std::string random_map::description() const
{
	return generator_data_["description"];
}

std::string random_map::id() const
{
	return generator_data_["id"];
}

campaign::campaign(const config& data) :
	level(data),
	id_(data["id"]),
	allow_era_choice_(level::allow_era_choice()),
	image_label_(),
	min_players_(2),
	max_players_(2)
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

	int min = data_["min_players"].to_int(2);
	int max = data_["max_players"].to_int(2);

	min_players_ = max_players_ =  min;

	if (max > min) {
		max_players_ = max;
	}
}

std::string campaign::id() const
{
	return id_;
}

bool campaign::allow_era_choice() const
{
	return allow_era_choice_;
}

int campaign::min_players() const
{
	return min_players_;
}

int campaign::max_players() const
{
	return max_players_;
}

create_engine::create_engine(game_display& disp, saved_game& state) :
	current_level_type_(),
	current_level_index_(0),
	current_era_index_(0),
	current_mod_index_(0),
	level_name_filter_(),
	player_count_filter_(1),
	scenarios_(),
	user_maps_(),
	user_scenarios_(),
	campaigns_(),
	sp_campaigns_(),
	random_maps_(),
	user_map_names_(),
	user_scenario_names_(),
	eras_(),
	mods_(),
	state_(state),
	dependency_manager_(resources::config_manager->game_config(), disp.video()),
	generator_(NULL)
{
	DBG_MP << "restoring game config\n";

	// Restore game config for multiplayer.
	state_ = saved_game();
	state_.classification().campaign_type = game_classification::MULTIPLAYER;
	resources::config_manager->
		load_game_config_for_game(state_.classification());

	//TODO the editor dir is already configurable, is the preferences value
	get_files_in_dir(get_user_data_dir() + "/editor/maps", &user_map_names_,
		NULL, FILE_NAME_ONLY);

	get_files_in_dir(get_user_data_dir() + "/editor/scenarios", &user_scenario_names_,
		NULL, FILE_NAME_ONLY);

	DBG_MP << "initializing all levels, eras and mods\n";

	init_all_levels();
	init_extras(ERA);
	init_extras(MOD);

	state_.mp_settings().saved_game = false;

	BOOST_FOREACH (const std::string& str, preferences::modifications()) {
		if (resources::config_manager->
				game_config().find_child("modification", "id", str))
			state_.mp_settings().active_mods.push_back(str);
	}

	if (current_level_type_ != level::CAMPAIGN &&
		current_level_type_ != level::SP_CAMPAIGN) {
		dependency_manager_.try_modifications(state_.mp_settings().active_mods, true);
	}

	reset_level_filters();
}

create_engine::~create_engine()
{
}

void create_engine::init_generated_level_data()
{
	DBG_MP << "initializing generated level data\n";

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
	DBG_MP << "preparing mp_game_settings for new level\n";

	state_.set_scenario(current_level().data());
	state_.mp_settings().hash = current_level().data().hash();
}

void create_engine::prepare_for_scenario()
{
	DBG_MP << "preparing data for campaign by reloading game config\n";

	state_.classification().scenario_define =
		current_level().data()["define"].str();
	state_.classification().era_define =
		resources::config_manager->game_config().find_child(
			"era", "id", get_parameters().mp_era)["define"].str();
	
	resources::config_manager->
		load_game_config_for_game(state_.classification());

	current_level().set_data(
		resources::config_manager->game_config().find_child(
		lexical_cast<std::string> (game_classification::MULTIPLAYER),
		"id", current_level().data()["id"]));
}

void create_engine::prepare_for_campaign(const std::string& difficulty)
{
	DBG_MP << "preparing data for campaign by reloading game config\n";

	if (difficulty != "") {
		state_.classification().difficulty = difficulty;
	}

	state_.classification().campaign = current_level().data()["id"].str();
	state_.classification().abbrev = current_level().data()["abbrev"].str();

	state_.classification().end_text = current_level().data()["end_text"].str();
	state_.classification().end_text_duration =
		current_level().data()["end_text_duration"];

	state_.classification().campaign_define =
		current_level().data()["define"].str();
	state_.classification().campaign_xtra_defines =
		utils::split(current_level().data()["extra_defines"]);
	state_.classification().era_define =
		resources::config_manager->game_config().find_child(
			"era", "id", get_parameters().mp_era)["define"].str();

	resources::config_manager->
		load_game_config_for_game(state_.classification());

	current_level().set_data(
		resources::config_manager->game_config().find_child(
		lexical_cast<std::string> (game_classification::MULTIPLAYER),
		"id", current_level().data()["first_scenario"]));
}

void create_engine::prepare_for_saved_game()
{
	DBG_MP << "preparing mp_game_settings for saved game\n";

	resources::config_manager->load_game_config_for_game(state_.classification());
	//The save migh be a start-of-scenario save so make sure we have the scenario data loaded.
	state_.expand_scenario();
	state_.mp_settings().saved_game = true;

	utils::string_map i18n_symbols;
	i18n_symbols["login"] = preferences::login();
	state_.mp_settings().name = vgettext("$login|’s game", i18n_symbols);
}

void create_engine::apply_level_filter(const std::string &name)
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
	scenarios_filtered_.clear();
	for (size_t i = 0; i<scenarios_.size(); i++) {
		scenarios_filtered_.push_back(i);
	}

	user_scenarios_filtered_.clear();
	for (size_t i = 0; i<user_scenarios_.size(); i++) {
		user_scenarios_filtered_.push_back(i);
	}

	user_maps_filtered_.clear();
	for (size_t i = 0; i<user_maps_.size(); i++) {
		user_maps_filtered_.push_back(i);
	}

	campaigns_filtered_.clear();
	for (size_t i = 0; i<campaigns_.size(); i++) {
		campaigns_filtered_.push_back(i);
	}

	sp_campaigns_filtered_.clear();
	for (size_t i = 0; i<sp_campaigns_.size(); i++) {
		sp_campaigns_filtered_.push_back(i);
	}

	random_maps_filtered_.clear();
	for (size_t i = 0; i<random_maps_.size(); i++) {
		random_maps_filtered_.push_back(i);
	}

	level_name_filter_ = "";
}

const std::string &create_engine::level_name_filter() const
{
	return level_name_filter_;
}

int create_engine::player_num_filter() const
{
	return player_count_filter_;
}

std::vector<std::string> create_engine::levels_menu_item_names() const
{
	std::vector<std::string> menu_names;

	BOOST_FOREACH(level_ptr level, get_levels_by_type(current_level_type_)) {
		menu_names.push_back(level->name() + HELP_STRING_SEPARATOR +
			level->name());
	}

	return menu_names;
}

std::vector<std::string> create_engine::extras_menu_item_names(
	const MP_EXTRA extra_type) const
{
	std::vector<std::string> names;

	BOOST_FOREACH(extras_metadata_ptr extra,
		get_const_extras_by_type(extra_type)) {
		names.push_back(extra->name);
	}

	return names;
}

level& create_engine::current_level() const
{
	switch (current_level_type_) {
	case level::SCENARIO: {
		return *scenarios_[current_level_index_];
	}
	case level::USER_SCENARIO: {
		return *user_scenarios_[current_level_index_];
	}
	case level::USER_MAP: {
		return *user_maps_[current_level_index_];
	}
	case level::RANDOM_MAP: {
		return *random_maps_[current_level_index_];
	}
	case level::CAMPAIGN: {
		return *campaigns_[current_level_index_];
	}
	case level::SP_CAMPAIGN:
	default: {
		return *sp_campaigns_[current_level_index_];
	}
	} // end switch
}

const create_engine::extras_metadata& create_engine::current_extra(
	const MP_EXTRA extra_type) const
{
	const size_t index = (extra_type == ERA) ?
		current_era_index_ : current_mod_index_;

	return *get_const_extras_by_type(extra_type)[index];
}

void create_engine::set_current_level_type(const level::TYPE type)
{
	current_level_type_ = type;
}

level::TYPE create_engine::current_level_type() const
{
	return current_level_type_;
}

void create_engine::set_current_level(const size_t index)
{
	switch (current_level_type()) {
	case level::CAMPAIGN:
		current_level_index_ = campaigns_filtered_[index];
		break;
	case level::SP_CAMPAIGN:
		current_level_index_ = sp_campaigns_filtered_[index];
		break;
	case level::SCENARIO:
		current_level_index_ = scenarios_filtered_[index];
		break;
	case level::RANDOM_MAP:
		current_level_index_ = random_maps_filtered_[index];
		break;
	case level::USER_MAP:
		current_level_index_ = user_maps_filtered_[index];
		break;
	case level::USER_SCENARIO:
		current_level_index_ = user_scenarios_filtered_[index];
	}

	if (current_level_type_ == level::RANDOM_MAP) {
		random_map* current_random_map =
			dynamic_cast<random_map*>(&current_level());

		generator_.assign(create_map_generator(
			current_random_map->generator_data()["map_generation"],
			current_random_map->generator_data().child("generator")));
	} else {
		generator_.assign(NULL);
	}

	if (current_level_type_ != level::CAMPAIGN &&
		current_level_type_ != level::SP_CAMPAIGN) {

		dependency_manager_.try_scenario(current_level().id());
	}
}

void create_engine::set_current_era_index(const size_t index)
{
	current_era_index_ = index;

	dependency_manager_.try_era_by_index(index);
}

void create_engine::set_current_mod_index(const size_t index)
{
	current_mod_index_ = index;
}

bool create_engine::toggle_current_mod()
{
	bool is_active = dependency_manager_.is_modification_active(current_mod_index_);
	dependency_manager_.try_modification_by_index(current_mod_index_, !is_active);

	state_.mp_settings().active_mods = dependency_manager_.get_modifications();

	return !is_active;
}

bool create_engine::generator_assigned() const
{
	return generator_ != NULL;
}

void create_engine::generator_user_config(display& disp)
{
	generator_->user_config(disp);
}

int create_engine::find_level_by_id(const std::string& id) const
{
	int i = 0;
	BOOST_FOREACH(user_map_ptr user_map, user_maps_) {
		if (user_map->id() == id) {
			return i;
		}
		i++;
	}

	i = 0;
	BOOST_FOREACH(random_map_ptr random_map, random_maps_) {
		if (random_map->id() == id) {
			return i;
		}
		i++;
	}

	i = 0;
	BOOST_FOREACH(scenario_ptr scenario, scenarios_) {
		if (scenario->id() == id) {
			return i;
		}
		i++;
	}

	i = 0;
	BOOST_FOREACH(scenario_ptr scenario, user_scenarios_) {
		if (scenario->id() == id) {
			return i;
		}
		i++;
	}

	i = 0;
	BOOST_FOREACH(campaign_ptr campaign, campaigns_) {
		if (campaign->id() == id) {
			return i;
		}
		i++;
	}

	i = 0;
	BOOST_FOREACH(campaign_ptr sp_campaign, sp_campaigns_) {
		if (sp_campaign->id() == id) {
			return i;
		}
		i++;
	}

	return -1;
}

int create_engine::find_extra_by_id(const MP_EXTRA extra_type,
	const std::string& id) const
{
	int i = 0;
	BOOST_FOREACH(extras_metadata_ptr extra,
		get_const_extras_by_type(extra_type)) {
		if (extra->id == id) {
			return i;
		}
		i++;
	}

	return -1;
}

level::TYPE create_engine::find_level_type_by_id(const std::string& id) const
{
	BOOST_FOREACH(user_map_ptr user_map, user_maps_) {
		if (user_map->id() == id) {
			return level::USER_MAP;
		}
	}
	BOOST_FOREACH(random_map_ptr random_map, random_maps_) {
		if (random_map->id() == id) {
			return level::RANDOM_MAP;
		}
	}
	BOOST_FOREACH(scenario_ptr scenario, scenarios_) {
		if (scenario->id() == id) {
			return level::SCENARIO;
		}
	}
	BOOST_FOREACH(scenario_ptr scenario, user_scenarios_) {
		if (scenario->id() == id) {
			return level::USER_SCENARIO;
		}
	}
	BOOST_FOREACH(campaign_ptr campaign, campaigns_) {
		if (campaign->id() == id) {
			return level::CAMPAIGN;
		}
	}
	return level::SP_CAMPAIGN;
}

const depcheck::manager& create_engine::dependency_manager() const
{
	return dependency_manager_;
}

void create_engine::init_active_mods()
{
	state_.mp_settings().active_mods = dependency_manager_.get_modifications();
}

std::vector<std::string>& create_engine::active_mods()
{
	return state_.mp_settings().active_mods;
}

const mp_game_settings& create_engine::get_parameters()
{
	DBG_MP << "getting parameter values" << std::endl;

	int era_index = current_level().allow_era_choice() ? current_era_index_ : 0;
	state_.mp_settings().mp_era = eras_[era_index]->id;

	return state_.mp_settings();
}

void create_engine::init_all_levels()
{
	if (const config &generic_multiplayer =
		resources::config_manager->game_config().child(
			"generic_multiplayer")) {
		config gen_mp_data = generic_multiplayer;

		// User maps.
		int dep_index_offset = 0;
		for(size_t i = 0; i < user_map_names_.size(); i++)
		{
			config user_map_data = gen_mp_data;
			user_map_data["map_data"] = read_map(user_map_names_[i]);

			// Check if a file is actually a map.
			// Note that invalid maps should be displayed in order to
			// show error messages in the GUI.
			bool add_map = true;
			boost::scoped_ptr<gamemap> map;
			try {
				map.reset(new gamemap(resources::config_manager->game_config(),
					user_map_data["map_data"]));
			} catch (incorrect_map_format_error& e) {
				user_map_data["description"] = _("Map could not be loaded: ") +
					e.message;

				ERR_CF << "map could not be loaded: " << e.message << '\n';
			} catch (twml_exception&) {
				add_map = false;
				dep_index_offset++;
			}

			if (add_map) {
				user_map_ptr new_user_map(new user_map(user_map_data,
					user_map_names_[i], map.get()));
				user_maps_.push_back(new_user_map);
				user_maps_.back()->set_metadata();

				// Since user maps are treated as scenarios,
				// some dependency info is required
				config depinfo;
				depinfo["id"] = user_map_names_[i];
				depinfo["name"] = user_map_names_[i];
				dependency_manager_.insert_element(depcheck::SCENARIO, depinfo,
				i - dep_index_offset);
			}
		}

		// User made scenarios.
		dep_index_offset = 0;
		for(size_t i = 0; i < user_scenario_names_.size(); i++)
		{
			config data;
			try {
				read(data, *(preprocess_file(get_user_data_dir() + "/editor/scenarios/" + user_scenario_names_[i])));
			} catch (config::error & e) {
				ERR_CF << "Caught a config error while parsing user made (editor) scenarios:\n" << e.message << std::endl;
				ERR_CF << "Skipping file: " << (get_user_data_dir() + "/editor/scenarios/" + user_scenario_names_[i]) << std::endl;
				continue;
			}

			scenario_ptr new_scenario(new scenario(data));
			if (new_scenario->id().empty()) continue;
			user_scenarios_.push_back(new_scenario);
			user_scenarios_.back()->set_metadata();

			// Since user scenarios are treated as scenarios,
			// some dependency info is required
			config depinfo;
			depinfo["id"] = data["id"];
			depinfo["name"] = data["name"];
			dependency_manager_.insert_element(depcheck::SCENARIO, depinfo,
					i - dep_index_offset++);
		}
	}

	// Stand-alone scenarios.
	BOOST_FOREACH(const config &data,
		resources::config_manager->game_config().child_range(
		lexical_cast<std::string> (game_classification::MULTIPLAYER)))
	{
		if (!data["allow_new_game"].to_bool(true))
			continue;

		if (!data["map_generation"].empty()) {
			random_map_ptr new_random_map(new random_map(data));
			random_maps_.push_back(new_random_map);
			random_maps_.back()->set_metadata();
		} else {
			scenario_ptr new_scenario(new scenario(data));
			scenarios_.push_back(new_scenario);
			scenarios_.back()->set_metadata();
		}
	}

	// Campaigns.
	BOOST_FOREACH(const config &data,
		resources::config_manager->game_config().child_range("campaign"))
	{
		const std::string& type = data["type"];

		if (type == "mp" || type == "hybrid") {
			campaign_ptr new_campaign(new campaign(data));
			campaigns_.push_back(new_campaign);
			campaigns_.back()->set_metadata();
		} else {
			campaign_ptr new_sp_campaign(new campaign(data));
			sp_campaigns_.push_back(new_sp_campaign);
			sp_campaigns_.back()->set_metadata();
		}
	}
}

void create_engine::init_extras(const MP_EXTRA extra_type)
{
	std::vector<extras_metadata_ptr>& extras = get_extras_by_type(extra_type);
	const std::string extra_name = (extra_type == ERA) ? "era" : "modification";

	BOOST_FOREACH(const config &extra,
		resources::config_manager->game_config().child_range(extra_name)) {

		extras_metadata_ptr new_extras_metadata(new extras_metadata());
		new_extras_metadata->id = extra["id"].str();
		new_extras_metadata->name = extra["name"].str();
		new_extras_metadata->description = extra["description"].str();

		extras.push_back(new_extras_metadata);
	}
}

void create_engine::apply_level_filters()
{
	scenarios_filtered_.clear();
	for (size_t i = 0; i<scenarios_.size(); i++) {
		if (contains_ignore_case(scenarios_[i]->name(), level_name_filter_) &&
			(player_count_filter_ == 1 ||
			 scenarios_[i]->num_players() == player_count_filter_)) {
			scenarios_filtered_.push_back(i);
		}
	}

	user_scenarios_filtered_.clear();
	for (size_t i = 0; i<user_scenarios_.size(); i++) {
		if (contains_ignore_case(user_scenarios_[i]->name(), level_name_filter_) &&
			(player_count_filter_ == 1 ||
			 user_scenarios_[i]->num_players() == player_count_filter_)) {
			user_scenarios_filtered_.push_back(i);
		}
	}

	user_maps_filtered_.clear();
	for (size_t i = 0; i<user_maps_.size(); i++) {
		if (contains_ignore_case(user_maps_[i]->name(), level_name_filter_) &&
			(player_count_filter_ == 1 ||
			 user_maps_[i]->num_players() == player_count_filter_)) {
			user_maps_filtered_.push_back(i);
		}
	}

	campaigns_filtered_.clear();
	for (size_t i = 0; i<campaigns_.size(); i++) {
		if (contains_ignore_case(campaigns_[i]->name(), level_name_filter_) &&
			(player_count_filter_ == 1 ||
			(campaigns_[i]->min_players() <= player_count_filter_ &&
			 campaigns_[i]->max_players() >= player_count_filter_))) {
			campaigns_filtered_.push_back(i);
		}
	}

	sp_campaigns_filtered_.clear();
	for (size_t i = 0; i<sp_campaigns_.size(); i++) {
		if (contains_ignore_case(sp_campaigns_[i]->name(), level_name_filter_) &&
			(player_count_filter_ == 1 ||
			(sp_campaigns_[i]->min_players() <= player_count_filter_ &&
			 sp_campaigns_[i]->max_players() >= player_count_filter_))) {
			sp_campaigns_filtered_.push_back(i);
		}
	}

	random_maps_filtered_.clear();
	for (size_t i = 0; i<random_maps_.size(); i++) {
		if (contains_ignore_case(random_maps_[i]->name(), level_name_filter_) &&
			(player_count_filter_ == 1 ||
			 random_maps_[i]->num_players() == player_count_filter_)) {
			random_maps_filtered_.push_back(i);
		}
	}

}

std::vector<create_engine::level_ptr>
	create_engine::get_levels_by_type_unfiltered(level::TYPE type) const
{
	std::vector<level_ptr> levels;
	switch (type) {
	case level::SCENARIO:
		BOOST_FOREACH(level_ptr level, scenarios_) {
			levels.push_back(level);
		}
		break;
	case level::USER_MAP:
		BOOST_FOREACH(level_ptr level, user_maps_) {
			levels.push_back(level);
		}
		break;
	case level::USER_SCENARIO:
		BOOST_FOREACH(level_ptr level, user_scenarios_) {
			levels.push_back(level);
		}
		break;
	case level::RANDOM_MAP:
		BOOST_FOREACH(level_ptr level, random_maps_) {
			levels.push_back(level);
		}
		break;
	case level::CAMPAIGN:
		BOOST_FOREACH(level_ptr level, campaigns_) {
			levels.push_back(level);
		}
		break;
	case level::SP_CAMPAIGN:
		BOOST_FOREACH(level_ptr level, sp_campaigns_) {
			levels.push_back(level);
		}
		break;
	} // end switch

	return levels;
}

std::vector<create_engine::level_ptr> create_engine::get_levels_by_type(level::TYPE type) const
{
	std::vector<level_ptr> levels;
	switch (type) {
	case level::SCENARIO:
		BOOST_FOREACH(size_t level, scenarios_filtered_) {
			levels.push_back(scenarios_[level]);
		}
		break;
	case level::USER_MAP:
		BOOST_FOREACH(size_t level, user_maps_filtered_) {
			levels.push_back(user_maps_[level]);
		}
		break;
	case level::USER_SCENARIO:
		BOOST_FOREACH(size_t level, user_scenarios_filtered_) {
			levels.push_back(user_scenarios_[level]);
		}
		break;
	case level::RANDOM_MAP:
		BOOST_FOREACH(size_t level, random_maps_filtered_) {
			levels.push_back(random_maps_[level]);
		}
		break;
	case level::CAMPAIGN:
		BOOST_FOREACH(size_t level, campaigns_filtered_) {
			levels.push_back(campaigns_[level]);
		}
		break;
	case level::SP_CAMPAIGN:
		BOOST_FOREACH(size_t level, sp_campaigns_filtered_) {
			levels.push_back(sp_campaigns_[level]);
		}
		break;
	} // end switch

	return levels;
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

} // end namespace mp
