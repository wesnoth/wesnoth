/*
	Copyright (C) 2013 - 2024
	by Andrius Silinskas <silinskas.andrius@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "game_initialization/connect_engine.hpp"
#include "hotkey/hotkey_manager.hpp"
#include "mt_rng.hpp"
#include "saved_game.hpp"
#include "tests/utils/fake_display.hpp"

/* Definitions */

class test_connect_engine : public ng::connect_engine {
public:
	test_connect_engine(saved_game& gamestate) :
		ng::connect_engine(gamestate, true, nullptr)
		{}
};

/* Variables */

namespace {

std::unique_ptr<saved_game> state;
std::unique_ptr<randomness::mt_rng> rng;

}

/* Global fixture */

struct mp_connect_fixture {
	mp_connect_fixture() :
		dummy_args({"wesnoth", "--noaddons"}),
		cmdline_opts(dummy_args),
		hotkey_manager(),
		config_manager()
	{
		test_utils::get_fake_display(1000, 500);
		config_manager.reset(new game_config_manager(cmdline_opts));
		config_manager->init_game_config(game_config_manager::NO_FORCE_RELOAD);

		state.reset(new saved_game());
		state->classification().type = campaign_type::type::multiplayer;
		state->classification().era_id = "era_default";
		config_manager->load_game_config_for_game(state->classification(), state->get_scenario_id());

		state->mp_settings().name = "multiplayer_The_Freelands";
		state->mp_settings().use_map_settings = true;
		state->mp_settings().saved_game = saved_game_mode::type::no;

		state->set_scenario(config_manager->game_config().find_mandatory_child("multiplayer", "id", state->mp_settings().name));

		state->mp_settings().num_turns = state->get_starting_point()["turns"];

		rng.reset(new randomness::mt_rng());
	}
	~mp_connect_fixture()
	{
	}
	std::vector<std::string> dummy_args;
	commandline_options cmdline_opts;
	hotkey::manager hotkey_manager;
	std::unique_ptr<game_config_manager> config_manager;
};


/* Test classes creation utilities */

static test_connect_engine* create_test_connect_engine()
{
	test_connect_engine* connect_engine =
		new test_connect_engine(*state);

	return connect_engine;
}

static ng::side_engine* create_side_engine(const config& defaults,
	test_connect_engine* connect_engine)
{
	config side_cfg = connect_engine->current_config()->mandatory_child("side");
	side_cfg.remove_attributes("faction");
	side_cfg.clear_children("default_faction");
	side_cfg.append(defaults);

	return new ng::side_engine(side_cfg, *connect_engine, 0);
}


/* Tests */

BOOST_FIXTURE_TEST_SUITE( mp_connect, mp_connect_fixture )


BOOST_AUTO_TEST_CASE( flg_map_settings2 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Recruit list with no faction.
	side.clear();
	side["recruit"] = "Elvish Archer";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->new_config()["recruit"], "Elvish Archer" );
}

BOOST_AUTO_TEST_CASE( flg_map_settings3 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Custom faction, no recruits.
	side.clear();
	side["faction"] = "Custom";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->flg().current_faction()["id"], "Custom" );
	BOOST_CHECK_EQUAL( side_engine->new_config()["recruit"].empty(), true );
}

BOOST_AUTO_TEST_CASE( flg_map_settings4 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Random faction.
	side.clear();
	side["faction"] = "Random";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->flg().current_faction()["id"], "Random" );
}

BOOST_AUTO_TEST_CASE( flg_map_settings5 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Valid faction.
	side.clear();
	side["faction"] = "Rebels";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->flg().current_faction()["id"], "Rebels" );
}

BOOST_AUTO_TEST_CASE( flg_map_settings6 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Invalid faction.
	side.clear();
	side["faction"] = "ThisFactionDoesNotExist";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK( side_engine->flg().choosable_factions().size() > 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_faction()["id"], "Random" );
}

