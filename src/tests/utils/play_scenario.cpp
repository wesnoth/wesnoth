/*
   Copyright (C) 2008 - 2015 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of thie Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-test"

#include "tests/utils/play_scenario.hpp"
#include "tests/utils/game_config_manager.hpp"
#include "tests/utils/fake_display.hpp"

#include "config_assign.hpp"
#include "game_display.hpp"
#include "saved_game.hpp"
#include "game_initialization/playcampaign.hpp"
#include "terrain_type_data.hpp"
#include "unit.hpp"
#include "unit_map.hpp"

#include <boost/make_shared.hpp>

#if !SDL_VERSION_ATLEAST(2, 0, 0)
namespace test_utils {
	play_scenario::play_scenario(const std::string& id) :
		id_(id),
		source_(),
		game_config_(get_test_config_ref()),
		tdata_(boost::make_shared<terrain_type_data>(game_config_)),
		current_time_(80),
		end_pos_()
	{
		add_initial_signals();
	}

	void play_scenario::add_initial_signals()
	{
		source_.type_key(current_time_++, SDLK_RETURN);
		current_time_+= 100;

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
		std::for_each(command.begin(), command.end(),
				add_key_type_to_source(source_, current_time_));
		source_.type_key(current_time_++, SDLK_RETURN);
	}

	class end_position_collector : public event_node {
		saved_game state_;
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
				units_ = game_display::get_singleton()->get_units();
			}

			saved_game& get_state()
			{
				return state_;
			}

			unit_map& get_units()
			{
				return units_;
			}
	};

	void play_scenario::play()
	{
		// We have to first append a "event" that collects end poisition
		end_pos_.reset(new end_position_collector(current_time_++));
		end_position_collector* end = static_cast<end_position_collector*>(end_pos_.get());
		source_.add_event(end_pos_);

		source_.type_key(current_time_++, SDLK_COLON, SDLMod(KMOD_LSHIFT | KMOD_SHIFT) );
		source_.type_key(current_time_++, SDLK_q);
		source_.type_key(current_time_++, SDLK_EXCLAIM);
		source_.type_key(current_time_++, SDLK_RETURN);

		saved_game& state = end->get_state();
		state.classification().campaign_type = game_classification::CAMPAIGN_TYPE::TEST;
		state.set_carryover_sides_start(
			config_of("next_scenario", id_)
		);
		campaign_controller controller(get_fake_display(1024, 768), state, game_config_, tdata_);

		controller.play_game();
	}

	std::string play_scenario::get_unit_id(const map_location &loc)
	{
		if (!end_pos_)
			return std::string();

		end_position_collector* end = static_cast<end_position_collector*>(end_pos_.get());

		unit_map::iterator it = end->get_units().find(loc);
		if (it == end->get_units().end())
			return std::string();
		return it->id();
	}

}
#endif
