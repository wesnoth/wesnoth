/*
   Copyright (C) 2008 - 2018 by Pauli Nieminen <paniemin@cc.hut.fi>
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

#include "log.hpp"
#include "config.hpp"
#include "units/unit.hpp"
#include "tests/utils/game_config_manager_tests.hpp"
#include "units/map.hpp"
#include "units/id.hpp"

#include "utils/functional.hpp"


/*
./test --report_level=detailed --log_level=all --run_test=interpolate_suite

 */

BOOST_AUTO_TEST_SUITE( unit_map_suite )

BOOST_AUTO_TEST_CASE( test_1 ) {
	config game_config(test_utils::get_test_config());

	config orc_config;
	orc_config["id"]="Orcish Grunt";
	orc_config["random_traits"]=false;
	orc_config["animate"]=false;
	unit_type orc_type(orc_config);

	unit_types.build_unit_type(orc_type, unit_type::FULL);

	unit_ptr orc1_side0_real = unit::create(orc_type, 0, false);
	unit_ptr orc2_side0_fake = unit::create(orc_type, 0, false);

	unit_map unit_map;

	typedef std::pair<unit_map::unit_iterator, bool> t_uresult;
	t_uresult uresult1 = unit_map.add(map_location(1,1), *orc1_side0_real);

	BOOST_CHECK_MESSAGE(uresult1.second == true, "Good Add");
	BOOST_CHECK_EQUAL(unit_map.size(), 1);

	unit_map::unit_iterator ui = unit_map.find(map_location(1,1));
	BOOST_CHECK_MESSAGE(uresult1.first == ui, "Good Add");
	BOOST_CHECK_MESSAGE(ui->underlying_id() == orc1_side0_real->underlying_id(), "Found Orc1");

	unit_map::unit_iterator ui2 = unit_map.find(map_location(1,2));
	BOOST_CHECK_MESSAGE(ui2 == unit_map.end(), "Not Found Orc1");
	ui2 = unit_map.find(orc1_side0_real->underlying_id()+1);
	BOOST_CHECK_MESSAGE(ui2 == unit_map.end(), "Not Found Orc1");

	//	unit * orc1p = new unit(orc1_side0_real);

	lg::set_log_domain_severity("engine", lg::err().get_severity() - 1); // Don't log anything
	lg::set_log_domain_severity("unit", lg::err());
	uresult1 = unit_map.add(map_location(1,1), *orc1_side0_real);
	lg::set_log_domain_severity("unit", lg::warn());
	lg::set_log_domain_severity("engine", lg::info());
	BOOST_CHECK_EQUAL(unit_map.size(), 1);
	BOOST_CHECK_MESSAGE(uresult1.second == false, "Didn't Add at occupied location.");
	BOOST_CHECK_MESSAGE(uresult1.first == unit_map.end(), "Didn't Add at occupied location.");

	lg::set_log_domain_severity("engine", lg::err().get_severity() - 1); // Don't log anything
	// If the location is invalid, the unit never needs to be cloned, so no warning is emitted in the unit domain
	uresult1 = unit_map.add(map_location(-1,1), *orc1_side0_real);
	lg::set_log_domain_severity("engine", lg::info());
	BOOST_CHECK_EQUAL(unit_map.size(), 1);
	BOOST_CHECK_MESSAGE(uresult1.second == false, "Didn't Add at invalid location.");
	BOOST_CHECK_MESSAGE(uresult1.first == unit_map.end(), "Didn't Add at invalid location.");


	// std::cerr<<"ID real ="<<orc1_side0_real.underlying_id()<<"\n";
	// std::cerr<<"ID fake ="<<orc2_side0_fake.underlying_id()<<"\n";

	lg::set_log_domain_severity("engine", lg::err().get_severity() - 1); // Don't log anything
	lg::set_log_domain_severity("unit", lg::err());
	uresult1 = unit_map.add(map_location(1,2), *orc1_side0_real);
	lg::set_log_domain_severity("unit", lg::warn());
	lg::set_log_domain_severity("engine", lg::info());
	BOOST_CHECK_EQUAL(unit_map.size(), 2);
	BOOST_CHECK_MESSAGE(uresult1.second == true, "Added in face of id collision.");
	BOOST_CHECK_MESSAGE(uresult1.first != unit_map.end(), "Added in face of id collision.");
	BOOST_CHECK_MESSAGE(uresult1.first->underlying_id() != orc1_side0_real->underlying_id(), "Found Orc1");

	BOOST_CHECK_MESSAGE(!unit_map.end().valid(), "Hmm, unit_map.end() is valid for dereference...");
	//To check that the collisions will cut off change the cutoff in unit_map.cpp from 1e6 to less than the guard value below
	// unit_map.add(map_location(1,3), orc2_side0_fake);
	// unit_map.add(map_location(1,3), orc2_side0_fake);

	// unsigned long long guard =0;
	// for(; guard< 2e2;++guard) {
	// 	unit_map.add(map_location(2,guard), orc1_side0_real);
	// };

	// std::cerr<<"BREAK\n;";
	// unit_map.add(map_location(1,3), orc2_side0_fake);
	// unit_map.add(map_location(1,4), orc2_side0_fake);
	// try {
	// 	unit_map.add(map_location(1,5), orc2_side0_fake);
	// }catch (std::runtime_error e ){
	// 	BOOST_CHECK_MESSAGE(std::string(e.what()) == std::string("One million collisions in unit_map")
	// 						, "One million uid collision exception");
	// }

}

