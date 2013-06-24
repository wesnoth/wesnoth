/*
   Copyright (C) 2007 - 2013
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#ifndef MULTIPLAYER_CREATE_HPP_INCLUDED
#define MULTIPLAYER_CREATE_HPP_INCLUDED

#include "gamestatus.hpp"
#include "mp_depcheck.hpp"
#include "mp_game_settings.hpp"
#include "multiplayer_ui.hpp"
#include "widgets/slider.hpp"
#include "widgets/combo.hpp"
#include "generators/mapgen.hpp"
#include "tooltips.hpp"

namespace mp {

struct mp_level
{
public:
	mp_level();

	void reset();

	void set_scenario();
	void set_campaign();

	enum TYPE { SCENARIO, CAMPAIGN };
	TYPE get_type() const;

	std::string map_data;
	std::string image_label;
	config campaign;

private:
	TYPE type;
};

class create : public mp::ui
{
public:
	create(game_display& dist, const config& game_config, chat& c, config& gamelist, bool local_players_only);
	~create();

	mp_game_settings& get_parameters();

protected:
	virtual void layout_children(const SDL_Rect& rect);
	virtual void process_event();
	virtual void hide_children(bool hide=true);

private:
	void get_level_image();
	void draw_level_image();

	enum SET_LEVEL {
		GENERIC_MULTIPLAYER,
		MULTIPLAYER,
		SAVED_GAME,
		GENERATED_MAP,
		CAMPAIGN
	};
	bool set_level_data(SET_LEVEL set_level, const int select);
	void set_level_sides(const int map_positions);

	bool new_campaign();

	void set_levels_menu(const bool init_dep_check = false);
	void synchronize_selections();

	bool local_players_only_;

	tooltips::manager tooltip_manager_;
	int era_selection_;
	int map_selection_;
	int mod_selection_;


	std::vector<std::string> user_maps_;
	std::vector<std::string> era_options_;
	std::vector<std::string> map_options_;
	std::vector<std::string> mod_options_;
	std::vector<std::string> era_descriptions_;
	std::vector<std::string> map_descriptions_;
	std::vector<std::string> mod_descriptions_;

	/**
	 * Due to maps not available the index of the selected map and mp scenarios
	 * is not 1:1 so we use a lookup table.
	 */
	std::vector<size_t> map_index_;

	gui::menu eras_menu_;
	gui::menu maps_menu_;
	gui::menu mods_menu_;

	gui::label filter_name_label_;
	gui::label filter_num_players_label_;
	gui::label map_generator_label_;
	gui::label era_label_;
	gui::label map_label_;
	gui::label mod_label_;
	gui::label map_size_label_;
	gui::label num_players_label_;

	gui::button launch_game_;
	gui::button cancel_game_;
	gui::button regenerate_map_;
	gui::button generator_settings_;

	gui::button show_scenarios_;
	gui::button show_campaigns_;

	gui::button switch_levels_menu_;

	gui::slider filter_num_players_slider_;

	gui::textbox description_;
	gui::textbox filter_name_;

	util::scoped_ptr<surface_restorer> image_restorer_;
	SDL_Rect image_rect_;

	boost::scoped_ptr<gamemap> map_;

	util::scoped_ptr<map_generator> generator_;

	mp_game_settings parameters_;

	mp_level mp_level_;
	game_state state_;

	depcheck::manager dependency_manager_;
};

} // end namespace mp

#endif

