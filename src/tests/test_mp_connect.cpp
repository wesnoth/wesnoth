/*
   Copyright (C) 2013 - 2015 by Andrius Silinskas <silinskas.andrius@gmail.com>
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
#include "game_initialization/multiplayer_connect.hpp"
#include "game_initialization/multiplayer_ui.hpp"
#include "hotkey/hotkey_manager.hpp"
#include "mt_rng.hpp"
#include "saved_game.hpp"

#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/assign.hpp>


/* Definitions */

/*class test_mp_connect : public mp::connect {
public:
	test_mp_connect(game_display& disp, const std::string& game_name,
		const config& game_config, mp::chat& c, config& gamelist,
		mp::connect_engine& engine) :
		mp::connect(disp, game_name, game_config, c, gamelist, engine)
		{}
};*/

class test_connect_engine : public ng::connect_engine {
public:
	test_connect_engine(game_display& /*disp*/, saved_game& gamestate) :
		ng::connect_engine(gamestate, true, NULL)
		{}
};


/* Variables */

namespace {

boost::scoped_ptr<game_display> disp;
boost::scoped_ptr<saved_game> state;
boost::scoped_ptr<rand_rng::mt_rng> rng;

}


/* Global fixture */

struct mp_connect_fixture {
	mp_connect_fixture() :
		video(),
		dummy_args(boost::assign::list_of("wesnoth")("--noaddons").convert_to_container<std::vector<std::string> >()),
		cmdline_opts(dummy_args),
		hotkey_manager(),
		config_manager()
	{
		video.make_fake();
		disp.reset(game_display::create_dummy_display(video));

		config_manager.reset(new game_config_manager(cmdline_opts, *disp,
			false));
		config_manager->init_game_config(game_config_manager::NO_FORCE_RELOAD);

		state.reset(new saved_game());
		state->classification().campaign_type = game_classification::CAMPAIGN_TYPE::MULTIPLAYER;
		config_manager->load_game_config_for_game(state->classification());

		state->mp_settings().mp_era = "era_default";
		state->mp_settings().name = "multiplayer_The_Freelands";
		state->mp_settings().use_map_settings = true;
		state->mp_settings().saved_game = false;

		state->set_scenario(config_manager->
			game_config().find_child(lexical_cast<std::string>(game_classification::CAMPAIGN_TYPE::MULTIPLAYER), "id", state->mp_settings().name));

		state->mp_settings().num_turns = state->get_starting_pos()["turns"];

		rng.reset(new rand_rng::mt_rng());
	}
	~mp_connect_fixture()
	{
	}
	CVideo video;
	std::vector<std::string> dummy_args;
	commandline_options cmdline_opts;
	hotkey::manager hotkey_manager;
	boost::scoped_ptr<game_config_manager> config_manager;
};


/* Test classes creation utilities */

static test_connect_engine* create_test_connect_engine()
{
	test_connect_engine* connect_engine =
		new test_connect_engine(*disp, *state);

	return connect_engine;
}

static ng::side_engine* create_side_engine(const config& defaults,
	test_connect_engine* connect_engine)
{
	config side_cfg = connect_engine->current_config()->child("side");
	side_cfg.append(defaults);

	return new ng::side_engine(side_cfg, *connect_engine, 0);
}


/* Tests */

BOOST_GLOBAL_FIXTURE( mp_connect_fixture )
BOOST_AUTO_TEST_SUITE( mp_connect )


