/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef MULTIPLAYER_HPP_INCLUDED
#define MULTIPLAYER_HPP_INCLUDED

#include "config.hpp"
#include "display.hpp"
#include "font.hpp"
#include "gamestatus.hpp"
#include "mapgen.hpp"
#include "multiplayer_lobby.hpp"
#include "scoped_resource.hpp"
#include "unit_types.hpp"
#include "video.hpp"

#include "widgets/button.hpp"
#include "widgets/combo.hpp"
#include "widgets/menu.hpp"
#include "widgets/slider.hpp"
#include "widgets/textbox.hpp"

//an object which guarantees that when it is destroyed, a 'leave game'
//message will be sent to any hosts still connected.
struct network_game_manager {
	network_game_manager() {}
	~network_game_manager();
};

class multiplayer_game_setup_dialog : public lobby::dialog, public font::floating_label_context
{
public:
	multiplayer_game_setup_dialog(display& disp, game_data& units_data,
                      const config& cfg, game_state& state, bool server=false, const std::string& controller="ai");

	virtual void set_area(const SDL_Rect& area);
	virtual void clear_area();

	lobby::RESULT process();

	void start_game();

private:
	display& disp_;
	game_data& units_data_;
	const config& cfg_;
	game_state& state_;
	bool server_;
	SDL_Rect area_;

	config* level_;

	int map_selection_;

	std::vector<std::string> user_maps_, map_options_;
	config scenario_data_;

	util::scoped_ptr<gui::menu> maps_menu_;
	util::scoped_ptr<gui::slider> turns_slider_, village_gold_slider_, xp_modifier_slider_;
	util::scoped_ptr<gui::button> fog_game_, shroud_game_, observers_game_,
	                              cancel_game_, launch_game_,
	                              regenerate_map_, generator_settings_;

	util::scoped_ptr<gui::combo> era_combo_, vision_combo_;
	util::scoped_ptr<gui::textbox> name_entry_;

	util::scoped_ptr<map_generator> generator_;

	surface_restorer turns_restorer_, village_gold_restorer_, xp_restorer_, playernum_restorer_,
	                 minimap_restorer_;

	const std::string controller_;
};

//function to host a multiplayer game. If server is true, then the
//game will accept connections from foreign hosts. Otherwise it'll
//just use existing network connections for players, or the game
//is an entirely local game
int play_multiplayer(display& disp, game_data& units_data,
                      const config& cfg, game_state& state, bool server=true);

#endif