BOOST_AUTO_TEST_CASE( flg_map_settings7 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Faction and recruit list.
	side.clear();
	side["recruit"] = "Elvish Archer";
	side["faction"] = "Undead";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
}

BOOST_AUTO_TEST_CASE( flg_map_settings8 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Carried over recruits.
	side.clear();
	side["previous_recruits"] = "Elvish Archer";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->new_config()["previous_recruits"], "Elvish Archer" );
}

BOOST_AUTO_TEST_CASE( flg_map_settings9 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Valid leader unit.
	side.clear();
	side["type"] = "Shadow";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->flg().choosable_leaders().size(), 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_leader(), "Shadow" );
	BOOST_CHECK_EQUAL( side_engine->new_config()["type"], "Shadow" );
}

BOOST_AUTO_TEST_CASE( flg_map_settings10 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Invalid leader unit.
	side.clear();
	side["type"] = "ThisUnitDoesNotExist";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->flg().choosable_leaders().size(), 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_leader(), "null" );
}

BOOST_AUTO_TEST_CASE( flg_map_settings11 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// No leader, Custom faction.
	side.clear();
	side["faction"] = "Custom";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK( side_engine->flg().choosable_leaders().size() > 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_leader(), "random" );
}

BOOST_AUTO_TEST_CASE( flg_map_settings12 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// No leader, Random faction.
	side.clear();
	side["faction"] = "Random";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->flg().choosable_leaders().size(), 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_leader(), "null" );
}

BOOST_AUTO_TEST_CASE( flg_map_settings13 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;


}

BOOST_AUTO_TEST_CASE( flg_map_settings14 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// No leader, regular faction.
	side.clear();
	side["faction"] = "Undead";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK( side_engine->flg().choosable_leaders().size() > 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_leader(), "random" );
}

BOOST_AUTO_TEST_CASE( flg_map_settings15 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

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
}

BOOST_AUTO_TEST_CASE( flg_map_settings16 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;


}

BOOST_AUTO_TEST_CASE( flg_map_settings17 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Random leader.
	side.clear();
	side["type"] = "random";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->flg().choosable_leaders().size(), 1 );
}

BOOST_AUTO_TEST_CASE( flg_map_settings18 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Leader with both genders.
	side.clear();
	side["type"] = "Elvish Archer";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->flg().choosable_genders().size(), 3 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_gender(), "random" );
}

BOOST_AUTO_TEST_CASE( flg_map_settings19 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Leader with only male gender.
	side.clear();
	side["type"] = "Swordsman";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->flg().choosable_genders().size(), 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_gender(), "male" );
}

BOOST_AUTO_TEST_CASE( flg_map_settings20 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Leader with only female gender.
	side.clear();
	side["type"] = "Elvish Druid";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->flg().choosable_genders().size(), 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_gender(), "female" );
}

BOOST_AUTO_TEST_CASE( flg_map_settings21 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Valid leader with valid gender.
	side.clear();
	side["type"] = "White Mage";
	side["gender"] = "female";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->flg().current_gender(), "female" );
}

BOOST_AUTO_TEST_CASE( flg_map_settings22 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Valid leader with invalid gender.
	side.clear();
	side["type"] = "Troll";
	side["gender"] = "female";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->flg().choosable_genders().size(), 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_gender(), "male" );
}

BOOST_AUTO_TEST_CASE( flg_map_settings23 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Leader with random gender.
	side.clear();
	side["type"] = "White Mage";
	side["gender"] = "random";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->flg().current_gender(), "random" );
}

BOOST_AUTO_TEST_CASE( flg_map_settings24 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;


}

BOOST_AUTO_TEST_CASE( flg_map_settings25 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// No leader.
	side.clear();
	side["leader_lock"] = true;
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK_EQUAL( side_engine->flg().choosable_leaders().size(), 1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_leader(), "null" );
}

