#include "synced_context.hpp"
#include "synced_commands.hpp"

#include "actions/undo.hpp"
#include "ai/manager.hpp"
#include "global.hpp"
#include "config.hpp"
#include "config_assign.hpp"
#include "replay.hpp"
#include "random_new.hpp"
#include "random_new_synced.hpp"
#include "random_new_deterministic.hpp"
#include "resources.hpp"
#include "synced_checkup.hpp"
#include "gamestatus.hpp"
#include "network.hpp"
#include "log.hpp"
#include "play_controller.hpp"
#include "actions/undo.hpp"
#include "game_end_exceptions.hpp"
#include <boost/lexical_cast.hpp>

#include <cassert>
#include <stdlib.h>
static lg::log_domain log_replay("replay");
#define DBG_REPLAY LOG_STREAM(debug, log_replay)
#define LOG_REPLAY LOG_STREAM(info, log_replay)
#define WRN_REPLAY LOG_STREAM(warn, log_replay)
#define ERR_REPLAY LOG_STREAM(err, log_replay)


synced_context::syced_state synced_context::state_ = synced_context::UNSYNCED;
bool synced_context::is_simultaneously_ = false;

bool synced_context::run_in_synced_context(const std::string& commandname, const config& data, bool use_undo, bool show,  bool store_in_replay, synced_command::error_handler_function error_handler)
{
	DBG_REPLAY << "run_in_synced_context:" << commandname << "\n";
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
		error_handler("commandname not found", true);
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

synced_context::syced_state synced_context::get_syced_state()
{
	return state_;
}

void synced_context::set_syced_state(syced_state newstate)
{
	state_ = newstate;
}

int synced_context::generate_random_seed()
{
	random_seed_choice cho;
	config retv_c = synced_context::ask_server("random_seed", cho);
	return retv_c["new_seed"];
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
	assert(get_syced_state() == SYNCED);
	//if we called the rng or if we sended data of this action over the network already, undoing is impossible.
	return (!is_simultaneously_) && (random_new::generator->get_random_calls() == 0);
}

void synced_context::pull_remote_user_input()
{
	//we sended data over the network.
	is_simultaneously_ = true;
	//code copied form persist_var, feels strange to call ai::.. functions for something where the ai isn't involved....
	//note that ai::manager::raise_sync_network isn't called by the ai at all anymore (one more reason to put it somehwere else)
	try
	{
		ai::manager::raise_user_interact();
	}
	catch(end_turn_exception&)
	{
		//ignore, since it will be thwown throw again.
	}
	
	try
	{
		ai::manager::raise_sync_network();
	}
	catch(end_turn_exception&)
	{
		//ignore, since it will be thwown again.
	}

	try
	{
		// in some cases network::receive_data only returns the wanted result on the second try.
		ai::manager::raise_sync_network();
	}
	catch(end_turn_exception&)
	{
		//ignore, since it will throw again.
	}

}


boost::shared_ptr<random_new::rng> synced_context::get_rng_for_action()
{
	/*
		i copied the code for "deterministic_mode" from the code for "difficulty".
		i don't know why we have this information twice, i just did it because we have the information for difficulty twice too.
	*/
	const std::string& v1 = resources::gamedata->random_mode();
	const std::string& v2 = resources::state_of_game->classification().random_mode;
	assert(v1==v2);
	const std::string& mode= v1; // = resources ... gamestate ... get_random_mode()
	if(mode == "deterministic")
	{
		return boost::shared_ptr<random_new::rng>(new random_new::rng_deterministic(resources::gamedata->rng()));
	}
	else
	{
		return boost::shared_ptr<random_new::rng>(new random_new::synced_rng(generate_random_seed));
	}
}

config synced_context::ask_server(const std::string &name, const mp_sync::user_choice &uch)
{
	assert(get_syced_state() == synced_context::SYNCED);
	
	int current_side = resources::controller->current_side();
	int side = current_side;
	bool is_mp_game = network::nconnections() != 0;
	const int max_side  = static_cast<int>(resources::teams->size());
	bool did_require = false;

	
	if ((*resources::teams)[side-1].is_empty())
	{
		/*
			
		*/
		DBG_REPLAY << "MP synchronization: side 1 being null-controlled in get_user_choice.\n";
		side = 1;
		while ( side <= max_side  &&  (*resources::teams)[side-1].is_empty() )
			side++;
		assert(side <= max_side);
	}


	
	assert(1 <= side  && side  <= max_side);
	assert(1 <= current_side  && current_side  <= max_side);

	DBG_REPLAY << "ask_server for :" << name << "\n";
	/*
		as soon as random or similar is involved, undoing is impossible.
	*/
	resources::undo_stack->clear();
	/*
		there might be speak or similar commands in the replay before the user input.
	*/
	while(true){

		do_replay_handle(current_side);
		// the current_side on the server is a lie because it can happen on one client we are already executing side 2 
		bool is_local_side = (*resources::teams)[side-1].is_local();
		bool is_replay_end = get_replay_source().at_end();

		if (is_replay_end && !is_mp_game)
		{
			/* The decision is ours, and it will be inserted
			into the replay. */
			DBG_REPLAY << "MP synchronization: local server choice\n";
			config cfg = uch.query_user(-1);
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
			if(is_local_side && !did_require)
			{
				config data;
				data.add_child("require_random");
				network::send_data(data,0);
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
			do_replay_handle(resources::controller->current_side());
			const config *action = get_replay_source().get_next_action();
			if (!action) 
			{
				replay::process_error("[" + name + "] expected but none found\n");
				return config();
			}
			if (!action->has_child(name)) 
			{
				replay::process_error("[" + name + "] expected but none found, found instead:\n " + action->debug() + "\n");
				return config();
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
	assert(synced_context::get_syced_state() == synced_context::UNSYNCED);
	
	synced_context::set_syced_state(synced_context::SYNCED);
	synced_context::reset_is_simultaneously();

	old_checkup_ = checkup_instance;
	checkup_instance = & new_checkup_;
	old_rng_ = random_new::generator;
	random_new::generator = new_rng_.get();
}
set_scontext_synced::~set_scontext_synced()
{
	LOG_REPLAY << "set_scontext_synced:: destructor\n";
	assert(synced_context::get_syced_state() == synced_context::SYNCED);
	assert(checkup_instance == &new_checkup_);
	config co;
	if(!checkup_instance->local_checkup(config_of("random_calls", new_rng_->get_random_calls()), co))
	{
		//if we really get -999 we have a very serious OOS.
		ERR_REPLAY << "We called random " << new_rng_->get_random_calls() << " times, but the original game called random " << co["random_calls"].to_int(-99) << " times.\n";
		ERR_REPLAY << co.debug() << "\n";
	}

	random_new::generator = old_rng_;
	synced_context::set_syced_state(synced_context::UNSYNCED);
	
	checkup_instance = old_checkup_;
}

int set_scontext_synced::get_random_calls()
{
	return new_rng_->get_random_calls();
}


set_scontext_local_choice::set_scontext_local_choice()
{
	
	assert(synced_context::get_syced_state() == synced_context::SYNCED);
	synced_context::set_syced_state(synced_context::LOCAL_CHOICE);

	
	old_rng_ = random_new::generator;
	//calling the synced rng form inside a local_choice would cause oos.
	//TODO use a member variable instead if new/delete
	random_new::generator = new random_new::rng();
}
set_scontext_local_choice::~set_scontext_local_choice()
{
	assert(synced_context::get_syced_state() == synced_context::LOCAL_CHOICE);
	synced_context::set_syced_state(synced_context::SYNCED);
	delete random_new::generator;
	random_new::generator = old_rng_;
}



random_seed_choice::random_seed_choice()
{

}

random_seed_choice::~random_seed_choice()
{

}

config random_seed_choice::query_user(int /*side*/) const
{
	//getting here means we are in a sp game

	
	config retv;
	retv["new_seed"] = rand();
	return retv;
}
config random_seed_choice::random_choice(int /*side*/) const
{
	//it obviously doesn't make sense to call the uninitialized random generator to generatoe a seed ofr the same random generator;
	//this shoud never happen
	assert(false && "random_seed_choice::random_choice called");

	config retv;
	retv["new_seed"] = 0;
	return retv;
}
