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

#include "synced_context.hpp"
#include "synced_commands.hpp"

#include "actions/undo.hpp"
#include "config.hpp"
#include "game_board.hpp"
#include "game_classification.hpp"
#include "game_data.hpp"
#include "log.hpp"
#include "play_controller.hpp"
#include "random.hpp"
#include "random_deterministic.hpp"
#include "random_synced.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "seed_rng.hpp"
#include "synced_checkup.hpp"
#include "syncmp_handler.hpp"
#include "units/id.hpp"
#include "whiteboard/manager.hpp"

#include <cassert>
#include <sstream>

static lg::log_domain log_replay("replay");
#define DBG_REPLAY LOG_STREAM(debug, log_replay)
#define LOG_REPLAY LOG_STREAM(info, log_replay)
#define WRN_REPLAY LOG_STREAM(warn, log_replay)
#define ERR_REPLAY LOG_STREAM(err, log_replay)

bool synced_context::run(const std::string& commandname,
	const config& data,
	bool use_undo,
	bool show,
	synced_command::error_handler_function error_handler)
{
	DBG_REPLAY << "run_in_synced_context:" << commandname;

	assert(use_undo || (!resources::undo_stack->can_redo() && !resources::undo_stack->can_undo()));

	// use this after resources::recorder->add_synced_command
	// because set_scontext_synced sets the checkup to the last added command
	set_scontext_synced sync;

	synced_command::map::iterator it = synced_command::registry().find(commandname);
	if(it == synced_command::registry().end()) {
		error_handler("commandname [" + commandname + "] not found");
	} else {
		bool success = it->second(data, use_undo, show, error_handler);
		if(!success) {
			return false;
		}
	}

	resources::controller->check_victory();

	sync.do_final_checkup();

	// TODO: It would be nice if this could automaticially detect that
	//       no entry was pushed to the undo stack for this action
	//       and always clear the undo stack in that case.
	if(undo_blocked()) {
		// This in particular helps the networking code to make sure this command is sent.
		resources::undo_stack->clear();
		send_user_choice();
	}

	DBG_REPLAY << "run_in_synced_context end";
	return true;
}

bool synced_context::run_and_store(const std::string& commandname,
	const config& data,
	bool use_undo,
	bool show,
	synced_command::error_handler_function error_handler)
{
	if(resources::controller->is_replay()) {
		ERR_REPLAY << "ignored attempt to invoke a synced command during replay";
		return false;
	}

	assert(resources::recorder->at_end());
	resources::recorder->add_synced_command(commandname, data);
	bool success = run(commandname, data, use_undo, show, error_handler);
	if(!success) {
		resources::recorder->undo();
	}

	return success;
}

bool synced_context::run_and_throw(const std::string& commandname,
	const config& data,
	bool use_undo,
	bool show,
	synced_command::error_handler_function error_handler)
{
	bool success = run_and_store(commandname, data, use_undo, show, error_handler);
	if(success) {
		resources::controller->maybe_throw_return_to_play_side();
	}

	return success;
}

bool synced_context::run_in_synced_context_if_not_already(const std::string& commandname,
	const config& data,
	bool use_undo,
	bool show,
	synced_command::error_handler_function error_handler)
{
	switch(synced_context::get_synced_state()) {
	case(synced_context::UNSYNCED): {
		return run_and_throw(commandname, data, use_undo, show, error_handler);
	}
	case(synced_context::LOCAL_CHOICE):
		ERR_REPLAY << "trying to execute action while being in a local_choice";
		// we reject it because such actions usually change the gamestate badly which is not intended during a
		// local_choice. Also we cannot invoke synced commands here, because multiple clients might run local choices
		// simultaneously so it could result in invoking different synced commands simultaneously.
		return false;
	case(synced_context::SYNCED): {
		synced_command::map::iterator it = synced_command::registry().find(commandname);
		if(it == synced_command::registry().end()) {
			error_handler("commandname [" + commandname + "] not found");
			return false;
		} else {
			return it->second(data, /*use_undo*/ false, show, error_handler);
		}
	}
	default:
		assert(false && "found unknown synced_context::synced_state");
		return false;
	}
}

void synced_context::default_error_function(const std::string& message)
{
	ERR_REPLAY << "Unexpected Error during synced execution" << message;
	assert(!"Unexpected Error during synced execution, more info in stderr.");
}

void synced_context::just_log_error_function(const std::string& message)
{
	ERR_REPLAY << "Error during synced execution: " << message;
}

void synced_context::ignore_error_function(const std::string& message)
{
	DBG_REPLAY << "Ignored during synced execution: " << message;
}

namespace
{
class random_server_choice : public synced_context::server_choice
{
public:
	/** We are in a game with no mp server and need to do this choice locally. */
	virtual config local_choice() const override
	{
		return config{"new_seed", seed_rng::next_seed_str()};
	}

	/** The request which is sent to the mp server. */
	virtual config request() const override
	{
		return config();
	}

	virtual const char* name() const override
	{
		return "random_seed";
	}
};
} // namespace

