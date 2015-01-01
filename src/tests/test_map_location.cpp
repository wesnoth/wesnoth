/*
   Copyright (C) 2014 - 2015 by Chris Beck <render787@gmail.com>
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

#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
#include <boost/test/unit_test.hpp>

#include "log.hpp"
#include "map_location.hpp"

BOOST_AUTO_TEST_SUITE ( test_map_location )

//#define MAP_LOCATION_GET_OUTPUT

static void characterization_distance_direction (const std::vector<map_location> & locs, const std::vector<map_location::DIRECTION> & dir_answers, const std::vector<size_t> & int_answers)
{
	BOOST_CHECK_EQUAL(dir_answers.size(), int_answers.size());

	std::vector<map_location::DIRECTION>::const_iterator dir_it = dir_answers.begin();
	std::vector<size_t>::const_iterator int_it = int_answers.begin();

	for (std::vector<map_location>::const_iterator it_a = locs.begin(); it_a != locs.end(); ++it_a) {
		for (std::vector<map_location>::const_iterator it_b = it_a + 1; it_b != locs.end(); ++it_b) {
			const map_location & a = *it_a;
			const map_location & b = *it_b;
#ifdef MAP_LOCATION_GET_OUTPUT
			std::cout << "(std::make_pair(" << distance_between(a,b) << ",\t\"" 
				<< map_location::write_direction( a.get_relative_dir(b)) << "\"))" << std::endl;
#else
			int expected_dist = *(int_it++);
			map_location::DIRECTION expected_dir = *(dir_it++);
			BOOST_CHECK_EQUAL( expected_dist, distance_between(a,b) );
			BOOST_CHECK_EQUAL( expected_dist, distance_between(b,a) );
			BOOST_CHECK_EQUAL( expected_dir, a.get_relative_dir(b) );			
			//TODO: Investigate why this is not a valid assertion.
			//BOOST_CHECK_EQUAL( map_location::get_opposite_dir(expected_dir), b.get_relative_dir(a) );
			BOOST_CHECK_EQUAL( a.vector_sum(b), b.vector_sum(a));
			map_location temp1 = a;
			temp1.vector_difference_assign(b);
			map_location temp2 = b;
			temp2.vector_difference_assign(a);
			BOOST_CHECK_EQUAL( temp1, temp2.vector_negation());
			BOOST_CHECK_EQUAL( a, a.vector_negation().vector_negation());
#endif
		}
	}

	BOOST_CHECK_MESSAGE( dir_it == dir_answers.end(), "Did not exhaust answers list.");
	BOOST_CHECK_MESSAGE( int_it == int_answers.end(), "Did not exhaust answers list.");
}

static size_t get_first (std::pair<size_t, std::string> arg) {return arg.first; }
static map_location::DIRECTION get_second (std::pair<size_t, std::string> arg) {return map_location::parse_direction(arg.second); }


BOOST_AUTO_TEST_CASE ( map_location_characterization_test )
{
	map_location a(3,4), b(10,8), c(0,9),z(0,0);
	map_location t1(a.vector_negation()), t2(b.vector_sum(c)), t3(a.vector_sum(c.vector_negation()));
	std::vector<map_location> locs;
	locs.push_back(a);
	locs.push_back(b);
	locs.push_back(c);
	locs.push_back(z);
	locs.push_back(t1);
	locs.push_back(t2);
	locs.push_back(t3);

	std::vector<std::pair<size_t, std::string> > generated_answers = boost::assign::list_of(std::make_pair(7,	"se"))
(std::make_pair(6,	"s"))
(std::make_pair(6,	"n"))
(std::make_pair(12,	"n"))
(std::make_pair(16,	"s"))
(std::make_pair(9,	"n"))
(std::make_pair(10,	"sw"))
(std::make_pair(13,	"nw"))
(std::make_pair(19,	"nw"))
(std::make_pair(9,	"s"))
(std::make_pair(16,	"n"))
(std::make_pair(9,	"n"))
(std::make_pair(15,	"n"))
(std::make_pair(13,	"se"))
(std::make_pair(15,	"n"))
(std::make_pair(6,	"n"))
(std::make_pair(22,	"s"))
(std::make_pair(6,	"n"))
(std::make_pair(28,	"s"))
(std::make_pair(6,	"se"))
(std::make_pair(25,	"n")).to_container(generated_answers);

	std::vector<size_t> ans1;
	std::vector<map_location::DIRECTION> ans2;
	std::transform(generated_answers.begin(), generated_answers.end(), back_inserter(ans1), &get_first);
	std::transform(generated_answers.begin(), generated_answers.end(), back_inserter(ans2), &get_second);

	//ans2 = map_location::parse_directions("se,s,n,n,s,n,ne,se,nw,nw,s,n,ne,n,n,se,n,ne,n,s,n,se,s,se,se,n,ne,se");

	characterization_distance_direction(locs, ans2, ans1);
}

BOOST_AUTO_TEST_SUITE_END()
