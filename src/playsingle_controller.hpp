/*
   Copyright (C) 2006 - 2018 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playlevel Copyright (C) 2003 by David White <dave@whitevine.net>
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

#include "play_controller.hpp"

#include "cursor.hpp"
#include "playturn_network_adapter.hpp"
#include "playturn.hpp"
#include "replay.hpp"
#include "saved_game.hpp"
#include "replay_controller.hpp"

#include <exception>

struct reset_gamestate_exception : public std::exception
{
	reset_gamestate_exception(std::shared_ptr<config> l, bool s = true) : level(l), start_replay(s) {}
	std::shared_ptr<config> level;
	bool start_replay;
};

class playsingle_controller : public play_controller
{
public:
	playsingle_controller(const config& level, saved_game& state_of_game,
		const config& game_config, const ter_data_cache & tdata, bool skip_replay);

	LEVEL_RESULT play_scenario(const config& level);
	void play_scenario_init();
	void play_scenario_main_loop();

	virtual void handle_generic_event(const std::string& name) override;

	virtual void check_objectives() override;
	virtual void on_not_observer() override {}
	virtual void maybe_linger();

	void end_turn();
	void force_end_turn() override;

	class hotkey_handler;
	std::string describe_result() const;

	bool get_player_type_changed() const { return player_type_changed_; }
	void set_player_type_changed() { player_type_changed_ = true; }
	virtual bool should_return_to_play_side() const override;
	replay_controller * get_replay_controller() { return replay_.get(); }
	bool is_replay() override { return get_replay_controller() != nullptr; }
	void enable_replay(bool is_unit_test = false);
	void on_replay_end(bool is_unit_test);
protected:
	virtual void play_side_impl() override;
	void before_human_turn();
	void show_turn_dialog();
	void execute_gotos();
	virtual void play_human_turn();
	virtual void after_human_turn();
	void end_turn_enable(bool enable);
	void play_ai_turn();
	virtual void play_idle_loop();
	virtual void do_idle_notification();
	virtual void play_network_turn();
	virtual void init_gui() override;

	const cursor::setter cursor_setter_;
	gui::floating_textbox textbox_info_;

	replay_network_sender replay_sender_;
	playturn_network_adapter network_reader_;
	turn_info turn_data_;
	enum END_TURN_STATE
	{
		/// The turn was not ended yet
		END_TURN_NONE,
		/// And endturn was required eigher by the player, by the ai or by [end_turn]
		END_TURN_REQUIRED,
		/// An [end_turn] was added to the replay.
		END_TURN_SYNCED,
	};
	END_TURN_STATE end_turn_;
	bool skip_next_turn_, ai_fallback_;
	std::unique_ptr<replay_controller> replay_;
	void linger();
	void sync_end_turn() override;
	void update_viewing_player() override;
	void reset_replay();
};
