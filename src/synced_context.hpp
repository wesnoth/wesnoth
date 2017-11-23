/*
   Copyright (C) 2014 - 2017 by David White <dave@whitevine.net>
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

#include "utils/functional.hpp"
#include "synced_commands.hpp"
#include "synced_checkup.hpp"
#include "replay.hpp"
#include "random.hpp"
#include "random_synced.hpp"
#include "game_events/pump.hpp" // for queued_event
#include "generic_event.hpp"
#include "mouse_handler_base.hpp"
#include <deque>

class config;

//only static methods.
class synced_context
{
public:
	enum synced_state
	{
		UNSYNCED,
		SYNCED,
		LOCAL_CHOICE
	};
	/**

		Sets the context to 'synced', initialises random context, and calls the given function.
		The plan is that in replay and in real game the same function is called.
		however, if you cannot call this function you can also use set_scontext_synced directly (use it like it's used in this method).

		movement commands are currently treated specially,
			thats because actions::move_unit returns a value and some function use that value.
			maybe i should add a way here to return a value.

		ai's attacks are also treated special because the ai wants to pass advancement_aspects.


		redoing does normaly not take place in a synced context, because we saved the dependent=true replaycommands in the replaystack data.
			also there are no events of similar fired when redoing an action (in most cases).

		@param use_undo this parameter is used to ignore undos during an ai move to optimize.
		@param error_handler an error handler for the case that data contains invalid data.

		@return true if the action was successful.



	 */
	static bool run(const std::string& commandname, const config& data, bool use_undo = true, bool show = true, synced_command::error_handler_function error_handler = default_error_function);
	static bool run_and_store(const std::string& commandname, const config& data, bool use_undo = true, bool show = true, synced_command::error_handler_function error_handler = default_error_function);
	static bool run_and_throw(const std::string& commandname, const config& data, bool use_undo = true, bool show = true, synced_command::error_handler_function error_handler = default_error_function);
	/**
		checks whether we are currently running in a synced context, and if not we enters it.
		this is never called from so_replay_handle.
	*/
	static bool run_in_synced_context_if_not_already(const std::string& commandname,const config& data, bool use_undo = true, bool show = true , synced_command::error_handler_function error_handler = default_error_function);
	/**
		@return whether we are currently executing a synced action like recruit, start, recall, disband, movement, attack, init_side, end_turn, fire_event, lua_ai, auto_shroud or similar.
	*/
	static synced_state get_synced_state();
	/**
		@return whether we are currently executing a synced action like recruit, start, recall, disband, movement, attack, init_side, end_turn, fire_event, lua_ai, auto_shroud or similar.
	*/
	static bool is_synced();
	/*
		should only be called form set_scontext_synced, set_scontext_local_choice
	*/
	static void set_synced_state(synced_state newstate);
	/*
		Generates a new seed for a synced event, by asking the 'server'
	*/
	static std::string generate_random_seed();
	/**
		called from get_user_choice while waiting for a remove user choice.
	*/
	static void pull_remote_user_input();
	/**
		called from get_user_choice to send a recently made choice to the other clients.
		Does not receive any data from the network any sends data.
	*/
	static void send_user_choice();
	/**
		a function to be passed to run_in_synced_context to assert false on error (the default).
	*/
	static void default_error_function(const std::string& message, bool heavy);
	/**
		a function to be passed to run_in_synced_context to log the error.
	*/
	static void just_log_error_function(const std::string& message, bool heavy);
	/**
		a function to be passed to run_in_synced_context to ignore the error.
	*/
	static void ignore_error_function(const std::string& message, bool heavy);
	/**
		@return a rng_deterministic if in determinsic mode otherwise a rng_synced.
	*/
	static std::shared_ptr<randomness::rng> get_rng_for_action();
	/**
		@return whether we already sent data about the current action to other clients. which means we cannot undo it.
		returns is_simultaneously_
	*/
	static bool is_simultaneously();
	/*
		sets is_simultaneously_ = false, called when entering the synced context.
	*/
	static void reset_is_simultaneously();
	/*
		sets is_simultaneously_ = true, called using a user choice that is not the currently plaing side.
	*/
	static void set_is_simultaneously();
	/**
		@return whether there were recently no methods called that prevent undoing.
	*/
	static bool can_undo();
	static void set_last_unit_id(int id);
	static int get_unit_id_diff();

	class server_choice
	{
	public:
		virtual ~server_choice(){}
		/// We are in a game with no mp server and need to do this choice locally
		virtual config local_choice() const = 0;
		/// the request which is sent to the mp server.
		virtual config request() const = 0;
		virtual const char* name() const = 0;
		void send_request() const;
	};
	/*
		if we are in a mp game, ask the server, otherwise generate the answer ourselves.
	*/
	static config ask_server_choice(const server_choice&);

	typedef std::deque<std::pair<config,game_events::queued_event>> event_list;
	static event_list& get_undo_commands() { return undo_commands_; }
	static void add_undo_commands(const config& commands, const game_events::queued_event& ctx);
	static void reset_undo_commands();
private:
	/*
		weather we are in a synced move, in a user_choice, or none of them
	*/
	static synced_state state_;
	/*
		As soon as get_user_choice is used with side != current_side (for example in generate_random_seed) other sides execute the command simultaneously and is_simultaneously is set to true.
		It's impossible to undo data that has been sent over the network.

		false = we are on a local turn and haven't sent anything yet.

		TODO: it would be better if the following variable were not static.
	*/
	static bool is_simultaneously_;
	/**
		Used to restore the unit id manager when undoing.
	*/
	static int last_unit_id_;
	/**
		Actions wml to be executed when the current action is undone.
	*/
	static event_list undo_commands_;
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
/*
	a RAII object to enter the synced context, cannot be called if we are already in a synced context.
*/
class set_scontext_synced : set_scontext_synced_base
{
public:
	set_scontext_synced();
	/*
		use this contructor if you have multiple synced_context but only one replay entry.
	*/
	set_scontext_synced(int num);
	~set_scontext_synced();
	int get_random_calls();
	void do_final_checkup(bool dont_throw = false);
private:
	//only called by contructors.
	void init();
	static checkup* generate_checkup(const std::string& tagname);
	checkup* old_checkup_;
	const std::unique_ptr<checkup> new_checkup_;
	events::command_disabler disabler_;
	bool did_final_checkup_;
};

/*
	a RAII object to temporary leave the synced context like in  wesnoth.synchronize_choice. Can only be used from inside a synced context.
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
	an object to leave the synced context during draw or unsynced wml items when we don’t know whether we are in a synced context or not.
	if we are in a synced context we leave the synced context, otherwise it has no effect.
	we need this because we might call lua's wesnoth.theme_items during draw and we don’t want to have that an effect on the gamestate in this case.
*/
class set_scontext_unsynced
{
public:
	set_scontext_unsynced();
private:
	const std::unique_ptr<leave_synced_context> leaver_;
};
