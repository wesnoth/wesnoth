/*
	Copyright (C) 2014 - 2024
	by David White <dave@whitevine.net>
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

#include "game_events/pump.hpp" // for queued_event
#include "mouse_handler_base.hpp"
#include "random.hpp"
#include "synced_checkup.hpp"
#include "synced_commands.hpp"

#include <deque>


// only static methods.
class synced_context
{
public:
	enum synced_state { UNSYNCED, SYNCED, LOCAL_CHOICE };

	/**
	 * Sets the context to 'synced', initialises random context, and calls the given function.
	 * The plan is that in replays and in real game the same function is called.
	 * However, if you cannot call this function you can also use set_scontext_synced directly
	 * (use it like it's used in this method).
	 *
	 * Movement commands are currently treated specially because actions::move_unit returns a
	 * value and some function use that value. Maybe I should add a way to return a value here.
	 *
	 * AI attacks are also treated special because the ai wants to pass advancement_aspects.
	 *
	 * Redoing does normally not take place in a synced context, because we saved the dependent=true
	 * replay commands in the replay stack data. There are also no events of similar fired when
	 * redoing an action (in most cases).
	 *
	 * @param commandname   The command to run.
	 * @param data          The data to use with the command.
	 * @param use_undo      This parameter is used to ignore undos during an ai move to optimize.
	 * @param show
	 * @param error_handler An error handler for the case that data contains invalid data.
	 *
	 * @return              True if the action was successful.
	 */
	static bool run(const std::string& commandname,
		const config& data,
		bool use_undo = true,
		bool show = true,
		synced_command::error_handler_function error_handler = default_error_function);

	static bool run_and_store(const std::string& commandname,
		const config& data,
		bool use_undo = true,
		bool show = true,
		synced_command::error_handler_function error_handler = default_error_function);

	static bool run_and_throw(const std::string& commandname,
		const config& data,
		bool use_undo = true,
		bool show = true,
		synced_command::error_handler_function error_handler = default_error_function);

	/**
	 * Checks whether we are currently running in a synced context, and if not we enters it.
	 * This is never called from so_replay_handle.
	 */
	static bool run_in_synced_context_if_not_already(const std::string& commandname,
		const config& data,
		bool use_undo = true,
		bool show = true,
		synced_command::error_handler_function error_handler = default_error_function);

	/**
	 * @return Whether we are currently executing a synced action like recruit, start, recall, disband, movement,
	 * attack, init_side, end_turn, fire_event, lua_ai, auto_shroud or similar.
	 */
	static synced_state get_synced_state()
	{
		return state_;
	}

	/**
	 * @return Whether we are currently executing a synced action like recruit, start, recall, disband, movement,
	 * attack, init_side, end_turn, fire_event, lua_ai, auto_shroud or similar.
	 */
	static bool is_synced()
	{
		return get_synced_state() == SYNCED;
	}

	/**
	 * @return Whether we are not currently executing a synced action like recruit, start, recall, disband, movement,
	 * attack, init_side, end_turn, fire_event, lua_ai, auto_shroud or similar. and not doing a local choice.
	 */
	static bool is_unsynced()
	{
		return get_synced_state() == UNSYNCED;
	}

	/** Should only be called form set_scontext_synced, set_scontext_local_choice */
	static void set_synced_state(synced_state newstate)
	{
		state_ = newstate;
	}

	/** Generates a new seed for a synced event, by asking the 'server' */
	static std::string generate_random_seed();

	/** called from get_user_choice while waiting for a remove user choice. */
	static void pull_remote_user_input();

	/**
	 * called from get_user_choice to send a recently made choice to the other clients.
	 * Does not receive any data from the network any sends data.
	 */
	static void send_user_choice();

	/** A function to be passed to run_in_synced_context to assert false on error (the default). */
	static void default_error_function(const std::string& message);

	/** A function to be passed to run_in_synced_context to log the error. */
	static void just_log_error_function(const std::string& message);

	/** A function to be passed to run_in_synced_context to ignore the error. */
	static void ignore_error_function(const std::string& message);

	/** @return A rng_deterministic if in determinsic mode otherwise a rng_synced. */
	static std::shared_ptr<randomness::rng> get_rng_for_action();

	/** @return whether we needed data from other clients about the action, in this case we need to send data about the current action to other clients. which means we cannot undo it. */
	static bool is_simultaneous()
	{
		return is_simultaneous_;
	}

	/** Sets is_simultaneous_ = false, called when entering the synced context. */
	static void reset_is_simultaneous()
	{
		is_simultaneous_ = false;
	}

	/** Sets is_simultaneous_ = true, called using a user choice that is not the currently playing side. */
	static void set_is_simultaneous();
	static void block_undo(bool do_block = true);
	static void reset_block_undo()
	{
		is_undo_blocked_ = false;
	}

	/** @return Whether we tracked something that can never be undone. */
	static bool undo_blocked();

	static void set_last_unit_id(int id)
	{
		last_unit_id_ = id;
	}

	static int get_unit_id_diff();

	class server_choice
	{
	public:
		virtual ~server_choice()
		{
		}

		/** We are in a game with no mp server and need to do this choice locally. */
		virtual config local_choice() const = 0;

		/** The request which is sent to the mp server. */
		virtual config request() const = 0;

		virtual const char* name() const = 0;

		int request_id() const;
		void send_request() const;
	};

	/** If we are in a mp game, ask the server, otherwise generate the answer ourselves. */
	static config ask_server_choice(const server_choice&);

	struct event_info {
		config cmds_;
		std::optional<int> lua_;
		game_events::queued_event evt_;
		event_info(const config& cmds, game_events::queued_event evt) : cmds_(cmds), evt_(evt) {}
		event_info(int lua, game_events::queued_event evt) : lua_(lua), evt_(evt) {}
		event_info(int lua, const config& args, game_events::queued_event evt) : cmds_(args), lua_(lua), evt_(evt) {}
	};

	typedef std::deque<event_info> event_list;
	static event_list& get_undo_commands()
	{
		return undo_commands_;
	}

	static void add_undo_commands(const config& commands, const game_events::queued_event& ctx);
	static void add_undo_commands(int fcn_idx, const game_events::queued_event& ctx);
	static void add_undo_commands(int fcn_idx, const config& args, const game_events::queued_event& ctx);

	static void reset_undo_commands()
	{
		undo_commands_.clear();
	}

	static bool ignore_undo();
