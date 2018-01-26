/*
   Copyright (C) 2015 - 2018 by the Battle for Wesnoth Project

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/


#include "synced_user_choice.hpp"

#include "actions/undo.hpp"
#include "floating_label.hpp"
#include "game_display.hpp"
#include "game_data.hpp"
#include "log.hpp"
#include "play_controller.hpp"
#include "synced_context.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "gui/dialogs/multiplayer/synced_choice_wait.hpp"
#include <set>
#include <map>
#include "formula/string_utils.hpp"
#include "font/standard_colors.hpp"

static lg::log_domain log_replay("replay");
#define DBG_REPLAY LOG_STREAM(debug, log_replay)
#define LOG_REPLAY LOG_STREAM(info, log_replay)
#define WRN_REPLAY LOG_STREAM(warn, log_replay)
#define ERR_REPLAY LOG_STREAM(err, log_replay)

namespace
{
	class user_choice_notifer_ingame
	{
		//the handle for the label on the screen -1 if not shown yet.
		int label_id_;
		std::string message_;
		unsigned int start_show_;

	public:
		user_choice_notifer_ingame()
			: label_id_(-1)
			, message_()
			, start_show_(SDL_GetTicks() + 2000)
		{

		}

		~user_choice_notifer_ingame()
		{
			if(label_id_ != -1) {
				end_show_label();
			}
		}

		void update(const std::string& message)
		{
			if(label_id_ == -1 && SDL_GetTicks() > start_show_)
			{
				start_show_label();
			}
			if(message == message_) {
				return;
			}
			message_ = message;
			if(label_id_ != -1) {
				end_show_label();
				start_show_label();
			}
		}

		void start_show_label()
		{
			assert(label_id_ == -1);
			SDL_Rect area = game_display::get_singleton()->map_outside_area();
			font::floating_label flabel(message_);
			flabel.set_font_size(font::SIZE_XLARGE);
			flabel.set_color(font::NORMAL_COLOR);
			flabel.set_position(area.w/2, area.h/4);
			flabel.set_lifetime(-1);
			flabel.set_clip_rect(area);
			label_id_ = font::add_floating_label(flabel);
		}

		void end_show_label()
		{
			assert(label_id_ != -1);
			font::remove_floating_label(label_id_);
			label_id_ = -1;
		}
	};
}

std::map<int,config> mp_sync::get_user_choice_multiple_sides(const std::string &name, const mp_sync::user_choice &uch,
	std::set<int> sides)
{
	//pass sides by copy because we need a copy.
	const bool is_synced = synced_context::is_synced();
	const int max_side  = static_cast<int>(resources::gameboard->teams().size());
	//we currently don't check for too early because lua's sync choice doesn't necessarily show screen dialogs.
	//It (currently) in the responsibility of the user of sync choice to not use dialogs during prestart events..
	if(!is_synced)
	{
		//we got called from inside lua's wesnoth.synchronize_choice or from a select event.
		replay::process_error("MP synchronization only works in a synced context (for example Select or preload events are no synced context).\n");
		return std::map<int,config>();
	}

	/*
		for empty sides we want to use random choice instead.
	*/
	std::set<int> empty_sides;
	for(int side : sides)
	{
		assert(1 <= side && side <= max_side);
		if( resources::gameboard->get_team(side).is_empty())
		{
			empty_sides.insert(side);
		}
	}

	for(int side : empty_sides)
	{
		sides.erase(side);
	}

	std::map<int,config> retv =  user_choice_manager::get_user_choice_internal(name, uch, sides);

	for(int side : empty_sides)
	{
		retv[side] = uch.random_choice(side);
	}
	return retv;

}

