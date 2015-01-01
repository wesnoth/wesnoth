/*
   Copyright (C) 2014 - 2015 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "synced_context.hpp"
#include "synced_commands.hpp"

#include "actions/undo.hpp"
#include "ai/manager.hpp"
#include "global.hpp"
#include "config.hpp"
#include "config_assign.hpp"
#include "game_classification.hpp"
#include "replay.hpp"
#include "random_new.hpp"
#include "random_new_synced.hpp"
#include "random_new_deterministic.hpp"
#include "resources.hpp"
#include "synced_checkup.hpp"
#include "game_data.hpp"
#include "network.hpp"
#include "log.hpp"
#include "lua_jailbreak_exception.hpp"
#include "play_controller.hpp"
#include "actions/undo.hpp"
#include "game_end_exceptions.hpp"
#include "seed_rng.hpp"
#include "syncmp_handler.hpp"

#include <boost/lexical_cast.hpp>

#include <cassert>
#include <stdlib.h>
#include <sstream>
#include <iomanip>
static lg::log_domain log_replay("replay");
#define DBG_REPLAY LOG_STREAM(debug, log_replay)
#define LOG_REPLAY LOG_STREAM(info, log_replay)
#define WRN_REPLAY LOG_STREAM(warn, log_replay)
#define ERR_REPLAY LOG_STREAM(err, log_replay)


synced_context::synced_state synced_context::state_ = synced_context::UNSYNCED;
bool synced_context::is_simultaneously_ = false;

bool synced_context::run_in_synced_context(const std::string& commandname, const config& data, bool use_undo, bool show,  bool store_in_replay, synced_command::error_handler_function error_handler)
{
	DBG_REPLAY << "run_in_synced_context:" << commandname << "\n";

	if(!recorder.at_end() && store_in_replay)
	{
		//Most likeley we are in a replay now.
		//We don't want to execute [do_command] & similar now
		WRN_REPLAY << "invalid run_in_synced_context call ignored" << std::endl;
		return false;
	}

	assert(use_undo || (!resources::undo_stack->can_redo() && !resources::undo_stack->can_undo()));
	if(store_in_replay)
	{
		recorder.add_synced_command(commandname, data);
	}
	/*
		use this after recorder.add_synced_command
		because set_scontext_synced sets the checkup to the last added command
	*/
	set_scontext_synced sco;
	synced_command::map::iterator it = synced_command::registry().find(commandname);
	if(it == synced_command::registry().end())
	{
		error_handler("commandname [" +commandname +"] not found", true);
	}
	else
	{
		bool success = it->second(data, use_undo, show, error_handler);
		if(!success && store_in_replay)
		{
			//remove it from replay if we weren't sucessful.
			recorder.undo();
			return false;
		}
	}

	// this might also be a good point to call resources::controller->check_victory();
	// because before for example if someone kills all units during a moveto event they don't loose.
	resources::controller->check_victory();
	DBG_REPLAY << "run_in_synced_context end\n";
	return true;
}


bool synced_context::run_in_synced_context_if_not_already(const std::string& commandname,const config& data, bool use_undo, bool show, synced_command::error_handler_function error_handler)
{
	switch(synced_context::get_synced_state())
	{
	case(synced_context::UNSYNCED):
		return run_in_synced_context(commandname, data, use_undo, show, true, error_handler);
	case(synced_context::LOCAL_CHOICE):
		ERR_REPLAY << "trying to execute action while being in a local_choice" << std::endl;
		//we reject it because such actions usually change the gamestate badly which is not intented during a local_choice.
		return false;
	case(synced_context::SYNCED):
	{
		synced_command::map::iterator it = synced_command::registry().find(commandname);
		if(it == synced_command::registry().end())
		{
			error_handler("commandname [" +commandname +"]not found", true);
			return false;
		}
		else
		{
			return it->second(data, /*use_undo*/ false, show, error_handler);
		}
	}
	default:
		assert(false && "found unknown synced_context::synced_state");
		return false;
	}
}

void synced_context::default_error_function(const std::string& message, bool /*heavy*/)
{
	ERR_REPLAY << "Very strange Error during synced execution " << message;
	assert(false && "Very strange Error during synced execution");
}

void synced_context::just_log_error_function(const std::string& message, bool /*heavy*/)
{
	ERR_REPLAY << "Error during synced execution: "  << message;
}

void synced_context::ignore_error_function(const std::string& message, bool /*heavy*/)
{
	DBG_REPLAY << "Ignored during synced execution: "  << message;
}

