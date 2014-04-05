
/*
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef SYNCED_CONTEXT_H_INCLUDED
#define SYNCED_CONTEXT_H_INCLUDED

#include <boost/function.hpp>
#include "synced_commands.hpp"
#include "synced_checkup.hpp"
#include "replay.hpp"
#include "random_new.hpp"
#include "random_new_synced.hpp"
#include "generic_event.hpp"
#include <boost/shared_ptr.hpp>
class config;

//only static methods.
class synced_context
{
public:
	enum syced_state {UNSYNCED, SYNCED, LOCAL_CHOICE};

	typedef boost::function1<void, config&> ftype;
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
		
		TODO: implement the optional "Deterministic Mode" for MP to lower network traffic.
		
		TODO: move_unit currently ignores when the unit moves further than it can, 
			it would be good to give an oos in this case.
		
		TODO: undos are currently recorded AFTER the action takes place, 
			that means you cannot disallow undoing an action during it's execution by calling undo_stack->clear();
			it would make things easier if that was possible.
		
		
		@param use_undo this parameter is used to ignore undos during an ai move to optimize.
		@param store_in_replay only true if called by do_replay_handle
		@param error_handler an error handler for the case that data contains invalid data.
		
		@return true if the action was successful.

		

	 */
	static bool run_in_synced_context(const std::string& commandname,const config& data, bool use_undo = true, bool show = true ,  bool store_in_replay = true , synced_command::error_handler_function error_handler = default_error_function);
	/*
		Returns whether we are currently executing a synced action like recruit, start, recall, disband, movement, attack, init_side, end_turn, fire_event, lua_ai, auto_shroud or similar.
	*/
	static syced_state get_syced_state();
	/*
		should only be called form set_scontext_synced, set_scontext_local_choice
	*/
	static void set_syced_state(syced_state newstate);
	/*
		Generates a new seed for a synced event, by asking the 'server'
	*/
	static int generate_random_seed();
	/*
		called from get_user_choice; 
	*/
	static void pull_remote_user_input();
	/*
		a function to be passed to run_in_synced_context to assert false on error (the default).
	*/
	static void default_error_function(const std::string& message, bool heavy);
	/*
		a function to be passed to run_in_synced_context to log the error.
	*/
	static void just_log_error_function(const std::string& message, bool heavy);
	/*
		a function to be passed to run_in_synced_context to ignore the error.
	*/
	static void ignore_error_function(const std::string& message, bool heavy);
	/*
		returns a rng_deterministic if in determinsic mode otherwise a rng_synced.
	*/
	static boost::shared_ptr<random_new::rng> get_rng_for(const std::string& commandname);
	/*
		returns is_simultaneously_
	*/
	static bool is_simultaneously();
	/*
		sets is_simultaneously_ = false, called when entering the synced context.
	*/
	static void reset_is_simultaneously();
private:
	/*
		similar to get_user_choice but asks the server instead of a user.
	*/
	static config ask_server(const std::string &name, const mp_sync::user_choice &uch);
	/*
		weather we are in a synced move, in a user_choice, or none of them
	*/
	static syced_state state_;
	/*
		As soon as get_user_choice is used with side != current_side (for example in generate_random_seed) other sides execute the command simultaneously and is_simultaneously is set to true.
		It's impossible to undo data that has been sended over the network.
		
		false = we are on a local turn and haven't sended anything yet.
	*/
	static bool is_simultaneously_;
	/*
		TODO: replace ai::manager::raise_sync_network with this event because ai::manager::raise_sync_network has nothing to do with ai anymore.
	*/
	static events::generic_event remote_user_input_required_;
};

/*
	a RAII object to enter the synced context, cannot be called if we are already in a synced context.
*/
class set_scontext_synced
{
public:
	set_scontext_synced(const std::string& commanname);
	set_scontext_synced();
	/*
		use this if you have multiple synced_context but only one replay entry.
	*/
	set_scontext_synced(int num);
	~set_scontext_synced();
	int get_random_calls();
private:
	//only called by contructors.
	void init();
	random_new::rng* old_rng_;
	boost::shared_ptr<random_new::rng> new_rng_;
	checkup* old_checkup_;
	synced_checkup new_checkup_;
};

/*
	a RAII object to temporary leave the synced context like in  wesnoth.synchronize_choice. Can only be used from inside a synced context.
*/

class set_scontext_local_choice
{
public:
	set_scontext_local_choice();
	~set_scontext_local_choice();
private:
	random_new::rng* old_rng_;
};

/*
	a helper object to server random seed generation.
*/
class random_seed_choice : public mp_sync::user_choice
{
public: 
	random_seed_choice();
	virtual ~random_seed_choice();
	virtual config query_user() const;
	virtual config random_choice() const;
};


#endif