/*
	fixes some rare cases and calls get_user_choice_internal if we are in a synced context.
*/
config mp_sync::get_user_choice(const std::string &name, const mp_sync::user_choice &uch,
	int side)
{
	const bool is_too_early = resources::gamedata->phase() != game_data::START && resources::gamedata->phase() != game_data::PLAY;
	const bool is_synced = synced_context::is_synced();
	const bool is_mp_game = resources::controller->is_networked_mp();//Only used in debugging output below
	const int max_side  = static_cast<int>(resources::gameboard->teams().size());
	bool is_side_null_controlled;

	if(!is_synced)
	{
		//we got called from inside lua's wesnoth.synchronize_choice or from a select event (or maybe a preload event?).
		//This doesn't cause problems and someone could use it for example to use a [message][option] inside a wesnoth.synchronize_choice which could be useful,
		//so just give a warning.
		LOG_REPLAY << "MP synchronization called during an unsynced context.\n";
		return uch.query_user(side);
	}
	if(is_too_early && uch.is_visible())
	{
		//We are in a prestart event or even earlier.
		//Although we are able to sync them, we cannot use query_user,
		//because we cannot (or shouldn't) put things on the screen inside a prestart event, this is true for SP and MP games.
		//Quotation form event wiki: "For things displayed on-screen such as character dialog, use start instead"
		return uch.random_choice(side);
	}
	//in start events it's unclear to decide on which side the function should be executed (default= side1 still).
	//But for advancements we can just decide on the side that owns the unit and that's in the responsibility of advance_unit_at.
	//For [message][option] and lua's sync_choice the scenario designer is responsible for that.
	//For [get_global_variable] side is never null.

	/*
		side = 0 should default to the currently active side per definition.
	*/
	if ( side < 1  ||  max_side < side )
	{
		if(side != 0)
		{
			ERR_REPLAY << "Invalid parameter for side in get_user_choice." << std::endl;
		}
		side = resources::controller->current_side();
		LOG_REPLAY << " side changed to " << side << "\n";
	}
	is_side_null_controlled = resources::gameboard->get_team(side).is_empty();

	LOG_REPLAY << "get_user_choice_called with"
			<< " name=" << name
			<< " is_synced=" << is_synced
			<< " is_mp_game=" << is_mp_game
			<< " is_side_null_controlled=" << is_side_null_controlled << "\n";

	if (is_side_null_controlled)
	{
		DBG_REPLAY << "MP synchronization: side 1 being null-controlled in get_user_choice.\n";
		//most likely we are in a start event with an empty side 1
		//but calling [set_global_variable] to an empty side might also cause this.
		//i think in that case we should better use uch.random_choice(),
		//which could return something like config {"invalid", true};
		side = 1;
		while ( side <= max_side  &&  resources::gameboard->get_team(side).is_empty() )
			side++;
		assert(side <= max_side);
	}


	assert(1 <= side && side <= max_side);

	std::set<int> sides;
	sides.insert(side);
	std::map<int, config> retv = user_choice_manager::get_user_choice_internal(name, uch, sides);
	if(retv.find(side) == retv.end())
	{
		//An error occured, get_user_choice_internal should have given an oos error message
		return config();
	}
	return retv[side];
}

user_choice_manager::user_choice_manager(const std::string &name, const mp_sync::user_choice &uch, const std::set<int>& sides)
	: required_(sides)
	, res_()
	, local_choice_(0)
	, wait_message_()
	, oos_(false)
	, uch_(uch)
	, tagname_(name)
	, current_side_(resources::controller->current_side())
	, changed_event_("user_choice_update")
{
	update_local_choice();
	const int max_side  = static_cast<int>(resources::gameboard->teams().size());

	for(int side : required_)
	{
		assert(1 <= side && side <= max_side);
		const team& t = resources::gameboard->get_team(side);
		assert(!t.is_empty());
		if(side != current_side_)
		{
			synced_context::set_is_simultaneously();
		}
	}

	do_replay_handle();
	search_in_replay();

}

void user_choice_manager::search_in_replay()
{
	while(!finished() && !oos_)
	{
		do_replay_handle();
		if(resources::recorder->at_end()) {
			return;
		}

		DBG_REPLAY << "MP synchronization: extracting choice from replay with has_local_side=" << has_local_choice() << "\n";

		const config *action = resources::recorder->get_next_action();
		assert(action); //action cannot be null because resources::recorder->at_end() returned false.
		if( !action->has_child(tagname_) || !(*action)["dependent"].to_bool())
		{
			replay::process_error("[" + tagname_ + "] expected but none found\n. found instead:\n" + action->debug());
			//We save this action for later
			resources::recorder->revert_action();
			// execute this local choice locally
			oos_ = true;
			changed_event_.notify_observers();
			return;
		}
		int from_side = (*action)["from_side"].to_int(0);
		if((*action)["side_invalid"].to_bool(false) == true)
		{
			//since this 'cheat' can have a quite heavy effect especially in umc content we give an oos error .
			replay::process_error("MP synchronization: side_invalid in replay data, this could mean someone wants to cheat.\n");
		}
		if(required_.find(from_side) == required_.end())
		{
			replay::process_error("MP synchronization: we got an answer from side " + std::to_string(from_side) + "for [" + tagname_ + "] which is not was we expected\n");
		}
		if(res_.find(from_side) != res_.end())
		{
			replay::process_error("MP synchronization: we got already our answer from side " + std::to_string(from_side) + "for [" + tagname_ + "] now we have it twice.\n");
		}
		res_[from_side] = action->child(tagname_);
		changed_event_.notify_observers();
	}
}
void user_choice_manager::pull()
{
	// there might be speak or similar commands in the replay before the user input.
	do_replay_handle();
	synced_context::pull_remote_user_input();
	do_replay_handle();
	update_local_choice();
	search_in_replay();
}