std::string synced_context::generate_random_seed()
{
	config retv_c = synced_context::ask_server_choice(random_server_choice());
	config::attribute_value seed_val = retv_c["new_seed"];

	return seed_val.str();
}

void synced_context::set_is_simultaneous()
{
	resources::undo_stack->clear();
	is_simultaneous_ = true;
}

bool synced_context::undo_blocked()
{
	// this method should only works in a synced context.
	assert(!is_unsynced());
	// if we sent data of this action over the network already, undoing is blocked.
	// if we called the rng, undoing is blocked.
	// if the game has ended, undoing is blocked.
	// if the turn has ended undoing is blocked.
	return is_simultaneous_
	    || (randomness::generator->get_random_calls() != 0)
	    || resources::controller->is_regular_game_end()
	    || resources::gamedata->end_turn_forced();
}

int synced_context::get_unit_id_diff()
{
	// this method only works in a synced context.
	assert(is_synced());
	return resources::gameboard->unit_id_manager().get_save_id() - last_unit_id_;
}

void synced_context::pull_remote_user_input()
{
	resources::controller->receive_actions();
}

// TODO: this is now also used for normal actions, maybe it should be renamed.
void synced_context::send_user_choice()
{
	assert(undo_blocked());
	resources::controller->send_actions();
}

std::shared_ptr<randomness::rng> synced_context::get_rng_for_action()
{
	const std::string& mode = resources::classification->random_mode;
	if(mode == "deterministic" || mode == "biased") {
		return std::make_shared<randomness::rng_deterministic>(resources::gamedata->rng());
	} else {
		return std::make_shared<randomness::synced_rng>(generate_random_seed);
	}
}

int synced_context::server_choice::request_id() const
{
	return resources::controller->get_server_request_number();
}

void synced_context::server_choice::send_request() const
{
	resources::controller->send_to_wesnothd(config {
		"request_choice", config {
			"request_id", request_id(),
			name(), request(),
		},
	});
}

config synced_context::ask_server_choice(const server_choice& sch)
{
	if(!is_synced()) {
		ERR_REPLAY << "Trying to ask the server for a '" << sch.name()
				   << "' choice in a unsynced context, doing the choice locally. This can cause OOS.";
		return sch.local_choice();
	}

	set_is_simultaneous();
	resources::controller->increase_server_request_number();
	const bool is_mp_game = resources::controller->is_networked_mp();
	bool did_require = false;

	DBG_REPLAY << "ask_server for random_seed";

	// As soon as random or similar is involved, undoing is impossible.
	resources::undo_stack->clear();

	// There might be speak or similar commands in the replay before the user input.
	while(true) {
		do_replay_handle();
		bool is_replay_end = resources::recorder->at_end();

		if(is_replay_end && !is_mp_game) {
			// The decision is ours, and it will be inserted into the replay.
			DBG_REPLAY << "MP synchronization: local server choice";
			leave_synced_context sync;
			config cfg = sch.local_choice();
			cfg["request_id"] = sch.request_id();
			//-1 for "server" todo: change that.
			resources::recorder->user_input(sch.name(), cfg, -1);
			return cfg;

		} else if(is_replay_end && is_mp_game) {
			DBG_REPLAY << "MP synchronization: remote server choice";

			// Here we can get into the situation that the decision has already been made but not received yet.
			synced_context::pull_remote_user_input();

			// FIXME: we should call play_controller::play_silce or the application will freeze while waiting for a
			// remote choice.
			resources::controller->play_slice();

			// We don't want to send multiple "require_random" to the server.
			if(!did_require) {
				sch.send_request();
				did_require = true;
			}

			SDL_Delay(10);
			continue;

		} else if(!is_replay_end) {
			// The decision has already been made, and must be extracted from the replay.
			DBG_REPLAY << "MP synchronization: replay server choice";
			do_replay_handle();

			const config* action = resources::recorder->get_next_action();
			if(!action) {
				replay::process_error("[" + std::string(sch.name()) + "] expected but none found\n");
				resources::recorder->revert_action();
				return sch.local_choice();
			}

			if(!action->has_child(sch.name())) {
				replay::process_error("[" + std::string(sch.name()) + "] expected but none found, found instead:\n "
									  + action->debug() + "\n");

				resources::recorder->revert_action();
				return sch.local_choice();
			}

			if((*action)["from_side"].str() != "server" || (*action)["side_invalid"].to_bool(false)) {
				// we can proceed without getting OOS in this case, but allowing this would allow a "player chan choose
				// their attack results in mp" cheat
				replay::process_error("wrong from_side or side_invalid this could mean someone wants to cheat\n");
			}

			config res = action->mandatory_child(sch.name());
			if(res["request_id"] != sch.request_id()) {
				WRN_REPLAY << "Unexpected request_id: " << res["request_id"] << " expected: " <<  sch.request_id();
			}
			return res;
		}
	}
}

void synced_context::add_undo_commands(const config& commands, const game_events::queued_event& ctx)
{
	undo_commands_.emplace_front(commands, ctx);
}

