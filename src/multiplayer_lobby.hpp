/* $Id$ */
/*
   Copyright (C) 
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef MULTIPLAYER_LOBBY_HPP_INCLUDED
#define MULTIPLAYER_LOBBY_HPP_INCLUDED

#include "config.hpp"
#include "display.hpp"
#include "multiplayer_ui.hpp"

#include "widgets/menu.hpp"

// This module controls the multiplayer lobby. A section on the server which
// allows players to chat, create games, and join games.
namespace mp {

class lobby : public ui
{
public:
	lobby(display& d, const config& cfg, chat& c, config& gamelist);

	virtual void process_event();

protected:
	virtual void hide_children(bool hide=true);
	virtual void layout_children(const SDL_Rect& rect);
	virtual void process_network_data(const config& data, const network::connection sock);

	virtual void gamelist_updated(bool silent=true);
private:

	std::vector<bool> game_vacant_slots_;
	std::vector<bool> game_observers_;

	gui::button observe_game_;
	gui::button join_game_;
	gui::button create_game_;
	gui::button quit_game_;

	gui::menu games_menu_;
	int current_game_;
};

}

#endif
