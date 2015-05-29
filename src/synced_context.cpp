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
#include "unit_id.hpp"
#include "whiteboard/manager.hpp"
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
int synced_context::last_unit_id_ = 0;
bool synced_context::is_simultaneously_ = false;

bool synced_context::run(const std::string& commandname, const config& data, bool use_undo, bool show, synced_command::error_handler_function error_handler)
{
	DBG_REPLAY << "run_in_synced_context:" << commandname << "\n";

	assert(use_undo || (!resources::undo_stack->can_redo() && !resources::undo_stack->can_undo()));
	/*
		use this after resources::recorder->add_synced_command
		because set_scontext_synced sets the checkup to the last added command
	*/
	set_scontext_synced sync;
	synced_command::map::iterator it = synced_command::registry().find(commandname);
	if(it == synced_command::registry().end())
	{
		error_handler("commandname [" +commandname +"] not found", true);
	}
	else
	{
		bool success = it->second(data, use_undo, show, error_handler);
		if(!success)
		{
			return false;
		}
	}

	// this might also be a good point to call resources::controller->check_victory();
	// because before for example if someone kills all units during a moveto event they don't loose.
	resources::controller->check_victory();
	sync.do_final_checkup();
	DBG_REPLAY << "run_in_synced_context end\n";
	return true;
}

bool synced_context::run_and_store(const std::string& commandname, const config& data, bool use_undo, bool show, synced_command::error_handler_function error_handler)
{
	assert(resources::recorder->at_end());
	resources::recorder->add_synced_command(commandname, data);
	bool success = run(commandname, data, use_undo, show, error_handler);
	if(!success)
	{
		resources::recorder->undo();
	}
	return success;
}

bool synced_context::run_and_throw(const std::string& commandname, const config& data, bool use_undo, bool show, synced_command::error_handler_function error_handler)
{
	bool success = run_and_store(commandname, data, use_undo, show, error_handler);
	if(success)
	{
		resources::controller->maybe_throw_return_to_play_side();
	}
	return success;
}