void synced_context::add_undo_commands(int idx, const game_events::queued_event& ctx)
{
	undo_commands_.emplace_front(idx, ctx);
}

void synced_context::add_undo_commands(int idx, const config& args, const game_events::queued_event& ctx)
{
	undo_commands_.emplace_front(idx, args, ctx);
}

bool synced_context::ignore_undo()
{
	auto& ct = resources::controller->current_team();
	// Ai doesn't undo stuff, disabling the undo stack allows us to send moves to other clients sooner.
	return ct.is_ai() && ct.auto_shroud_updates();
}

set_scontext_synced_base::set_scontext_synced_base()
	: new_rng_(synced_context::get_rng_for_action())
	, old_rng_(randomness::generator)
{
	LOG_REPLAY << "set_scontext_synced_base::set_scontext_synced_base";

	assert(!resources::whiteboard->has_planned_unit_map());
	assert(synced_context::get_synced_state() == synced_context::UNSYNCED);

	synced_context::set_synced_state(synced_context::SYNCED);
	synced_context::reset_is_simultaneous();
	synced_context::set_last_unit_id(resources::gameboard->unit_id_manager().get_save_id());
	synced_context::reset_undo_commands();

	old_rng_ = randomness::generator;
	randomness::generator = new_rng_.get();
}

set_scontext_synced_base::~set_scontext_synced_base()
{
	LOG_REPLAY << "set_scontext_synced_base:: destructor";
	assert(synced_context::get_synced_state() == synced_context::SYNCED);
	randomness::generator = old_rng_;
	synced_context::set_synced_state(synced_context::UNSYNCED);
}

set_scontext_synced::set_scontext_synced()
	: set_scontext_synced_base()
	, new_checkup_(generate_checkup("checkup"))
	, disabler_()
{
	init();
}

set_scontext_synced::set_scontext_synced(int number)
	: set_scontext_synced_base()
	, new_checkup_(generate_checkup("checkup" + std::to_string(number)))
	, disabler_()
{
	init();
}

checkup* set_scontext_synced::generate_checkup(const std::string& tagname)
{
	if(resources::classification->oos_debug) {
		return new mp_debug_checkup();
	} else {
		return new synced_checkup(resources::recorder->get_last_real_command().child_or_add(tagname));
	}
}

/*
	so we don't have to write the same code 3 times.
*/
void set_scontext_synced::init()
{
	LOG_REPLAY << "set_scontext_synced::set_scontext_synced";
	did_final_checkup_ = false;
	old_checkup_ = checkup_instance;
	checkup_instance = &*new_checkup_;
}

void set_scontext_synced::do_final_checkup(bool dont_throw)
{
	assert(!did_final_checkup_);
	std::stringstream msg;
	config co;
	config cn {
		"random_calls", new_rng_->get_random_calls(),
		"next_unit_id", resources::gameboard->unit_id_manager().get_save_id() + 1,
	};

	if(checkup_instance->local_checkup(cn, co)) {
		return;
	}

	if(co["random_calls"].empty()) {
		msg << "cannot find random_calls check in replay" << std::endl;
	} else if(co["random_calls"] != cn["random_calls"]) {
		msg << "We called random " << new_rng_->get_random_calls() << " times, but the original game called random "
			<< co["random_calls"].to_int() << " times." << std::endl;
	}

	// Ignore empty next_unit_id to prevent false positives with older saves.
	if(!co["next_unit_id"].empty() && co["next_unit_id"] != cn["next_unit_id"]) {
		msg << "Our next unit id is " << cn["next_unit_id"].to_int() << " but during the original the next unit id was "
			<< co["next_unit_id"].to_int() << std::endl;
	}

	if(!msg.str().empty()) {
		msg << co.debug() << std::endl;
		if(dont_throw) {
			ERR_REPLAY << msg.str();
		} else {
			replay::process_error(msg.str());
		}
	}

	did_final_checkup_ = true;
}

set_scontext_synced::~set_scontext_synced()
{
	LOG_REPLAY << "set_scontext_synced:: destructor";
	assert(checkup_instance == &*new_checkup_);
	if(!did_final_checkup_) {
		// do_final_checkup(true);
	}
	checkup_instance = old_checkup_;
}

int set_scontext_synced::get_random_calls()
{
	return new_rng_->get_random_calls();
}

leave_synced_context::leave_synced_context()
	: old_rng_(randomness::generator)
{
	assert(synced_context::get_synced_state() == synced_context::SYNCED);
	synced_context::set_synced_state(synced_context::LOCAL_CHOICE);

	// calling the synced rng form inside a local_choice would cause oos.
	// TODO: should we also reset the synced checkup?
	randomness::generator = &randomness::rng::default_instance();
}

leave_synced_context::~leave_synced_context()
{
	assert(synced_context::get_synced_state() == synced_context::LOCAL_CHOICE);
	synced_context::set_synced_state(synced_context::SYNCED);
	randomness::generator = old_rng_;
}

set_scontext_unsynced::set_scontext_unsynced()
	: leaver_(synced_context::is_synced() ? new leave_synced_context() : nullptr)
{
}
