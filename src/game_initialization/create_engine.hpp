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
#ifndef MULTIPLAYER_CREATE_ENGINE_HPP_INCLUDED
#define MULTIPLAYER_CREATE_ENGINE_HPP_INCLUDED

#include "config.hpp"
#include "map.hpp"
#include "depcheck.hpp"
#include "mp_game_settings.hpp"
#include "game_display.hpp"

#include <boost/scoped_ptr.hpp>
#include <string>
#include <utility>

class saved_game;
class map_generator;
namespace ng {
class level
{
public:
	level(const config& data);
	virtual ~level() {}

	enum TYPE { SCENARIO, USER_MAP, USER_SCENARIO, RANDOM_MAP, CAMPAIGN, SP_CAMPAIGN};

	virtual bool can_launch_game() const = 0;

	virtual surface* create_image_surface(const SDL_Rect& image_rect) = 0;

	virtual void set_metadata() = 0;

	virtual std::string name() const;
	virtual std::string icon() const;
	virtual std::string description() const;
	virtual std::string id() const;
	virtual bool allow_era_choice() const;

	void set_data(const config& data);
	const config& data() const;
	config& data();

protected:
	config data_;

private:
	level(const level&);
	void operator=(const level&);
};

class scenario : public level
{
public:
	scenario(const config& data);
	virtual ~scenario();

	bool can_launch_game() const;

	surface* create_image_surface(const SDL_Rect& image_rect);

	void set_metadata();

	int num_players() const;
	std::string map_size() const;

protected:
	void set_sides();

	boost::scoped_ptr<gamemap> map_;

private:
	scenario(const scenario&);
	void operator=(const scenario&);

	int num_players_;
};

class user_map : public scenario
{
public:
	user_map(const config& data, const std::string& name, gamemap* map);
	virtual ~user_map();

	void set_metadata();

	std::string name() const;
	std::string description() const;
	std::string id() const;

private:
	user_map(const user_map&);
	void operator=(const user_map&);

	std::string name_;
};

class random_map : public scenario
{
public:
	random_map(const config& data);
	virtual ~random_map();

	const config& generator_data() const;

	std::string name() const;
	std::string description() const;
	std::string id() const;
	std::string generator_name() const;

	map_generator * create_map_generator() const;

	bool generate_whole_scenario() const;

private:
	random_map(const random_map&);
	void operator=(const random_map&);

	config generator_data_;

	bool generate_whole_scenario_;
	std::string generator_name_;
};

class campaign : public level
{
public:
	campaign(const config& data);
	virtual ~campaign();

	bool can_launch_game() const;

	surface* create_image_surface(const SDL_Rect& image_rect);

	void set_metadata();

	void mark_if_completed();

	std::string id() const;

	bool allow_era_choice() const;

	int min_players() const;
	int max_players() const;

private:
	campaign(const campaign&);
	void operator=(const campaign&);

	std::string id_;
	bool allow_era_choice_;
	std::string image_label_;
	int min_players_;
	int max_players_;
};

class create_engine
{
public:
	create_engine(game_display& disp, saved_game& state);
	~create_engine();

	enum MP_EXTRA { ERA, MOD };

	struct extras_metadata
	{
		std::string id;
		std::string name;
		std::string description;
	};

	typedef boost::shared_ptr<extras_metadata> extras_metadata_ptr;

	typedef boost::shared_ptr<level> level_ptr;
	typedef boost::shared_ptr<scenario> scenario_ptr;
	typedef boost::shared_ptr<user_map> user_map_ptr;
	typedef boost::shared_ptr<random_map> random_map_ptr;
	typedef boost::shared_ptr<campaign> campaign_ptr;

	void init_generated_level_data();

	void prepare_for_new_level();
	void prepare_for_era_and_mods();
	void prepare_for_scenario();
	void prepare_for_campaign(const std::string& difficulty);
	void prepare_for_saved_game();
	//random maps, user maps
	void prepare_for_other();
	
	std::string select_campaign_difficulty(int set_value = -1);

	void apply_level_filter(const std::string& name);
	void apply_level_filter(int players);
	void reset_level_filters();

	const std::string& level_name_filter() const;
	int player_num_filter() const;

	std::vector<level_ptr> get_levels_by_type_unfiltered(level::TYPE type) const;
	std::vector<level_ptr> get_levels_by_type(level::TYPE type) const;

	std::vector<std::string> levels_menu_item_names() const;
	std::vector<std::string> extras_menu_item_names(
		const MP_EXTRA extra_type) const;

	level& current_level() const;
	const extras_metadata& current_extra(const MP_EXTRA extra_type) const;

	void set_current_level_type(const level::TYPE);
	level::TYPE current_level_type() const;

	void set_current_level(const size_t index);

	void set_current_era_index(const size_t index, bool force = false);
	void set_current_mod_index(const size_t index);

	size_t current_era_index() const;
	size_t current_mod_index() const;

	const std::vector<extras_metadata_ptr>&
		get_const_extras_by_type(const MP_EXTRA extra_type) const;
	std::vector<extras_metadata_ptr>&
		get_extras_by_type(const MP_EXTRA extra_type);

	bool toggle_current_mod(bool force = false);
	
	bool generator_assigned() const;
	void generator_user_config(display& disp);

	int find_level_by_id(const std::string& id) const;
	int find_extra_by_id(const MP_EXTRA extra_type, const std::string& id) const;
	level::TYPE find_level_type_by_id(const std::string& id) const;

	const depcheck::manager& dependency_manager() const;

	void init_active_mods();
	std::vector<std::string>& active_mods();

	const mp_game_settings& get_parameters();

	saved_game& get_state();

private:
	create_engine(const create_engine&);
	void operator=(const create_engine&);

	void init_all_levels();
	void init_extras(const MP_EXTRA extra_type);
	void apply_level_filters();

	size_t map_level_index(size_t index) const;

	level::TYPE current_level_type_;
	size_t current_level_index_;

	size_t current_era_index_;
	size_t current_mod_index_;

	std::string level_name_filter_;
	int player_count_filter_;

	std::vector<scenario_ptr> scenarios_;
	std::vector<user_map_ptr> user_maps_;
	std::vector<scenario_ptr> user_scenarios_;
	std::vector<campaign_ptr> campaigns_;
	std::vector<campaign_ptr> sp_campaigns_;
	std::vector<random_map_ptr> random_maps_;

	std::vector<size_t> scenarios_filtered_;
	std::vector<size_t> user_maps_filtered_;
	std::vector<size_t> user_scenarios_filtered_;
	std::vector<size_t> campaigns_filtered_;
	std::vector<size_t> sp_campaigns_filtered_;
	std::vector<size_t> random_maps_filtered_;

	std::vector<std::string> user_map_names_;
	std::vector<std::string> user_scenario_names_;

	std::vector<extras_metadata_ptr> eras_;
	std::vector<extras_metadata_ptr> mods_;

	saved_game& state_;

	game_display& disp_;

	depcheck::manager dependency_manager_;

	boost::scoped_ptr<map_generator> generator_;
};

} // end namespace ng
#endif
