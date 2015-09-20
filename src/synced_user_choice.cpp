/*
   Copyright (C) 2015 by the Battle for Wesnoth Project

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
#include "config_assign.hpp"
#include "floating_label.hpp"
#include "game_display.hpp"
#include "game_data.hpp"
#include "log.hpp"
#include "network.hpp"
#include "play_controller.hpp"
#include "synced_context.hpp"
#include "replay.hpp"
#include "resources.hpp"

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <set>
#include <map>

static lg::log_domain log_replay("replay");
#define DBG_REPLAY LOG_STREAM(debug, log_replay)
#define LOG_REPLAY LOG_STREAM(info, log_replay)
#define WRN_REPLAY LOG_STREAM(warn, log_replay)
#define ERR_REPLAY LOG_STREAM(err, log_replay)

namespace
{
	class user_choice_notifer_base
	{
	public:
		virtual void update_message(const std::string&) {}
		virtual void update() {}
		virtual ~user_choice_notifer_base() {}
	};

	class user_choice_notifer_ingame : public user_choice_notifer_base
	{
		//the handle for the label on the screen -i if not shown yet.
		int label_id_;
		unsigned int start_show_;
		std::string message_;
		
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

		virtual void update()
		{
			if(label_id_ == -1 && SDL_GetTicks() > start_show_)
			{
				start_show_label();
			}
		}

		virtual void update_message(const std::string& message)
		{
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
			SDL_Rect area = resources::screen->map_outside_area();
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

	user_choice_notifer_base * create_user_choice_notifer()
	{
		const bool is_too_early = resources::gamedata->phase() != game_data::START && resources::gamedata->phase() != game_data::PLAY;
		if(is_too_early) {
			return new user_choice_notifer_base();
		}
		else {
			return new user_choice_notifer_ingame();
		}
	}
	struct notifer_ptr
	{
		boost::scoped_ptr<user_choice_notifer_base> m_;
		notifer_ptr()
			: m_()
		{
		}
		void activate()
		{
			if(!m_) {
				m_.reset(create_user_choice_notifer());
			}
		}
		void deactivate()
		{
			if(m_) {
				m_.reset();
			}
		}
		void update(const std::string& message)
		{
			if(m_) {
				m_->update_message(message);
				m_->update();
			}
		}

	};
}


static std::map<int, config> get_user_choice_internal(const std::string &name, const mp_sync::user_choice &uch, const std::set<int>& sides)
{
	const int max_side  = static_cast<int>(resources::teams->size());

	BOOST_FOREACH(int side, sides)
	{
		//the caller has to ensure this.
		assert(1 <= side && side <= max_side);
		assert(!(*resources::teams)[side-1].is_empty());
	}


	//this should never change during the execution of this function.
	const int current_side = resources::controller->current_side();
	const bool is_mp_game = network::nconnections() != 0;
	// whether sides contains a side that is not the currently active side.
	const bool contains_other_side = !sides.empty() && (sides.size() != 1 || sides.find(current_side) == sides.end());
	if(contains_other_side)
	{
		synced_context::set_is_simultaneously();
	}
	std::map<int,config> retv;
	notifer_ptr notifer;
	/*
		when we got all our answers we stop.
	*/
	while(retv.size() != sides.size())
	{
		/*
			there might be speak or similar commands in the replay before the user input.
		*/
		do_replay_handle();

		/*
			these value might change due to player left/reassign during pull_remote_user_input
		*/
		//equals to any side in sides that is local, 0 if no such side exists.
		int local_side = 0;
		//if for any side from which we need an answer
		std::string message;
		BOOST_FOREACH(int side, sides)
		{
			//and we havent already received our answer from that side
			if(retv.find(side) == retv.end())
			{
				message += " ";
				message += lexical_cast<std::string>(side);
				//and it is local
				if((*resources::teams)[side-1].is_local() && !(*resources::teams)[side-1].is_idle())
				{
					//then we have to make a local choice.
					local_side = side;
					break;
				}
			}
		}
		message = "waiting for " + uch.description() + " from side(s)" + message;
		bool has_local_side = local_side != 0;
		bool is_replay_end = resources::recorder->at_end();

		if (is_replay_end && has_local_side)
		{
			notifer.deactivate();
			leave_synced_context sync;
			/* At least one of the decisions is ours, and it will be inserted
			   into the replay. */
			DBG_REPLAY << "MP synchronization: local choice\n";
			config cfg = uch.query_user(local_side);

			resources::recorder->user_input(name, cfg, local_side);
			retv[local_side]= cfg;

			//send data to others.
			//but if there wasn't any data sended during this turn, we don't want to bein wth that now.
			//TODO: we should send user choices during nonundoable actions immideatley.
			if(synced_context::is_simultaneously() || current_side != local_side)
			{
				synced_context::send_user_choice();
			}
			continue;

		}
		else if(is_replay_end && !has_local_side)
		{
			//we are in a mp game, and the data has not been recieved yet.
			DBG_REPLAY << "MP synchronization: waiting for remote choice\n";

			assert(is_mp_game);
			synced_context::pull_remote_user_input();
			notifer.activate();
			notifer.update(message);
			SDL_Delay(10);
			continue;
		}
		else if(!is_replay_end)
		{
			DBG_REPLAY << "MP synchronization: extracting choice from replay with has_local_side=" << has_local_side << "\n";

			const config *action = resources::recorder->get_next_action();
			assert(action); //action cannot be null because resources::recorder->at_end() returned false.
			if( !action->has_child(name) || !(*action)["dependent"].to_bool())
			{
				replay::process_error("[" + name + "] expected but none found\n. found instead:\n" + action->debug());
				//We save this action for later
				resources::recorder->revert_action();
				//and let the user try to get the intended result.
				BOOST_FOREACH(int side, sides)
				{
					if(retv.find(side) == retv.end())
					{
						retv[side] = uch.query_user(side);
					}
				}
				return retv;
			}
			int from_side = (*action)["from_side"].to_int(0);
			if ((*action)["side_invalid"].to_bool(false) == true)
			{
				//since this 'cheat' can have a quite heavy effect especialy in umc content we give an oos error .
				replay::process_error("MP synchronization: side_invalid in replay data, this could mean someone wants to cheat.\n");
			}
			if (sides.find(from_side) == sides.end())
			{
				replay::process_error("MP synchronization: we got an answer from side " + boost::lexical_cast<std::string>(from_side) + "for [" + name + "] which is not was we expected\n");
				continue;
			}
			if(retv.find(from_side) != retv.end())
			{
				replay::process_error("MP synchronization: we got already our answer from side " + boost::lexical_cast<std::string>(from_side) + "for [" + name + "] now we have it twice.\n");
			}
			retv[from_side] = action->child(name);
			continue;
		}
	}//while
	return retv;
}

