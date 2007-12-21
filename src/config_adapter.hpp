/* $Id$ */
/*
   Copyright (C) 2005 - 2007 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playlevel Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file config_adapter.hpp
//!

#ifndef CONFIG_ADAPTER_HPP_INCLUDED
#define CONFIG_ADAPTER_HPP_INCLUDED


#include <set>
#include <string>
#include <vector>
#include "config.hpp"

class gamestatus;
class game_state;
class game_data;
class gamemap;
class unit_map;
class team;

// This module is responsible for constructing objects like 'team'
// or 'unit' out of config-information, that is based on WML.

std::string get_unique_saveid(const config& cfg, std::set<std::string>& seen_save_ids);
int get_first_human_team(const config::child_list::const_iterator& cfg, const config::child_list& unit_cfg);
void get_player_info(const config& cfg, game_state& gamestate, std::string save_id, std::vector<team>& teams, const config& level, const game_data& gameinfo, gamemap& map, unit_map& units, gamestatus& game_status, bool snapshot, bool replay);
const config* get_theme(const config& game_config, std::string theme_name);

#endif
