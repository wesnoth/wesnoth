/* $Id: config.hpp 8785 2005-11-08 22:23:03Z jhinrichs $ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef CONFIG_ADAPTER_HPP_INCLUDED
#define CONFIG_ADAPTER_HPP_INCLUDED

#include "global.hpp"

#include <string>
#include <vector>

#include "config.hpp"
#include "gamestatus.hpp"

class unit;

//This module is responsible for constructing objects like 'team'
//or 'unit' out of config-information, that is based on WML.

std::string get_unique_saveid(const config& cfg, std::set<std::string>& seen_save_ids);
int get_first_human_team(const config::child_list::const_iterator& cfg, const config::child_list& unit_cfg);
void get_player_info(const config& cfg, game_state& gamestate, std::string save_id, std::vector<team>& teams, const config& level, const game_data& gameinfo, gamemap& map, std::map<gamemap::location,unit>& units);
const config* get_theme(const config& game_config, std::string theme_name);

#endif