std::map<int,config> mp_sync::get_user_choice_multiple_sides(const std::string &name, const mp_sync::user_choice &uch,
	std::set<int> sides)
{
	//pass sides by copy because we need a copy.
	const bool is_synced = synced_context::is_synced();
	const int max_side  = static_cast<int>(resources::teams->size());
	//we currently don't check for too early because luas sync choice doesn't necessarily show screen dialogs.
	//It (currently) in the responsibility of the user of sync choice to not use dialogs during prestart events..
	if(!is_synced)
	{
		//we got called from inside luas wesnoth.synchronize_choice or from a select event.
		replay::process_error("MP synchronization only works in a synced context (for example Select or preload events are no synced context).\n");
		return std::map<int,config>();
	}

	/*
		for empty sides we want to use random choice instead.
	*/
	std::set<int> empty_sides;
	BOOST_FOREACH(int side, sides)
	{
		assert(1 <= side && side <= max_side);
		if( (*resources::teams)[side-1].is_empty())
		{
			empty_sides.insert(side);
		}
	}

	BOOST_FOREACH(int side, empty_sides)
	{
		sides.erase(side);
	}

	std::map<int,config> retv =  get_user_choice_internal(name, uch, sides);

	BOOST_FOREACH(int side, empty_sides)
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
	const bool is_mp_game = network::nconnections() != 0;//Only used in debugging output below
	const int max_side  = static_cast<int>(resources::teams->size());
	bool is_side_null_controlled;

	if(!is_synced)
	{
		//we got called from inside luas wesnoth.synchronize_choice or from a select event (or maybe a preload event?).
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
	//For [message][option] and luas sync_choice the scenario designer is responsible for that.
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
	is_side_null_controlled = (*resources::teams)[side-1].is_empty();

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
		//which could return something like config_of("invalid", true);
		side = 1;
		while ( side <= max_side  &&  (*resources::teams)[side-1].is_empty() )
			side++;
		assert(side <= max_side);
	}


	assert(1 <= side && side <= max_side);

	std::set<int> sides;
	sides.insert(side);
	std::map<int, config> retv = get_user_choice_internal(name, uch, sides);
	if(retv.find(side) == retv.end())
	{
		//An error occured, get_user_choice_internal should have given an oos error message
		return config();
	}
	return retv[side];
}