BOOST_AUTO_TEST_CASE( flg_map_settings26 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Resolve random faction.
	side.clear();
	side["faction"] = "Random";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	side_engine->resolve_random(*rng);
	BOOST_CHECK( side_engine->flg().current_faction()["id"] != "Random" );
	BOOST_CHECK( side_engine->flg().current_leader() != "random" && side_engine->flg().current_leader() != "null");
	BOOST_CHECK( side_engine->flg().current_gender() != "random" && side_engine->flg().current_gender() != "null");
}

BOOST_AUTO_TEST_CASE( flg_map_settings27 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Resolve random faction with default leader.
	side.clear();
	side["faction"] = "Random";
	side["type"] = "Troll";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	side_engine->resolve_random(*rng);
	BOOST_CHECK( side_engine->flg().current_faction()["id"] != "Random" );
	BOOST_CHECK_EQUAL( side_engine->flg().current_leader(), "Troll" );
	BOOST_CHECK( side_engine->flg().current_gender() != "random" && side_engine->flg().current_gender() != "null" );
}

BOOST_AUTO_TEST_CASE( flg_map_settings28 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

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
}

BOOST_AUTO_TEST_CASE( flg_map_settings29 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = true;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Resolve random leader.
	side.clear();
	side["type"] = "random";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	side_engine->resolve_random(*rng);
	BOOST_CHECK( side_engine->flg().current_leader() != "random" );
}





BOOST_AUTO_TEST_CASE( flg_no_map_settings1 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = false;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	const std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Recruit list with no faction.
	side.clear();
	side["recruit"] = "Elvish Archer";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK( side_engine->flg().choosable_factions().size() >  1 );
}

BOOST_AUTO_TEST_CASE( flg_no_map_settings2 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = false;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	const std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Custom faction, no recruits.
	side.clear();
	side["faction"] = "Custom";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK( side_engine->flg().choosable_factions().size() >  1 );
	BOOST_CHECK_EQUAL( side_engine->flg().current_faction()["id"], "Custom" );
	BOOST_CHECK_EQUAL( side_engine->new_config()["recruit"].empty(), true );
}

BOOST_AUTO_TEST_CASE( flg_no_map_settings3 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = false;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	const std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Carried over recruits.
	side.clear();
	side["previous_recruits"] = "Elvish Archer";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK( side_engine->flg().choosable_factions().size() >  1 );
	BOOST_CHECK_EQUAL( side_engine->new_config()["previous_recruits"], "Elvish Archer" );
}

BOOST_AUTO_TEST_CASE( flg_no_map_settings4 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = false;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	const std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Explicit leader for faction with multiple leaders.
	side.clear();
	side["type"] = "Goblin Impaler";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	side_engine->flg().set_current_faction("Rebels");
	BOOST_CHECK( side_engine->flg().choosable_leaders().size() > 1 );
}

BOOST_AUTO_TEST_CASE( flg_no_map_settings5 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = false;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	const std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Duplicate leaders.
	side.clear();
	side["faction"] = "Custom";
	side["type"] = "Swordsman";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	BOOST_CHECK( side_engine->flg().choosable_leaders().size() > 1 );
	const std::vector<std::string>& leaders = side_engine->flg().choosable_leaders();
	BOOST_CHECK_EQUAL( std::count(leaders.begin(), leaders.end(), "Swordsman"), 1 );
}

BOOST_AUTO_TEST_CASE( flg_no_map_settings6 )
{
	// Set up side_engine and its dependencies.
	state->mp_settings().use_map_settings = false;
	state->mp_settings().saved_game = saved_game_mode::type::no;
	const std::unique_ptr<test_connect_engine> connect_engine(create_test_connect_engine());
	ng::side_engine_ptr side_engine;
	config side;

	// Explicit gender for unit with both genders available.
	side.clear();
	side["gender"] = "female";
	side_engine.reset(create_side_engine(side, connect_engine.get()));
	side_engine->flg().set_current_faction("Rebels");
	side_engine->flg().set_current_leader("Elvish Ranger");
	BOOST_CHECK_EQUAL( side_engine->flg().current_gender(), "random" );
}

BOOST_AUTO_TEST_SUITE_END()
