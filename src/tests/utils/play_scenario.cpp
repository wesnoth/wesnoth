/* $Id$ */
/*
   Copyright (C) 2008 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of thie Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "tests/utils/play_scenario.hpp"
#include "tests/utils/game_config_manager.hpp"
#include "tests/utils/fake_event_source.hpp"
#include "tests/utils/fake_display.hpp"

#include "gamemap.hpp"
#include "gamestatus.hpp"
#include "playcampaign.hpp"
#include "upload_log.hpp"

#include <boost/bind.hpp>


namespace test_utils {
	play_scenario::play_scenario(const std::string& id) :
		id_(id),
		source_(),
		game_config_(get_test_config_ref()),	
		current_time_(5)
	{
		add_initial_signals();
	}

	void play_scenario::add_initial_signals()
	{
		source_.type_key(current_time_++, SDLK_RETURN);
	}

	struct add_key_type_to_source {
		add_key_type_to_source(fake_event_source& source, timing& time) :
			source_(source),
			time_(time)
		{}
		void operator()(const std::string::value_type& c)
		{

			source_.type_key(time_++, static_cast<SDLKey>(c));
		}
		private:
		fake_event_source& source_;
		timing& time_;
	};

	void play_scenario::add_formula_command(const std::string& command)
	{
		// Activate command line;
		source_.type_key(current_time_++, SDLK_f);
		current_time_ += size_t(2);
		std::for_each(command.begin(), command.end(),
				add_key_type_to_source(source_, current_time_));
	}

	class end_position_collector : public event_node {
		game_state state_;
		unit_map units_;

		public:
			end_position_collector(const size_t time) : 
				event_node(time, SDL_Event()),
				state_(),
				units_()
			{
			}

			virtual void fire_event()
			{
				// Now collect data and quit the game
				std::cerr << "Collecting game data\n\n\n";
				units_ = game_display::get_signelton()->get_units();
			}
			
			game_state& get_state()
			{
				return state_;
			}		
	};

	void play_scenario::play()
	{
		// We have to first append a "event" that collects end poisition
		end_pos_.reset(new end_position_collector(current_time_++));
		end_position_collector* end = static_cast<end_position_collector*>(end_pos_.get());
		source_.add_event(end_pos_);

		const std::string end_command = ":q";
		std::for_each(end_command.begin(), end_command.end(),
				add_key_type_to_source(source_, current_time_));
		
		upload_log no_upload(false);
		game_state& state = end->get_state();
		state.campaign_type = "test";
		state.scenario = id_;
		play_game(get_fake_display(), state, game_config_, no_upload);
	}

	gamemap::location find_unit_loc(const std::string& id)
	{
		if (!end_pos_)
			return gamemap::location(-133,-133);


		return ;
	}

}
