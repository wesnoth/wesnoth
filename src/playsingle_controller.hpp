/*
	Copyright (C) 2006 - 2022
	by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
	Copyright (C) 2003 by David White <dave@whitevine.net>
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

#include "play_controller.hpp"

#include "cursor.hpp"
#include "lua_jailbreak_exception.hpp"
#include "playturn_network_adapter.hpp"
#include "playturn.hpp"
#include "replay.hpp"

#include <exception>

class replay_controller;
class saved_game;

struct reset_gamestate_exception : public lua_jailbreak_exception, public std::exception
{
	reset_gamestate_exception(std::shared_ptr<config> l, std::shared_ptr<config> stats, bool s = true) : level(l), stats_(stats), start_replay(s) {}
	std::shared_ptr<config> level;
	std::shared_ptr<config> stats_;
	bool start_replay;
	const char * what() const noexcept { return "reset_gamestate_exception"; }
private:

	IMPLEMENT_LUA_JAILBREAK_EXCEPTION(reset_gamestate_exception)
};

class playsingle_controller : public play_controller
{
public:
	playsingle_controller(const config& level, saved_game& state_of_game, bool skip_replay);

	~playsingle_controller();
	level_result::type play_scenario(const config& level);
	void play_scenario_init(const config& level);
	void skip_empty_sides(int& side_num);
	void play_some();
	void finish_side_turn();
	void do_end_level();
	void play_scenario_main_loop();

	virtual void handle_generic_event(const std::string& name) override;

	virtual void check_objectives() override;
	virtual void on_not_observer() override {}
	virtual bool is_host() const { return true; }
	virtual void maybe_linger();

	void end_turn();
	void force_end_turn() override;
	void require_end_turn();

	class hotkey_handler;
	std::string describe_result() const;

	bool get_player_type_changed() const { return player_type_changed_; }
	void set_player_type_changed() { player_type_changed_ = true; }
	virtual bool should_return_to_play_side() const override;
	replay_controller * get_replay_controller() const override { return replay_controller_.get(); }
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

	/// Helper to send our actions to the server
	/// Used by turn_data_
	replay_network_sender replay_sender_;
	/// Used by turn_data_
	playturn_network_adapter network_reader_;
	/// Helper to read and execute (in particular replay data/ user actions ) messsages from the server
	turn_info turn_data_;
	/// true iff the user has pressed the end turn button this turn.
	/// (or wants to end linger mode, which is implemented via the same button)
	bool end_turn_requested_;
	/// true when the current side is actually an ai side but was taken over by a human (usually for debugging purposes),
	/// we need this variable to remember to give the ai control back next turn.
	bool ai_fallback_;
	/// non-null when replay mode in active, is used in singleplayer and for the "back to turn" feature in multiplayer.
	std::unique_ptr<replay_controller> replay_controller_;
	void linger();
	void update_gui_linger();
	void sync_end_turn() override;
	void update_viewing_player() override;
	void reset_replay();
};
