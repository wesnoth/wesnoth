/*
	Copyright (C) 2014 - 2024
	by Chris Beck <render787@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "ai/manager.hpp"
#include "filter_context.hpp"
#include "game_board.hpp"
#include "game_data.hpp"
#include "tod_manager.hpp"

class game_display;
class play_controller;
class game_lua_kernel;
class reports;

namespace game_events { class manager; class wmi_manager; }

namespace pathfind { class manager; }


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
	ai::manager ai_manager_;
	const std::unique_ptr<game_events::manager> events_manager_;
	/**
	 * undo_stack_ is never nullptr. It is implemented as a pointer so that
	 * undo_list can be an incomplete type at this point (which reduces the
	 * number of files that depend on actions/undo.hpp).
	 */
	const std::unique_ptr<actions::undo_list> undo_stack_;
	int player_number_;
	int next_player_number_;
	/** True if healing should be done at the beginning of the next side turn */
	bool do_healing_;
	bool victory_when_enemies_defeated_;
	bool remove_from_carryover_on_defeat_;

	std::optional<end_level_data> end_level_data_;
	// used to sync with the mpserver
	int server_request_number_;


	game_events::wmi_manager& get_wml_menu_items();
	const game_events::wmi_manager& get_wml_menu_items() const;

	game_state(const config & level, play_controller &);

	~game_state();

	void place_sides_in_preferred_locations(const config& level);

	void init(const config& level, play_controller &);

	void set_game_display(game_display *);

	void write(config& cfg) const;

	/** Inherited from @ref filter_context. */
	virtual const display_context& get_disp_context() const override
	{
		return board_;
	}

	/** Inherited from @ref filter_context. */
	virtual const tod_manager& get_tod_man() const override
	{
		return tod_manager_;
	}

	/** Inherited from @ref filter_context. */
	virtual const game_data* get_game_data() const override
	{
		return &gamedata_;
	}

	/** Inherited from @ref filter_context. */
	virtual game_lua_kernel* get_lua_kernel() const override
	{
		return lua_kernel_.get();
	}


	bool in_phase(game_data::PHASE phase) const
	{
		return gamedata_.phase() == phase;
	}

	template< typename... Arguments >
	bool in_phase(game_data::PHASE phase, Arguments ... args) const
	{
		return in_phase(phase) || in_phase(args...);
	}

	/** Checks to see if a leader at @a leader_loc could recruit somewhere. */
	bool can_recruit_from(const map_location& leader_loc, int side) const;
	/** Checks to see if @a leader (assumed a leader) can recruit somewhere. */
	/** This takes into account terrain, shroud, and the presence of visible units. */
	bool can_recruit_from(const unit& leader) const;

	/** Checks to see if a leader at @a leader_loc could recruit on @a recruit_loc. */
	bool can_recruit_on(const map_location& leader_loc, const map_location& recruit_loc, int side) const;
	/**
	 * Checks to see if @a leader (assumed a leader) can recruit on @a recruit_loc.
	 * This takes into account terrain, shroud, and whether or not there is already
	 * a visible unit at recruit_loc.
	 */
	bool can_recruit_on(const unit& leader, const map_location& recruit_loc) const;

	/** Checks if any of the sides leaders can recruit at a location */
	bool side_can_recruit_on(int side, map_location loc) const;

	/** Checks whether this is not the last scenario (usually of a campaign)*/
	bool has_next_scenario() const;
	/** creates a new side during a game. @todo: maybe add parameters like id etc? */
	void add_side_wml(config cfg);
};
