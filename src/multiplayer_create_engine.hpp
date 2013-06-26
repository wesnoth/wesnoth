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
#ifndef MULTIPLAYER_CREATE_ENGINE_HPP_INCLUDED
#define MULTIPLAYER_CREATE_ENGINE_HPP_INCLUDED

#include "config.hpp"
#include "map.hpp"
#include "generators/mapgen.hpp"
#include "mp_depcheck.hpp"
#include "mp_game_settings.hpp"

#include <boost/scoped_ptr.hpp>
#include <string>

namespace mp {

class level
{
public:
	level(const config& data);
	virtual ~level() {};

	enum TYPE { SCENARIO, USER_MAP, CAMPAIGN };

	virtual bool can_launch_game() const = 0;

	virtual surface* create_image_surface(const SDL_Rect& image_rect) = 0;

	virtual void set_metadata() = 0;

	virtual std::string name() const;
	virtual std::string description() const;

	void set_data(const config& data);
	const config& data() const;

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
	~scenario();

	bool can_launch_game() const;

	surface* create_image_surface(const SDL_Rect& image_rect);

	void set_metadata();

	int num_players() const;
	std::string map_size() const;

private:
	scenario(const scenario&);
	void operator=(const scenario&);

	void set_sides();

	boost::scoped_ptr<gamemap> map_;
	int num_players_;
};

class user_map : public scenario
{
public:
	user_map(const std::string& name);
	~user_map();

	std::string name() const;
	std::string description() const;

private:
	user_map(const user_map&);
	void operator=(const user_map&);

	std::string name_;
};

class campaign : public level
{
public:
	campaign(const config& data);
	~campaign();

	bool can_launch_game() const;

	surface* create_image_surface(const SDL_Rect& image_rect);

	void set_metadata();

private:
	campaign(const campaign&);
	void operator=(const campaign&);

	std::string image_label_;
};

class create_engine
{
public:
	create_engine(level::TYPE current_level_type, mp_game_settings& parameters,
		depcheck::manager& dependency_manager);
	~create_engine();

	typedef boost::shared_ptr<level> level_ptr;
	typedef boost::shared_ptr<scenario> scenario_ptr;
	typedef boost::shared_ptr<user_map> user_map_ptr;
	typedef boost::shared_ptr<campaign> campaign_ptr;

	void init_current_level_data();
	void init_generated_level_data();

	void prepare_for_new_level();
	void prepare_for_campaign(const std::string& difficulty);
	void prepare_for_saved_game();

	std::vector<std::string>
		levels_menu_item_names(const level::TYPE type) const;

	level& current_level() const;

	void set_current_level_type(const level::TYPE);
	level::TYPE current_level_type() const;

	void set_current_level_index(const size_t index);
	size_t current_level_index() const;

	size_t user_maps_count() const;

	bool generator_assigned() const;
	void generator_user_config(display& disp);

private:
	create_engine(const create_engine&);
	void operator=(const create_engine&);

	void init_all_levels();

	config const* find_selected_level(const std::string& level_type);

	mp_game_settings& parameters_;
	depcheck::manager& dependency_manager_;

	level::TYPE current_level_type_;
	size_t current_level_index_;

	std::vector<scenario_ptr> scenarios_;
	std::vector<user_map_ptr> user_maps_;
	std::vector<campaign_ptr> campaigns_;

	std::vector<std::string> user_map_names_;

	util::scoped_ptr<map_generator> generator_;
};

} // end namespace mp

#endif
