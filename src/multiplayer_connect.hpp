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
#ifndef MULTIPLAYER_CONNECT_H_INCLUDED
#define MULTIPLAYER_CONNECT_H_INCLUDED

#include "network.hpp"
#include "widgets/textbox.hpp"
#include "widgets/button.hpp"
#include "widgets/combo.hpp"
#include "widgets/menu.hpp"
#include "widgets/slider.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class mp_connect
{

public:
	mp_connect(display& disp, std::string game_name,
		   config &cfg, game_data& units_data,
		   game_state& state, bool join = false);
	~mp_connect();

	int load_map(int map, int num_turns, int village_gold,
		     bool fog_game, bool shroud_game);
	int gui_do();

private:
	void lists_init();
	void gui_init();
	void gui_update();
	void add_player(const std::string& name);
	void remove_player(const std::string& name);
	void update_positions();
	void update_network();
	void is_full();

	display *disp_;

	config *cfg_;
	game_data *data_;
	game_state *state_;
	config *level_;
	std::map<config*,network::connection> positions_;

	config loaded_level_;

	bool show_replay_;
	bool save_;
	int status_;
	bool join_;
	bool full_;

	int width_;
	int height_;

	std::vector<std::string> player_types_;
	std::vector<std::string> player_races_;
	std::vector<std::string> player_teams_;
	std::vector<std::string> player_colors_;

	std::vector<gui::combo> combos_type_;
	std::vector<gui::combo> combos_race_;
	std::vector<gui::combo> combos_team_;
	std::vector<gui::combo> combos_color_;
	std::vector<gui::slider> sliders_gold_;

	gui::button launch_;
	gui::button cancel_;

	std::vector<surface_restorer> gold_bg_;
};

#endif