BOOST_AUTO_TEST_CASE( flg_map_settings )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = false;
	boost::scoped_ptr<test_connect_engine>
		connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Recruit list with no faction.
	side.clear();
	side["recruit"] = "Elvish Archer";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	//BOOST_CHECK_EQUAL( side_engine->flg().choosable_factions().size(), 1 );
	//BOOST_CHECK_EQUAL( side_engine->flg().current_faction()["id"], "Custom" );
	BOOST_CHECK_EQUAL( side_engine->new_config()["recruit"], "Elvish Archer" );

	// Custom faction, no recruits.
	side.clear();
	side["faction"] = "Custom";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	//BOOST_CHECK_EQUAL( side_engine->flg().choosable_factions().size(), 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_faction()["id"], "Custom" );
	BOOST_CHECK_EQUAL( side_engine->new_config()["recruit"].empty(), true );

	// Random faction.
	side.clear();
	side["faction"] = "Random";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	//BOOST_CHECK_EQUAL( side_engine->flg().choosable_factions().size(), 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_faction()["id"], "Random" );

	// Valid faction.
	side.clear();
	side["faction"] = "Rebels";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	//BOOST_CHECK_EQUAL( side_engine->flg().choosable_factions().size(), 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_faction()["id"], "Rebels" );

	// Invalid faction.
	side.clear();
	side["faction"] = "ThisFactionDoesNotExist";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK( side_engine->flg().choosable_factions().size() > 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_faction()["id"], "Random" );

	// Faction and recruit list.
	side.clear();
	side["recruit"] = "Elvish Archer";
	side["faction"] = "Undead";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	//BOOST_CHECK_EQUAL( side_engine->flg().choosable_factions().size(), 1 );
	//BOOST_CHECK_EQUAL( side_engine->flg().current_faction()["id"], "Custom" );
	//BOOST_CHECK_EQUAL( side_engine->new_config()["recruit"], "Elvish Archer" );

	// Carried over recruits.
	side.clear();
	side["previous_recruits"] = "Elvish Archer";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	//BOOST_CHECK_EQUAL( side_engine->flg().choosable_factions().size(), 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_faction()["id"], "Custom" );
	BOOST_CHECK_EQUAL( side_engine->new_config()["previous_recruits"],
		"Elvish Archer" );

	// Valid leader unit.
	side.clear();
	side["type"] = "Shadow";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->flg().choosable_leaders().size(), 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_leader(), "Shadow" );
	BOOST_CHECK_EQUAL( side_engine->new_config()["type"], "Shadow" );

	// Invalid leader unit.
	side.clear();
	side["type"] = "ThisUnitDoesNotExist";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->flg().choosable_leaders().size(), 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_leader(), "null" );

	// No leader, Custom faction.
	side.clear();
	side["faction"] = "Custom";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK( side_engine->flg().choosable_leaders().size() > 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_leader(), "random" );

	// No leader, Random faction.
	side.clear();
	side["faction"] = "Random";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->flg().choosable_leaders().size(), 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_leader(), "null" );

	// No leader, regular faction.
	side.clear();
	side["faction"] = "Undead";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK( side_engine->flg().choosable_leaders().size() > 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_leader(), "random" );

	// Carried over leader.
	side.clear();
	side["id"] = "LeaderID";
	side["type"] = "Elvish Archer";
	config& unit = side.add_child("unit");
	unit["id"] = "LeaderID";
	unit["type"] = "Elvish Ranger";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->flg().choosable_leaders().size(), 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_leader(), "Elvish Ranger" );

	// Random leader.
	side.clear();
	side["type"] = "random";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->flg().choosable_leaders().size(), 1 );

	// Leader with both genders.
	side.clear();
	side["type"] = "Elvish Archer";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->flg().choosable_genders().size(), 3 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_gender(), "random" );

	// Leader with only male gender.
	side.clear();
	side["type"] = "Swordsman";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->flg().choosable_genders().size(), 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_gender(), "male" );

	// Leader with only female gender.
	side.clear();
	side["type"] = "Elvish Druid";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->flg().choosable_genders().size(), 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_gender(), "female" );

	// Valid leader with valid gender.
	side.clear();
	side["type"] = "White Mage";
	side["gender"] = "female";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	//BOOST_CHECK_EQUAL( side_engine->flg().choosable_genders().size(), 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_gender(), "female" );

	// Valid leader with invalid gender.
	side.clear();
	side["type"] = "Troll";
	side["gender"] = "female";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->flg().choosable_genders().size(), 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_gender(), "male" );

	// Leader with random gender.
	side.clear();
	side["type"] = "White Mage";
	side["gender"] = "random";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	//BOOST_CHECK_EQUAL( side_engine->flg().choosable_genders().size(), 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_gender(), "random" );

	// No leader.
	side.clear();
	side["no_leader"] = true;
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->flg().choosable_leaders().size(), 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_leader(), "null" );

	// Resolve random faction.
	side.clear();
	side["faction"] = "Random";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	side_engine->resolve_random(*rng);
	BOOST_CHECK( side_engine->flg().current_faction()["id"] != "Random" );
	BOOST_CHECK( side_engine->flg().current_leader() != "random" &&
		side_engine->flg().current_leader() != "null");
	BOOST_CHECK( side_engine->flg().current_gender() != "random" &&
		side_engine->flg().current_gender() != "null");

	// Resolve random faction with default leader.
	side.clear();
	side["faction"] = "Random";
	side["type"] = "Troll";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	side_engine->resolve_random(*rng);
	BOOST_CHECK( side_engine->flg().current_faction()["id"] != "Random" );
	BOOST_CHECK_EQUAL( side_engine->flg().current_leader(), "Troll" );
	BOOST_CHECK( side_engine->flg().current_gender() != "random" &&
		side_engine->flg().current_gender() != "null" );

	// Resolve random faction with default leader and gender.
	side.clear();
	side["faction"] = "Random";
	side["type"] = "White Mage";
	side["gender"] = "male";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	side_engine->resolve_random(*rng);
	BOOST_CHECK( side_engine->flg().current_faction()["id"] != "Random" );
	BOOST_CHECK_EQUAL( side_engine->flg().current_leader(), "White Mage" );
	BOOST_CHECK_EQUAL( side_engine->flg().current_gender(), "male" );

	// Resolve random leader.
	side.clear();
	side["type"] = "random";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	side_engine->resolve_random(*rng);
	BOOST_CHECK( side_engine->flg().current_leader() != "random" );
}