void user_choice_manager::update_local_choice()
{
	int local_choice_prev = local_choice_;
	//equals to any side in sides that is local, 0 if no such side exists.
	local_choice_ = 0;
	//if for any side from which we need an answer
	std::string sides_str;
	for(int side : required_)
	{
		//and we havent already received our answer from that side
		if(res_.find(side) == res_.end())
		{
			sides_str += " ";
			sides_str += std::to_string(side);
			//and it is local
			if(resources::gameboard->get_team(side).is_local() && !resources::gameboard->get_team(side).is_idle())
			{
				//then we have to make a local choice.
				local_choice_ = side;
				break;
			}
		}
	}
	wait_message_ = vgettext("waiting for $desc from side(s) $sides", {std::make_pair("desc", uch_.description()), std::make_pair("sides", sides_str)});
	if(local_choice_prev != local_choice_) {
		changed_event_.notify_observers();
	}
}

void user_choice_manager::ask_local_choice()
{
	assert(local_choice_ != 0);

	leave_synced_context sync;
	/* At least one of the decisions is ours, and it will be inserted
	into the replay. */
	DBG_REPLAY << "MP synchronization: local choice\n";
	config cfg = uch_.query_user(local_choice_);
	if(res_.find(local_choice_) != res_.end()) {
		// It might be possible that we this choice was already made by another client while we were in uch_.query_user
		// becase our side might be ressigned while we made our choice.
		WRN_REPLAY << "Discarding a local choice because we found it already on the replay";
		return;
	}
	resources::recorder->user_input(tagname_, cfg, local_choice_);
	res_[local_choice_] = cfg;

	//send data to others.
	//but if there wasn't any data sent during this turn, we don't want to bein wth that now.
	//TODO: we should send user choices during nonundoable actions immideatley.
	if(synced_context::is_simultaneously() || current_side_ != local_choice_)
	{
		synced_context::send_user_choice();
	}
	update_local_choice();
}

void user_choice_manager::fix_oos()
{
	assert(oos_);
	ERR_REPLAY << "A sync error appeared while waiting for a synced user choice of type '" << uch_.description() << "' ([" + tagname_ + "]), doing the choice locally\n";
	for(int side : required_)
	{
		if(res_.find(side) == res_.end())
		{
			ERR_REPLAY << "Doing a local choice for side " << side << "\n";
			res_[side] = uch_.query_user(side);
		}
	}
	oos_ = false;
}

static void wait_ingame(user_choice_manager& man)
{
	user_choice_notifer_ingame notifer;
	while(!man.finished() && man.waiting())
	{
		if(resources::gamedata->phase() == game_data::PLAY || resources::gamedata->phase() == game_data::START)
		{
			//during the prestart/preload event the screen is locked and we shouldn't call user_interact.
			//because that might result in crashs if someone clicks anywhere during screenlock.

			// calls man.pull via events.cpp -> pump_monitor::process
			resources::controller->play_slice();
		}

		notifer.update(man.wait_message());
	}
}

static void wait_prestart(user_choice_manager& man)
{
	gui2::dialogs::synched_choice_wait scw(man);
	scw.show();
}

std::map<int, config> user_choice_manager::get_user_choice_internal(const std::string &name, const mp_sync::user_choice &uch, const std::set<int>& sides)
{
	const bool is_too_early = resources::gamedata->phase() != game_data::START && resources::gamedata->phase() != game_data::PLAY;
	user_choice_manager man(name, uch, sides);
	while(!man.finished())
	{
		if(man.waiting())
		{
			if(is_too_early) {
				wait_prestart(man);
			}
			else {
				wait_ingame(man);
			}
		}
		else if(man.has_local_choice())
		{
			man.ask_local_choice();
		}
		else
		{
			man.fix_oos();
		}
	}
	return man.res_;
}

void user_choice_manager::process(events::pump_info&)
{
	if(waiting())
	{
		pull();
	}
}


