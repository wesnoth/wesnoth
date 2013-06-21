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

#include "mp_depcheck.hpp"
#include "mp_game_settings.hpp"
#include "multiplayer_ui.hpp"
#include "widgets/slider.hpp"
#include "widgets/combo.hpp"
#include "generators/mapgen.hpp"
#include "tooltips.hpp"

namespace mp {

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
	boost::shared_ptr<gamemap> get_map();

	enum SET_LEVEL {
		GENERIC_MULTIPLAYER,
		MULTIPLAYER,
		SAVED_GAME,
		GENERATED_MAP
	};
	bool set_level_data(SET_LEVEL set_level, const int select);
	void set_level_sides(const int map_positions);

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

	gui::slider filter_num_players_slider_;

	gui::textbox description_;
	gui::textbox filter_name_;

	util::scoped_ptr<surface_restorer> minimap_restorer_;
	SDL_Rect minimap_rect_;

	util::scoped_ptr<map_generator> generator_;

	mp_game_settings parameters_;

	depcheck::manager dependency_manager_;
};

} // end namespace mp

#endif

