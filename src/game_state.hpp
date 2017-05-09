/*
   Copyright (C) 2014 - 2017 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

class config;

#include "filter_context.hpp"
#include "game_board.hpp"
#include "game_data.hpp"
#include "tod_manager.hpp"
#include "units/id.hpp"

class game_display;
class play_controller;
class game_lua_kernel;
class reports;

namespace game_events { class manager; class wmi_container; }
namespace game_events { struct event_context; }

namespace pathfind { class manager; }

namespace wb { class manager; }

namespace actions { class undo_list; }

class game_state : public filter_context
{
private:
	friend class replay_controller;
public:
	game_data gamedata_;
	game_board board_;
	tod_manager tod_manager_;
	std::unique_ptr<pathfind::manager> pathfind_manager_;
	const std::unique_ptr<reports> reports_;
	std::unique_ptr<game_lua_kernel> lua_kernel_;
	const std::unique_ptr<game_events::manager> events_manager_;
	/// undo_stack_ is never nullptr. It is implemented as a pointer so that
	/// undo_list can be an incomplete type at this point (which reduces the
	/// number of files that depend on actions/undo.hpp).
	const std::unique_ptr<actions::undo_list> undo_stack_;
	int player_number_;

	boost::optional<end_level_data> end_level_data_;
	bool init_side_done_;
	bool start_event_fired_;
	// used to sync with the mpserver
	int server_request_number_;
	bool& init_side_done() { return init_side_done_; }


	game_events::wmi_container& get_wml_menu_items();
	const game_events::wmi_container& get_wml_menu_items() const;
	int first_human_team_; //needed to initialize the viewpoint during setup
	bool has_human_sides() const { return first_human_team_ != -1; }

	game_state(const config & level, play_controller &, const ter_data_cache & tdata);
	/// The third parameter is an optimisation.
	game_state(const config & level, play_controller &, game_board& board);

	~game_state();

	void place_sides_in_preferred_locations(const config& level);

	void init(const config& level, play_controller &);

	void set_game_display(game_display *);

	void write(config& cfg) const;

	virtual const display_context & get_disp_context() const { return board_; }
	virtual const tod_manager & get_tod_man() const { return tod_manager_; }
	virtual const game_data * get_game_data() const { return &gamedata_; }
	virtual game_lua_kernel * get_lua_kernel() const { return lua_kernel_.get(); }

	/// Checks to see if a leader at @a leader_loc could recruit somewhere.
	bool can_recruit_from(const map_location& leader_loc, int side) const;
	/// Checks to see if @a leader (assumed a leader) can recruit somewhere.
	/// This takes into account terrain, shroud, and the presence of visible units.
	bool can_recruit_from(const unit& leader) const;

	/// Checks to see if a leader at @a leader_loc could recruit on @a recruit_loc.
	bool can_recruit_on(const map_location& leader_loc, const map_location& recruit_loc, int side) const;
	/// Checks to see if @a leader (assumed a leader) can recruit on @a recruit_loc.
	/// This takes into account terrain, shroud, and whether or not there is already
	/// a visible unit at recruit_loc.
	bool can_recruit_on(const unit& leader, const map_location& recruit_loc) const;

	/// Checks if any of the sides leaders can recruit at a location
	bool side_can_recruit_on(int side, map_location loc) const;
};