synced_context::synced_state synced_context::get_synced_state()
{
	return state_;
}

void synced_context::set_synced_state(synced_state newstate)
{
	state_ = newstate;
}

std::string synced_context::generate_random_seed()
{
	config retv_c = synced_context::ask_server_for_seed();
	config::attribute_value seed_val = retv_c["new_seed"];

	return seed_val.str();
}

bool synced_context::is_simultaneously()
{
	return is_simultaneously_;
}

void  synced_context::reset_is_simultaneously()
{
	is_simultaneously_ = false;
}

bool synced_context::can_undo()
{
	//this method should only works in a synced context.
	assert(get_synced_state() == SYNCED);
	//if we called the rng or if we sended data of this action over the network already, undoing is impossible.
	return (!is_simultaneously_) && (random_new::generator->get_random_calls() == 0);
}
namespace
{
	class lua_network_error : public network::error , public tlua_jailbreak_exception
	{
	public:
		lua_network_error(network::error base)
			: network::error(base), tlua_jailbreak_exception()
		{}
	private:
		IMPLEMENT_LUA_JAILBREAK_EXCEPTION(lua_network_error)
	};
}

void synced_context::pull_remote_user_input()
{
	//we sended data over the network.
	is_simultaneously_ = true;
	//code copied form persist_var, feels strange to call ai::.. functions for something where the ai isn't involved....
	//note that ai::manager::raise_sync_network isn't called by the ai at all anymore (one more reason to put it somehwere else)
	try{
		if(resources::gamedata->phase() == game_data::PLAY || resources::gamedata->phase() == game_data::START)
		{
			//during the prestart/preload event the screen is locked and we shouldn't call user_interact.
			//because that might result in crashs if someone clicks anywhere during screenlock.
			try
			{
				ai::manager::raise_user_interact();
			}
			catch(end_turn_exception&)
			{
				//ignore, since it will be thwown throw again.
			}
		}
		syncmp_registry::pull_remote_choice();
	}
	catch(network::error& err)
	{
		throw lua_network_error(err);
	}

}

void synced_context::send_user_choice()
{
	is_simultaneously_ = true;
	syncmp_registry::send_user_choice();
}

boost::shared_ptr<random_new::rng> synced_context::get_rng_for_action()
{
	const std::string& mode = resources::classification->random_mode;
	if(mode == "deterministic")
	{
		return boost::shared_ptr<random_new::rng>(new random_new::rng_deterministic(resources::gamedata->rng()));
	}
	else
	{
		return boost::shared_ptr<random_new::rng>(new random_new::synced_rng(generate_random_seed));
	}
}

static void send_require_random()
{
	config data;
	config& rr = data.add_child("require_random");
	rr["request_id"] = resources::controller->get_server_request_number();
	network::send_data(data,0);		
}


config synced_context::ask_server_for_seed()
{
	resources::controller->increase_server_request_number();
	std::string name = "random_seed";
	assert(get_synced_state() == synced_context::SYNCED);
	const bool is_mp_game = network::nconnections() != 0;
	bool did_require = false;

	DBG_REPLAY << "ask_server for random_seed\n";
	/*
		as soon as random or similar is involved, undoing is impossible.
	*/
	resources::undo_stack->clear();
	/*
		there might be speak or similar commands in the replay before the user input.
	*/
	while(true) {

		do_replay_handle();
		bool is_replay_end = get_replay_source().at_end();

		if (is_replay_end && !is_mp_game)
		{
			/* The decision is ours, and it will be inserted
			into the replay. */
			DBG_REPLAY << "MP synchronization: local server choice\n";
			config cfg = config_of("new_seed", seed_rng::next_seed_str());
			//-1 for "server" todo: change that.
			recorder.user_input(name, cfg, -1);
			return cfg;

		}
		else if(is_replay_end && is_mp_game)
		{
			DBG_REPLAY << "MP synchronization: remote server choice\n";

			//here we can get into the situation that the decision has already been made but not received yet.
			synced_context::pull_remote_user_input();

			/*
				we don't want to send multiple "require_random" to the server.
			*/
			if(!did_require)
			{	
				send_require_random();
				did_require = true;
			}

			SDL_Delay(10);
			continue;

		}
		else if (!is_replay_end)
		{
			/* The decision has already been made, and must
			be extracted from the replay. */
			DBG_REPLAY << "MP synchronization: replay server choice\n";
			do_replay_handle();
			const config *action = get_replay_source().get_next_action();
			if (!action)
			{
				replay::process_error("[" + name + "] expected but none found\n");
				get_replay_source().revert_action();
				return config_of("new_seed", seed_rng::next_seed_str());
			}
			if (!action->has_child(name))
			{
				replay::process_error("[" + name + "] expected but none found, found instead:\n " + action->debug() + "\n");

				get_replay_source().revert_action();
				return config_of("new_seed", seed_rng::next_seed_str());
			}
			if((*action)["from_side"].str() != "server" || (*action)["side_invalid"].to_bool(false) )
			{
				//we can proceed without getting OOS in this case, but allowing this would allow a "player chan choose their attack results in mp" cheat
				replay::process_error("wrong from_side or side_invalid this could mean someone wants to cheat\n");
			}
			return action->child(name);
		}
	}
}

