/*
   Copyright (C) 2012 - 2018 by Étienne Simon <etienne.jl.simon@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-test"

#include "whiteboard/action.hpp"
#include "whiteboard/side_actions.hpp"
#include "whiteboard/typedefs.hpp"
#include "whiteboard/visitor.hpp"

#include <boost/test/unit_test.hpp>

using namespace wb;

struct dummy_action: action{
	dummy_action(size_t team_index, bool hidden, int id): action(team_index, hidden), id_(id) {}
	int id_;

	// un-abstraction
	std::ostream& print(std::ostream& s) const { s<<id_; return s; }
	void accept(visitor&){}
	std::shared_ptr<dummy_action> shared_from_this() { return std::static_pointer_cast<dummy_action>(action::shared_from_this()); }
	void execute(bool& success, bool& complete){ success=true; complete=true; }
	void apply_temp_modifier(unit_map&){}
	void remove_temp_modifier(unit_map&){}
	void draw_hex(const map_location&){}
	map_location get_numbering_hex() const { return map_location(); }
	unit_ptr get_unit() const { return unit_ptr(); }
	fake_unit_ptr get_fake_unit(){ return fake_unit_ptr(); }
	error check_validity() const { return OK; }
};

BOOST_AUTO_TEST_SUITE( whiteboard_side_actions_container )

BOOST_AUTO_TEST_CASE( test_insertion )
{
	side_actions_container sac;
	std::shared_ptr<dummy_action> dact;

	// Basic insertions
	std::shared_ptr<dummy_action> act1(new dummy_action(0, false, 1));
	std::shared_ptr<dummy_action> act2(new dummy_action(0, false, 2));
	std::shared_ptr<dummy_action> act3(new dummy_action(0, false, 3));

	sac.queue(0, act2);
	sac.queue(0, act3);
	sac.insert(sac.begin(), act1);

	BOOST_REQUIRE(sac.num_turns() == 1);

	int tmp=0;
	for(action_ptr act : sac) {
		++tmp;
		BOOST_REQUIRE(dact = std::dynamic_pointer_cast<dummy_action>(act));
		BOOST_REQUIRE(dact->id_ == tmp);
	}

	// Multi-turn insertions
	std::shared_ptr<dummy_action> act4(new dummy_action(0, false, 4));
	std::shared_ptr<dummy_action> act5(new dummy_action(0, false, 5));
	std::shared_ptr<dummy_action> act6(new dummy_action(0, false, 6));
	std::shared_ptr<dummy_action> act7(new dummy_action(0, false, 7));
	std::shared_ptr<dummy_action> act8(new dummy_action(0, false, 8));
	sac.queue(1, act5);
	sac.queue(2, act8);
	sac.queue(1, act7);
	sac.queue(0, act4);
	sac.insert(sac.turn_begin(1)+1, act6);

	BOOST_REQUIRE(sac.num_turns() == 3);

	tmp=0;
	for(action_ptr act : sac) {
		++tmp;
		BOOST_REQUIRE(dact = std::dynamic_pointer_cast<dummy_action>(act));
		BOOST_REQUIRE(dact->id_ == tmp);
	}

	BOOST_REQUIRE(dact = std::dynamic_pointer_cast<dummy_action>(*sac.turn_begin(1)));
	BOOST_REQUIRE(dact->id_ == 5);

	BOOST_REQUIRE(dact = std::dynamic_pointer_cast<dummy_action>(*(1+sac.turn_begin(1))));
	BOOST_REQUIRE(dact->id_ == 6);

	BOOST_REQUIRE(sac.turn_size(1) == 3);
	BOOST_REQUIRE(3+sac.turn_begin(1) == sac.turn_end(1));
}

BOOST_AUTO_TEST_CASE( test_removal )
{
	side_actions_container sac;
	std::shared_ptr<dummy_action> dact;

	std::shared_ptr<dummy_action> act1(new dummy_action(0, false, 1));
	std::shared_ptr<dummy_action> act2(new dummy_action(0, false, 2));
	std::shared_ptr<dummy_action> act3(new dummy_action(0, false, 3));
	std::shared_ptr<dummy_action> act4(new dummy_action(0, false, 4));
	std::shared_ptr<dummy_action> act5(new dummy_action(0, false, 5));
	std::shared_ptr<dummy_action> act6(new dummy_action(0, false, 6));

	sac.queue(0, act1);
	side_actions::iterator ite2 = sac.queue(0, act2);
	sac.queue(0, act3);
	side_actions::iterator ite4 = sac.queue(1, act4);
	sac.queue(1, act5);
	side_actions::iterator ite6 = sac.queue(2, act6);

	sac.erase(ite2);
	sac.erase(ite4);
	sac.erase(ite6);

	BOOST_REQUIRE(sac.num_turns() == 2);
	side_actions::iterator it = sac.begin();
	for(int i=1; i<6; i+=2, ++it){
		BOOST_REQUIRE(dact = std::dynamic_pointer_cast<dummy_action>(*it));
		BOOST_REQUIRE(dact->id_ == i);
	}
}

BOOST_AUTO_TEST_SUITE_END()
