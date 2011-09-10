/* $Id$ */
/*
   Copyright (C) 2008 - 2011 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of thie Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include <boost/test/unit_test.hpp>

#include "log.hpp"
#include "config.hpp"
#include "unit.hpp"
#include "tests/utils/game_config_manager.hpp"
#include "unit_map.hpp"
#include "unit_id.hpp"

#include <boost/bind.hpp>


/*
./test --report_level=detailed --log_level=all --run_test=interpolate_suite

 */

BOOST_AUTO_TEST_SUITE( unit_map_suite )

namespace{

}

BOOST_AUTO_TEST_CASE( test_1 ) {

	config game_config(test_utils::get_test_config());

	config orc_config;
	orc_config["id"]="Orcish Grunt";
	orc_config["random_traits"]=false;
	unit_type orc_type(orc_config);

	unit orc1_side0_real(&orc_type, 0, true);
	unit orc2_side0_fake(&orc_type, 0, false);

	unit_map unit_map;

	typedef std::pair<unit_map::unit_iterator, bool> t_uresult;
	t_uresult uresult1 = unit_map.add(map_location(1,1), orc1_side0_real);

	BOOST_CHECK_MESSAGE(uresult1.second == true, "Good Add");

	unit_map::unit_iterator ui = unit_map.find(map_location(1,1));
	BOOST_CHECK_MESSAGE(uresult1.first == ui, "Good Add");
	BOOST_CHECK_MESSAGE(ui->underlying_id() == orc1_side0_real.underlying_id(), "Found Orc1");

	unit_map::unit_iterator ui2 = unit_map.find(map_location(1,2));
	BOOST_CHECK_MESSAGE(ui2 == unit_map.end(), "Not Found Orc1");
	ui2 = unit_map.find(orc1_side0_real.underlying_id()+1);
	BOOST_CHECK_MESSAGE(ui2 == unit_map.end(), "Not Found Orc1");

	//	unit * orc1p = new unit(orc1_side0_real);

	uresult1 = unit_map.add(map_location(1,1), orc1_side0_real);
	BOOST_CHECK_MESSAGE(uresult1.second == false, "Didn't Add at occupied location.");
	BOOST_CHECK_MESSAGE(uresult1.first == unit_map.end(), "Didn't Add at occupied location.");

	uresult1 = unit_map.add(map_location(-1,1), orc1_side0_real);
	BOOST_CHECK_MESSAGE(uresult1.second == false, "Didn't Add at invalid location.");
	BOOST_CHECK_MESSAGE(uresult1.first == unit_map.end(), "Didn't Add at invalid location.");


	// std::cerr<<"ID real ="<<orc1_side0_real.underlying_id()<<"\n";
	// std::cerr<<"ID fake ="<<orc2_side0_fake.underlying_id()<<"\n";

	uresult1 = unit_map.add(map_location(1,2), orc1_side0_real);
	BOOST_CHECK_MESSAGE(uresult1.second == true, "Added in face of id collision.");
	BOOST_CHECK_MESSAGE(uresult1.first != unit_map.end(), "Added in face of id collision.");
	BOOST_CHECK_MESSAGE(uresult1.first->underlying_id() != orc1_side0_real.underlying_id(), "Found Orc1");



	//To check that the collisions will cut off change the cutoff in unit_map.cpp from 1e6 to less than the guard value below
	// unit_map.add(map_location(1,3), orc2_side0_fake);
	// unit_map.add(map_location(1,3), orc2_side0_fake);

	// unsigned long long guard =0;
	// for(; guard< 2e2;++guard) {
	// 	unit_map.add(map_location(2,guard), orc1_side0_real);
	// };

	// n_unit::id_manager::instance().clear();
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

/* vim: set ts=4 sw=4: */
BOOST_AUTO_TEST_SUITE_END()