BOOST_AUTO_TEST_CASE( flg_no_map_settings )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = false;
	state->mp_settings().saved_game = false;
	boost::scoped_ptr<test_connect_engine>
		connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Recruit list with no faction.
	side.clear();
	side["recruit"] = "Elvish Archer";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK( side_engine->flg().choosable_factions().size() >  1 );
	//BOOST_CHECK_EQUAL( side_engine->flg().current_faction()["id"], "Custom" );

	// Custom faction, no recruits.
	side.clear();
	side["faction"] = "Custom";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK( side_engine->flg().choosable_factions().size() >  1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_faction()["id"], "Custom" );
	BOOST_CHECK_EQUAL( side_engine->new_config()["recruit"].empty(), true );

	// Carried over recruits.
	side.clear();
	side["previous_recruits"] = "Elvish Archer";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK( side_engine->flg().choosable_factions().size() >  1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_faction()["id"], "Custom" );
	BOOST_CHECK_EQUAL( side_engine->new_config()["previous_recruits"],
		"Elvish Archer" );

	// Explicit leader for faction with multiple leaders.
	side.clear();
	side["type"] = "Goblin Impaler";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	side_engine->flg().set_current_faction("Rebels");
	BOOST_CHECK( side_engine->flg().choosable_leaders().size() > 1 );

	// Duplicate leaders.
	side.clear();
	side["faction"] = "Custom";
	side["type"] = "Swordsman";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK( side_engine->flg().choosable_leaders().size() > 1 );
	const std::vector<std::string>& leaders =
		side_engine->flg().choosable_leaders();
	BOOST_CHECK_EQUAL( std::count(leaders.begin(), leaders.end(), "Swordsman"),
		1 );

	// Explicit gender for unit with both genders available.
	side.clear();
	side["gender"] = "female";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	side_engine->flg().set_current_faction("Rebels");
	side_engine->flg().set_current_leader("Elvish Ranger");
	BOOST_CHECK_EQUAL( side_engine->flg().current_gender(), "random" );
}

BOOST_AUTO_TEST_CASE( flg_saved_game )
{
	// TODO
}

BOOST_AUTO_TEST_SUITE_END()

