/*
   Copyright (C) 2013 by Andrius Silinskas <silinskas.andrius@gmail.com>
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

#include <boost/test/unit_test.hpp>

#include "game_config_manager.hpp"
#include "game_display.hpp"
#include "multiplayer_connect.hpp"
#include "multiplayer_ui.hpp"

#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>


/* Definitions */

class test_mp_connect : public mp::connect {
public:
	test_mp_connect(game_display& disp, const config& game_config,
		mp::chat& c, config& gamelist, const mp_game_settings& params) :
		mp::connect(disp, game_config, c, gamelist, params, true, true)
		{}

	mp::connect_engine& engine() { return mp::connect::engine(); }

};

class test_mp_connect_engine : public mp::connect_engine {
public:
	test_mp_connect_engine(game_display& disp, const mp_game_settings& params) :
		mp::connect_engine(disp, params, true, true)
		{}

	std::vector<mp::side_engine_ptr>& side_engines()
		{ return mp::connect_engine::side_engines(); }
};


/* Variables */

static boost::scoped_ptr<game_display> disp;
static mp_game_settings params;


/* Global fixture */

struct mp_connect_fixture {
	mp_connect_fixture() :
		video(),
		dummy_argv(),
		cmdline_opts(1, dummy_argv),
		config_manager(NULL)
	{
		video.make_fake();
		disp.reset(game_display::create_dummy_display(video));

		config_manager = new game_config_manager(cmdline_opts, *disp, false);
		config_manager->init_game_config(game_config_manager::NO_FORCE_RELOAD);

		game_state state;
		state.classification().campaign_type = "multiplayer";
		config_manager->load_game_config_for_game(state.classification());

		params.mp_era = "era_default";
		params.name = "multiplayer_The_Freelands";
		params.use_map_settings = true;
		params.saved_game = false;
		params.num_turns = params.scenario_data["turns"];
		params.scenario_data = resources::config_manager->
			game_config().find_child("multiplayer", "id", params.name);
	}
	~mp_connect_fixture()
	{
		delete config_manager;
		config_manager = NULL;
	}
	CVideo video;
	char* dummy_argv[];
	commandline_options cmdline_opts;
	game_config_manager* config_manager;
};


/* Test classes creation utilities */

static test_mp_connect_engine* create_test_mp_connect_engine()
{
	test_mp_connect_engine* mp_connect_engine =
		new test_mp_connect_engine(*disp, params);

	// There must be at least one team.
	mp_connect_engine->team_names().push_back("1");
	mp_connect_engine->user_team_names().push_back("Team: 1");

	// Sides need to be explicitly created and assigned for engine.
	config::child_itors sides = mp_connect_engine->
		current_config()->child_range("side");
	int index = 0;
	BOOST_FOREACH(const config &s, sides) {
		mp::side_engine_ptr new_side_engine(new mp::side_engine(s,
			*mp_connect_engine, index));
		mp_connect_engine->add_side_engine(new_side_engine);

		index++;
	}

	mp_connect_engine->assign_side_for_host();

	return mp_connect_engine;
}

static mp::side_engine* create_mp_side_engine(
	test_mp_connect_engine* connect_engine)
{
	const config& side_cfg = connect_engine->current_config()->child("side");

	return new mp::side_engine(side_cfg, *connect_engine, 0);
}

/* Helper functions */

std::string side_current_faction_id(mp::side_engine_ptr side_engine)
{
	return (*side_engine->choosable_factions()[side_engine->
		current_faction_index()])["id"];
}

const std::string& side_current_leader(mp::side_engine_ptr side_engine)
{
	return side_engine->choosable_leaders()[side_engine->
		current_leader_index()];
}

const std::string& side_current_gender(mp::side_engine_ptr side_engine)
{
	return side_engine->choosable_genders()[side_engine->
		current_gender_index()];
}


/* Tests */

BOOST_GLOBAL_FIXTURE( mp_connect_fixture )
BOOST_AUTO_TEST_SUITE( mp_connect )

BOOST_AUTO_TEST_CASE( side_engine )
{
	// Set up side_engine and its dependencies.
	boost::scoped_ptr<test_mp_connect_engine>
		connect_engine(create_test_mp_connect_engine());
	mp::side_engine_ptr side_engine(create_mp_side_engine(
		connect_engine.get()));

	BOOST_CHECK( side_engine->ready_for_start() );
	BOOST_CHECK( !side_engine->available() );

	// Faction before and after resolved random.
	BOOST_CHECK_EQUAL( side_current_faction_id(side_engine), "Random" );
	side_engine->resolve_random();
	BOOST_CHECK( side_current_faction_id(side_engine) != "Random" );

	// Import network user.
	config data;
	data["name"] = "test_user";
	data["faction"] = "Rebels";
	data["leader"] = "White Mage";
	data["gender"] = "female";
	side_engine->import_network_user(data);
	BOOST_CHECK_EQUAL( side_current_faction_id(side_engine), "Rebels" );
	BOOST_CHECK_EQUAL( side_current_leader(side_engine), "White Mage" );
	BOOST_CHECK_EQUAL( side_current_gender(side_engine), "female" );

	// Set faction to Random.
	side_engine->set_current_faction(side_engine->choosable_factions()[0]);
	BOOST_CHECK_EQUAL( side_current_faction_id(side_engine), "Random" );
	BOOST_CHECK_EQUAL( side_current_leader(side_engine), "null" );
	BOOST_CHECK_EQUAL( side_current_gender(side_engine), "null" );

	// Side config.
	config side_config = side_engine->new_config();
	BOOST_CHECK_EQUAL( side_config["current_player"], "test_user" );
	BOOST_CHECK_EQUAL( side_config["type"], "random" );
	BOOST_CHECK_EQUAL( side_config["gender"], "random" );
}

BOOST_AUTO_TEST_SUITE_END()