set_scontext_synced::set_scontext_synced()
	: new_rng_(synced_context::get_rng_for_action()), new_checkup_(recorder.get_last_real_command().child_or_add("checkup")), disabler_()
{
	init();
}

set_scontext_synced::set_scontext_synced(int number)
	: new_rng_(synced_context::get_rng_for_action()), new_checkup_(recorder.get_last_real_command().child_or_add("checkup" + boost::lexical_cast<std::string>(number))), disabler_()
{
	init();
}

/*
	so we dont have to write the same code 3 times.
*/
void set_scontext_synced::init()
{
	LOG_REPLAY << "set_scontext_synced::set_scontext_synced\n";
	assert(synced_context::get_synced_state() == synced_context::UNSYNCED);

	synced_context::set_synced_state(synced_context::SYNCED);
	synced_context::reset_is_simultaneously();

	old_checkup_ = checkup_instance;
	checkup_instance = & new_checkup_;
	old_rng_ = random_new::generator;
	random_new::generator = new_rng_.get();
}
set_scontext_synced::~set_scontext_synced()
{
	LOG_REPLAY << "set_scontext_synced:: destructor\n";
	assert(synced_context::get_synced_state() == synced_context::SYNCED);
	assert(checkup_instance == &new_checkup_);
	config co;
	if(!checkup_instance->local_checkup(config_of("random_calls", new_rng_->get_random_calls()), co))
	{
		//if we really get -999 we have a very serious OOS.
		ERR_REPLAY << "We called random " << new_rng_->get_random_calls() << " times, but the original game called random " << co["random_calls"].to_int(-99) << " times." << std::endl;
		ERR_REPLAY << co.debug() << std::endl;
	}

	random_new::generator = old_rng_;
	synced_context::set_synced_state(synced_context::UNSYNCED);

	checkup_instance = old_checkup_;
}

int set_scontext_synced::get_random_calls()
{
	return new_rng_->get_random_calls();
}


set_scontext_local_choice::set_scontext_local_choice()
{

	assert(synced_context::get_synced_state() == synced_context::SYNCED);
	synced_context::set_synced_state(synced_context::LOCAL_CHOICE);


	old_rng_ = random_new::generator;
	//calling the synced rng form inside a local_choice would cause oos.
	//TODO use a member variable instead if new/delete
	random_new::generator = new random_new::rng();
}
set_scontext_local_choice::~set_scontext_local_choice()
{
	assert(synced_context::get_synced_state() == synced_context::LOCAL_CHOICE);
	synced_context::set_synced_state(synced_context::SYNCED);
	delete random_new::generator;
	random_new::generator = old_rng_;
}

set_scontext_leave_for_draw::set_scontext_leave_for_draw()
	: previous_state_(synced_context::get_synced_state())
{
	if(previous_state_ != synced_context::SYNCED)
	{
		old_rng_= NULL;
		return;
	}
	synced_context::set_synced_state(synced_context::LOCAL_CHOICE);

	assert(random_new::generator);
	old_rng_ = random_new::generator;
	//calling the synced rng form inside a local_choice would cause oos.
	//TODO use a member variable instead if new/delete
	random_new::generator = new random_new::rng();
}
set_scontext_leave_for_draw::~set_scontext_leave_for_draw()
{
	if(previous_state_ != synced_context::SYNCED)
	{
		return;
	}
	assert(old_rng_);
	assert(random_new::generator);
	assert(synced_context::get_synced_state() == synced_context::LOCAL_CHOICE);
	synced_context::set_synced_state(synced_context::SYNCED);
	delete random_new::generator;
	random_new::generator = old_rng_;
}