BOOST_AUTO_TEST_CASE( track_real_unit_by_underlying_id ) {
	config game_config(test_utils::get_test_config());

	config orc_config;
	orc_config["id"]="Orcish Grunt";
	orc_config["random_traits"] = false;
	orc_config["animate"]=false;
	unit_type orc_type(orc_config);

	unit_types.build_unit_type(orc_type, unit_type::FULL);

	unit_ptr orc1_side0_real = unit::create(orc_type, 0, true);

	std::size_t underlying_id = orc1_side0_real->underlying_id();
	map_location hex = map_location(1,1);

	unit_map unit_map;

	typedef std::pair<unit_map::unit_iterator, bool> t_uresult;
	t_uresult uresult1 = unit_map.add(hex, *orc1_side0_real);

	BOOST_CHECK(uresult1.second == true);

	{
		unit_map::unit_iterator ui = unit_map.find(underlying_id);
		BOOST_CHECK(uresult1.first == ui);
		BOOST_CHECK(ui->underlying_id() == orc1_side0_real->underlying_id());
	}

	unit_ptr extracted_unit = unit_map.extract(hex);

	{
		unit_map::unit_iterator ui = unit_map.find(underlying_id);
		BOOST_CHECK(ui == unit_map.end());
	}

	unit_map.insert(extracted_unit);
	extracted_unit.reset();

	{
		unit_map::unit_iterator ui = unit_map.find(underlying_id);
		BOOST_CHECK(uresult1.first == ui);
		BOOST_CHECK(ui->underlying_id() == orc1_side0_real->underlying_id());
	}
}

BOOST_AUTO_TEST_CASE( track_fake_unit_by_underlying_id ) {
	config game_config(test_utils::get_test_config());

	config orc_config;
	orc_config["id"]="Orcish Grunt";
	orc_config["random_traits"] = false;
	orc_config["animate"]=false;
	unit_type orc_type(orc_config);

	unit_types.build_unit_type(orc_type, unit_type::FULL);

	unit_ptr orc1_side0_fake = unit::create(orc_type, 0, false);

	std::size_t underlying_id = orc1_side0_fake->underlying_id();
	map_location hex = map_location(1,1);

	unit_map unit_map;

	typedef std::pair<unit_map::unit_iterator, bool> t_uresult;
	t_uresult uresult1 = unit_map.add(hex, *orc1_side0_fake);

	BOOST_CHECK(uresult1.second == true);

	{
		unit_map::unit_iterator ui = unit_map.find(underlying_id);
		BOOST_CHECK(uresult1.first == ui);
		BOOST_CHECK(ui->underlying_id() == orc1_side0_fake->underlying_id());
	}

	unit_ptr extracted_unit = unit_map.extract(hex);

	{
		unit_map::unit_iterator ui = unit_map.find(underlying_id);
		BOOST_CHECK(ui == unit_map.end());
	}

	unit_map.insert(extracted_unit);
	extracted_unit.reset();

	{
		unit_map::unit_iterator ui = unit_map.find(underlying_id);
		BOOST_CHECK(uresult1.first == ui);
		BOOST_CHECK(ui->underlying_id() == orc1_side0_fake->underlying_id());
	}
}

BOOST_AUTO_TEST_CASE( track_real_unit_by_iterator ) {
	config game_config(test_utils::get_test_config());

	config orc_config;
	orc_config["id"]="Orcish Grunt";
	orc_config["random_traits"] = false;
	orc_config["animate"]=false;
	unit_type orc_type(orc_config);

	unit_types.build_unit_type(orc_type, unit_type::FULL);

	unit_ptr orc1_side0_real = unit::create(orc_type, 0, true);

	map_location hex = map_location(1,1);

	unit_map unit_map;

	typedef std::pair<unit_map::unit_iterator, bool> t_uresult;
	t_uresult uresult1 = unit_map.add(hex, *orc1_side0_real);

	unit_map::unit_iterator unit_iterator = uresult1.first;

	BOOST_CHECK(unit_iterator.valid());

	unit_ptr extracted_unit = unit_map.extract(hex);

	BOOST_CHECK_MESSAGE(unit_iterator.valid() == false, "Iterator should be invalid after extraction.");

	unit_map.insert(extracted_unit);

	BOOST_CHECK_MESSAGE(unit_iterator.valid(), "Iterator should be valid after extraction and reinsertion.");

	unit_map::unit_iterator unit_iterator2 = unit_map.find(hex);
	BOOST_CHECK(unit_iterator2.valid());
	BOOST_CHECK(unit_iterator == unit_iterator2);
}

BOOST_AUTO_TEST_CASE( track_fake_unit_by_iterator ) {
	config game_config(test_utils::get_test_config());

	config orc_config;
	orc_config["id"]="Orcish Grunt";
	orc_config["random_traits"] = false;
	orc_config["animate"]=false;
	unit_type orc_type(orc_config);

	unit_types.build_unit_type(orc_type, unit_type::FULL);

	unit_ptr orc1_side0_fake = unit::create(orc_type, 0, false);

	map_location hex = map_location(1,1);

	unit_map unit_map;

	typedef std::pair<unit_map::unit_iterator, bool> t_uresult;
	t_uresult uresult1 = unit_map.add(hex, *orc1_side0_fake);

	unit_map::unit_iterator unit_iterator = uresult1.first;

	BOOST_CHECK(unit_iterator.valid());

	unit_ptr extracted_unit = unit_map.extract(hex);

	BOOST_CHECK_MESSAGE(unit_iterator.valid() == false, "Iterator should be invalid after extraction.");

	unit_map.insert(extracted_unit);

	BOOST_CHECK_MESSAGE(unit_iterator.valid(), "Iterator should be valid after extraction and reinsertion.");

	unit_map::unit_iterator unit_iterator2 = unit_map.find(hex);
	BOOST_CHECK(unit_iterator2.valid());
	BOOST_CHECK(unit_iterator == unit_iterator2);
}

/* vim: set ts=4 sw=4: */
BOOST_AUTO_TEST_SUITE_END()
