//* $Id$ */
/*
   Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Maintain status of a game, load&save games.
 */

#include "global.hpp"
#include "config.hpp"

#include "gamestatus.hpp"

#include "actions.hpp"
#include "foreach.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "game_preferences.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "statistics.hpp"
#include "team.hpp"
#include "unit.hpp"
#include "unit_id.hpp"
#include "wesconfig.h"
#include "wml_exception.hpp"
#include "formula_string_utils.hpp"
#include "map.hpp"
#include "pathfind/pathfind.hpp"
#include "whiteboard/side_actions.hpp"

#include <boost/bind.hpp>

#ifndef _MSC_VER
#include <sys/time.h>
#endif

class team_builder {
public:
	team_builder(const config& side_cfg,
		     const std::string &save_id, t_teams& teams,
		     const config& level, gamemap& map, unit_map& units,
				 bool snapshot, const config &starting_pos);

	void build_team_stage_one();
	void build_team_stage_two();

protected:

	int gold_info_ngold_;
	bool gold_info_add_;
	std::deque<config> leader_configs_;
	const config &level_;
	gamemap &map_;
	const config *player_cfg_;
	bool player_exists_;
	const std::string save_id_;
	std::set<std::string> seen_ids_;
	int side_;
	const config &side_cfg_;
	bool snapshot_;
	const config &starting_pos_;
	team *t_;
	t_teams &teams_;
	std::vector<const config*> unit_configs_;
	unit_map &units_;


	void log_step(const char *s) const ;
	void init();
	bool use_player_cfg() const ;
	void gold();
	void new_team();
	void objectives();
	void previous_recruits();
	void handle_unit(const config &u, const char *origin);
	void handle_leader(const config &leader);
	void leader();
	void prepare_units();
	void place_units();
};