private:
	/** Weather we are in a synced move, in a user_choice, or none of them. */
	static inline synced_state state_ = synced_context::UNSYNCED;

	/**
	 * As soon as get_user_choice is used with side != current_side (for example in generate_random_seed) other sides
	 * execute the command simultaneously and is_simultaneously is set to true. It's impossible to undo data that has
	 * been sent over the network.
	 *
	 * false = we are on a local turn and haven't sent anything yet.
	 *
	 * TODO: it would be better if the following variable were not static.
	 */
	static inline bool is_simultaneous_ = false;
	static inline bool is_undo_blocked_ = false;

	/** Used to restore the unit id manager when undoing. */
	static inline int last_unit_id_ = 0;

	/** Actions to be executed when the current action is undone. */
	static inline event_list undo_commands_ {};
};

class set_scontext_synced_base
{
public:
	set_scontext_synced_base();
	~set_scontext_synced_base();

protected:
	std::shared_ptr<randomness::rng> new_rng_;
	randomness::rng* old_rng_;
};

/** A RAII object to enter the synced context, cannot be called if we are already in a synced context. */
class set_scontext_synced : set_scontext_synced_base
{
public:
	set_scontext_synced();

	/** Use this constructor if you have multiple synced_context but only one replay entry. */
	set_scontext_synced(int num);

	~set_scontext_synced();

	int get_random_calls();
	void do_final_checkup(bool dont_throw = false);

private:
	// only called by constructors.
	void init();
	static checkup* generate_checkup(const std::string& tagname);
	checkup* old_checkup_;
	const std::unique_ptr<checkup> new_checkup_;
	events::command_disabler disabler_;
	bool did_final_checkup_;
};

/**
 * A RAII object to temporary leave the synced context like in  wesnoth.synchronize_choice.
 * Can only be used from inside a synced context.
 */
class leave_synced_context
{
public:
	leave_synced_context();
	~leave_synced_context();

private:
	randomness::rng* old_rng_;
};

/**
 * An object to leave the synced context during draw or unsynced wml items when we don’t know whether we are in a
 * synced context or not. if we are in a synced context we leave the synced context, otherwise it has no effect. we need
 * This because we might call lua's wesnoth.interface.game_display during draw and we don’t want to have that an effect
 * on the gamestate in this case.
 */
class set_scontext_unsynced
{
public:
	set_scontext_unsynced();

private:
	const std::unique_ptr<leave_synced_context> leaver_;
};