bool synced_context::run_in_synced_context_if_not_already(const std::string& commandname,const config& data, bool use_undo, bool show, synced_command::error_handler_function error_handler)
{
	switch(synced_context::get_synced_state())
	{
	case(synced_context::UNSYNCED):
	{
		if(resources::controller->is_replay())
		{
			ERR_REPLAY << "ignored attempt to invoke a synced command during replay\n";
			return false;
		}
		return run_and_throw(commandname, data, use_undo, show, error_handler);
	}
	case(synced_context::LOCAL_CHOICE):
		ERR_REPLAY << "trying to execute action while being in a local_choice" << std::endl;
		//we reject it because such actions usually change the gamestate badly which is not intented during a local_choice.
		//Also we cannot invoke synced commands here, becasue multiple clients might run local choices
		//simultaniously so it could result in invoking different synced commands simultaniously.
		return false;
	case(synced_context::SYNCED):
	{
		synced_command::map::iterator it = synced_command::registry().find(commandname);
		if(it == synced_command::registry().end())
		{
			error_handler("commandname [" +commandname +"] not found", true);
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

void  synced_context::set_is_simultaneously()
{
	is_simultaneously_ = true;
}

bool synced_context::can_undo()
{
	//this method should only works in a synced context.
	assert(get_synced_state() == SYNCED);
	//if we called the rng or if we sended data of this action over the network already, undoing is impossible.
	return (!is_simultaneously_) && (random_new::generator->get_random_calls() == 0);
}

void synced_context::set_last_unit_id(int id)
{
	last_unit_id_ = id;
}

int synced_context::get_unit_id_diff()
{
	//this method only works in a synced context.
	assert(get_synced_state() == SYNCED);
	return n_unit::id_manager::instance().get_save_id() - last_unit_id_;
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
	//code copied form persist_var, feels strange to call ai::.. functions for something where the ai isn't involved....
	//note that ai::manager::raise_sync_network isn't called by the ai at all anymore (one more reason to put it somehwere else)
	try{
		if(resources::gamedata->phase() == game_data::PLAY || resources::gamedata->phase() == game_data::START)
		{
			//during the prestart/preload event the screen is locked and we shouldn't call user_interact.
			//because that might result in crashs if someone clicks anywhere during screenlock.
			ai::manager::raise_user_interact();
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
	assert(is_simultaneously_);
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
	set_is_simultaneously();
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
		bool is_replay_end = resources::recorder->at_end();

		if (is_replay_end && !is_mp_game)
		{
			/* The decision is ours, and it will be inserted
			into the replay. */
			DBG_REPLAY << "MP synchronization: local server choice\n";
			config cfg = config_of("new_seed", seed_rng::next_seed_str());
			//-1 for "server" todo: change that.
			resources::recorder->user_input(name, cfg, -1);
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
			const config *action = resources::recorder->get_next_action();
			if (!action)
			{
				replay::process_error("[" + name + "] expected but none found\n");
				resources::recorder->revert_action();
				return config_of("new_seed", seed_rng::next_seed_str());
			}
			if (!action->has_child(name))
			{
				replay::process_error("[" + name + "] expected but none found, found instead:\n " + action->debug() + "\n");

				resources::recorder->revert_action();
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

set_scontext_synced_base::set_scontext_synced_base()
	: new_rng_(synced_context::get_rng_for_action())
	, old_rng_(random_new::generator)
{
	LOG_REPLAY << "set_scontext_synced_base::set_scontext_synced_base\n";
	assert(!resources::whiteboard->has_planned_unit_map());
	assert(synced_context::get_synced_state() == synced_context::UNSYNCED);
	synced_context::set_synced_state(synced_context::SYNCED);
	synced_context::reset_is_simultaneously();
	synced_context::set_last_unit_id(n_unit::id_manager::instance().get_save_id());
	old_rng_ = random_new::generator;
	random_new::generator = new_rng_.get();
}
set_scontext_synced_base::~set_scontext_synced_base()
{
	LOG_REPLAY << "set_scontext_synced_base:: destructor\n";
	assert(synced_context::get_synced_state() == synced_context::SYNCED);
	random_new::generator = old_rng_;
	synced_context::set_synced_state(synced_context::UNSYNCED);
}

set_scontext_synced::set_scontext_synced()
	: set_scontext_synced_base()
	, new_checkup_(generate_checkup("checkup")), disabler_()
{
	init();
}

set_scontext_synced::set_scontext_synced(int number)
	: set_scontext_synced_base()
	, new_checkup_(generate_checkup("checkup" + boost::lexical_cast<std::string>(number))), disabler_()
{
	init();
}

checkup* set_scontext_synced::generate_checkup(const std::string& tagname)
{
	if(resources::classification->oos_debug)
	{
		return new mp_debug_checkup();
	}
	else
	{
		return new synced_checkup(resources::recorder->get_last_real_command().child_or_add(tagname));
	}
}

/*
	so we dont have to write the same code 3 times.
*/
void set_scontext_synced::init()
{
	LOG_REPLAY << "set_scontext_synced::set_scontext_synced\n";
	did_final_checkup_ = false;
	old_checkup_ = checkup_instance;
	checkup_instance = &*new_checkup_;
}

void set_scontext_synced::do_final_checkup(bool dont_throw)
{
	assert(!did_final_checkup_);
	std::stringstream msg;
	config co;
	config cn = config_of
		("random_calls", new_rng_->get_random_calls())
		("next_unit_id", n_unit::id_manager::instance().get_save_id() + 1);
	if(checkup_instance->local_checkup(cn, co))
	{
		return;
	}
	if(co["random_calls"].empty())
	{
		msg << "cannot find random_calls check in replay" << std::endl;
	}
	else if(co["random_calls"] != cn["random_calls"])
	{
		msg << "We called random " << new_rng_->get_random_calls() << " times, but the original game called random " << co["random_calls"].to_int() << " times." << std::endl;
	}
	//Ignore empty next_unit_id to prevent false positives with older saves.
	if(!co["next_unit_id"].empty() && co["next_unit_id"] != cn["next_unit_id"])
	{
		msg << "Our next unit id is " << cn["next_unit_id"].to_int() << " but during the original the next unit id was " << co["next_unit_id"].to_int() << std::endl;
	}
	if(!msg.str().empty())
	{
		msg << co.debug() << std::endl;
		if(dont_throw)
		{
			ERR_REPLAY << msg.str() << std::flush;
		}
		else
		{
			replay::process_error(msg.str());
		}
	}
	did_final_checkup_ = true;
}

set_scontext_synced::~set_scontext_synced()
{
	LOG_REPLAY << "set_scontext_synced:: destructor\n";
	assert(checkup_instance == &*new_checkup_);
	if(!did_final_checkup_)
	{
		//do_final_checkup(true);
	}
	checkup_instance = old_checkup_;
}

int set_scontext_synced::get_random_calls()
{
	return new_rng_->get_random_calls();
}


set_scontext_local_choice::set_scontext_local_choice()
{
	//TODO: should we also reset the synced checkup?
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
