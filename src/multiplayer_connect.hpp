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

#include "multiplayer_lobby.hpp"
#include "multiplayer_client.hpp"
#include "network.hpp"
#include "widgets/textbox.hpp"
#include "widgets/button.hpp"
#include "widgets/combo.hpp"
#include "widgets/menu.hpp"
#include "widgets/slider.hpp"
#include "widgets/scrollpane.hpp"
#include "widgets/label.hpp"

#include <deque>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class mp_connect : public lobby::dialog
{

public:
	mp_connect(display& disp, std::string game_name,
		   const config &cfg, game_data& units_data,
		   game_state& state, bool join = false,
	       const std::string& default_controller="ai");
	~mp_connect();

	int load_map(const std::string& era, config& scenario, int num_turns, int village_gold, int xpmodifier,
	             bool fog_game, bool shroud_game, bool allow_observers, bool share_view, bool share_maps);

	void start_game();

private:
	virtual void set_area(const SDL_Rect& rect);
	virtual void clear_area();

	lobby::RESULT process();
	bool manages_network() const { return true; }
	bool get_network_data(config& cfg);

	void lists_init();
	void gui_update();
	void add_player(const std::string& name);
	void remove_player(const std::string& name);
	void update_positions();
	void update_network();
	bool is_full();

	size_t combo_index_to_team(size_t index) const;

	display *disp_;

	std::string era_;

	const config *cfg_;
	game_data *data_;
	game_state *state_;
	config *level_;

	//the state the scenario is in before changes,
	//so that we can generate a diff to send to clients
	config old_level_;
	std::map<config*,network::connection> positions_;

	bool show_replay_;
	bool save_;
	int status_;
	bool join_;

	SDL_Rect rect_;

	std::vector<std::string> player_types_;
	std::vector<std::string> player_races_;
	std::vector<std::string> player_teams_;
	std::vector<std::string> player_colors_;

	//std::vector<std::vector<std::string> > player_leaders_;
	std::vector<leader_list_manager> player_leaders_;
	std::vector<std::string> possible_faction_ids_;

	std::vector<std::string> team_names_;
	std::vector<int> team_indices_;

	gui::scrollpane scroll_pane_;

	std::vector<gui::label> player_numbers_;
	std::vector<gui::combo> combos_type_;
	std::vector<gui::combo> combos_race_;
	std::vector<gui::combo> combos_leader_;
	std::vector<gui::combo> combos_team_;
	std::vector<gui::combo> combos_color_;
	std::vector<gui::slider> sliders_gold_;
	std::vector<gui::label> labels_gold_;

	gui::button ai_;
	gui::button launch_;
	gui::button cancel_;

	gui::label waiting_label_;
	bool message_full_;

	std::deque<config> network_data_;

	const std::string default_controller_;
	const std::string team_prefix_;
};

#endif
