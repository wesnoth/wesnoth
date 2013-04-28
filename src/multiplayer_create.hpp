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
#include "mp_options.hpp"
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

	void synchronize_selections();

	bool local_players_only_;

	tooltips::manager tooltip_manager_;
	int era_selection_;
	int map_selection_;


	std::vector<std::string> user_maps_;
	std::vector<std::string> map_options_;
	std::vector<std::string> era_options_;
	config available_mods_;

	/**
	 * Due to maps not available the index of the selected map and mp scenarios
	 * is not 1:1 so we use a lookup table.
	 */
	std::vector<size_t> map_index_;

	gui::menu eras_menu_;
	gui::menu maps_menu_;

	gui::label era_label_;
	gui::label map_label_;
	gui::label map_size_label_;
	gui::label num_players_label_;

	gui::button choose_mods_;
	gui::button launch_game_;
	gui::button cancel_game_;
	gui::button regenerate_map_;
	gui::button generator_settings_;

	util::scoped_ptr<surface_restorer> minimap_restorer_;
	SDL_Rect minimap_rect_;

	util::scoped_ptr<map_generator> generator_;

	mp_game_settings parameters_;

	depcheck::manager dependency_manager_;
};

} // end namespace mp

#endif

