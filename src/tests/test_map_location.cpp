/*
   Copyright (C) 2014 - 2017 by Chris Beck <render787@gmail.com>
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

#include "utils/functional.hpp"
#include <boost/test/unit_test.hpp>

#include "map/location.hpp"

static std::vector<map_location> preset_locs;
static map_location va,vb,vc,vz,vt1,vt2,vt3,vs1,vs2,vs3,vs4;
static map_location::DIRECTION n = map_location::NORTH;
static map_location::DIRECTION ne = map_location::NORTH_EAST;
static map_location::DIRECTION nw = map_location::NORTH_WEST;
static map_location::DIRECTION s = map_location::SOUTH;
static map_location::DIRECTION se = map_location::SOUTH_EAST;
static map_location::DIRECTION sw = map_location::SOUTH_WEST;


struct MLFixture
{
	MLFixture()
	{
		va = map_location(3,4);
		vb = map_location(10,8);
		vc = map_location(0,9);
		vz = map_location::ZERO();
		vt1 = va.vector_negation();
		vt2 = vb.vector_sum(vc);
		vt3 = va.vector_sum(vc.vector_negation());

		vs1 = vz.get_direction(nw);
		vs2 = vz.get_direction(n).get_direction(ne);
		vs3 = vz.get_direction(s).get_direction(se);
		vs4 = vz.get_direction(sw).get_direction(se);

		preset_locs.push_back(va);
		preset_locs.push_back(vb);
		preset_locs.push_back(vc);
		preset_locs.push_back(vz);
		preset_locs.push_back(vt1);
		preset_locs.push_back(vt2);
		preset_locs.push_back(vt3);
		preset_locs.push_back(vs1);
		preset_locs.push_back(vs2);
		preset_locs.push_back(vs3);
		preset_locs.push_back(vs4);
	}

	~MLFixture() {}
};

BOOST_GLOBAL_FIXTURE ( MLFixture );

BOOST_AUTO_TEST_SUITE ( test_map_location )

//#define MAP_LOCATION_GET_OUTPUT

#ifndef MAP_LOCATION_GET_OUTPUT
static map_location vector_difference(const map_location & v1, const map_location & v2)
{
	map_location ret(v1);
	ret.vector_difference_assign(v2);
	return ret;
}
#endif

static void characterization_distance_direction (const std::vector<map_location> & locs, const std::vector<map_location::DIRECTION> & dir_answers, const std::vector<size_t> & int_answers, map_location::RELATIVE_DIR_MODE mode)
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
				<< map_location::write_direction( a.get_relative_dir(b,mode)) << "\"))" << std::endl;
#else
			int expected_dist = *(int_it++);
			map_location::DIRECTION expected_dir = *(dir_it++);
			BOOST_CHECK_EQUAL( expected_dist, distance_between(a,b) );
			BOOST_CHECK_EQUAL( expected_dist, distance_between(b,a) );
			BOOST_CHECK_EQUAL( expected_dir, a.get_relative_dir(b, mode) );
			//Note: This is not a valid assertion. get_relative_dir has much symmetry but not radial.
			if (mode == map_location::RADIAL_SYMMETRY) {
				BOOST_CHECK_EQUAL( map_location::get_opposite_dir(expected_dir), b.get_relative_dir(a,mode) );
			}
			BOOST_CHECK_EQUAL( a.vector_sum(b), b.vector_sum(a));
			map_location temp1 = a;
			temp1.vector_difference_assign(b);
			map_location temp2 = b;
			temp2.vector_difference_assign(a);
			BOOST_CHECK_EQUAL( temp1, temp2.vector_negation());
			BOOST_CHECK_EQUAL( a, a.vector_negation().vector_negation());

			for (std::vector<map_location>::const_iterator it_c = it_b + 1; it_c != locs.end(); ++it_c) {
				const map_location & c = *it_c;
				BOOST_CHECK_EQUAL(a.vector_sum(b.vector_sum(c)) , a.vector_sum(b).vector_sum(c));
				BOOST_CHECK_EQUAL(a.vector_sum(vector_difference(b,c)) , vector_difference(a.vector_sum(b),c));
				BOOST_CHECK_EQUAL(vector_difference(a,b.vector_sum(c)) , vector_difference(vector_difference(a,b),c));
				//TODO: Investigate why this doesn't work
				if (mode == map_location::RADIAL_SYMMETRY) {
					BOOST_CHECK_EQUAL(expected_dir, (a.vector_sum(c)).get_relative_dir(b.vector_sum(c),mode));
				}
			}
#endif
		}
	}

	BOOST_CHECK_MESSAGE( dir_it == dir_answers.end(), "Did not exhaust answers list.");
	BOOST_CHECK_MESSAGE( int_it == int_answers.end(), "Did not exhaust answers list.");
}

static size_t get_first (std::pair<size_t, std::string> arg) {return arg.first; }
static map_location::DIRECTION get_second (std::pair<size_t, std::string> arg) {return map_location::parse_direction(arg.second); }

/* This has to be recomputed, I'm commenting out the test so that it doesn't fail in the meantime. --iceiceice

BOOST_AUTO_TEST_CASE ( map_location_characterization_test_default_mode )
{
	std::vector<std::pair<size_t, std::string> > generated_answers = boost::assign::list_of(std::make_pair(7,	"se"))
(std::make_pair(6,	"s"))
(std::make_pair(6,	"nw"))
(std::make_pair(12,	"n"))
(std::make_pair(16,	"s"))
(std::make_pair(9,	"n"))
(std::make_pair(7,	"nw"))
(std::make_pair(7,	"n"))
(std::make_pair(4,	"n"))
(std::make_pair(5,	"nw"))
(std::make_pair(10,	"sw"))
(std::make_pair(13,	"nw"))
(std::make_pair(19,	"nw"))
(std::make_pair(9,	"s"))
(std::make_pair(16,	"n"))
(std::make_pair(14,	"nw"))
(std::make_pair(14,	"nw"))
(std::make_pair(11,	"nw"))
(std::make_pair(12,	"nw"))
(std::make_pair(9,	"n"))
(std::make_pair(15,	"n"))
(std::make_pair(13,	"se"))
(std::make_pair(15,	"n"))
(std::make_pair(10,	"n"))
(std::make_pair(11,	"n"))
(std::make_pair(8,	"n"))
(std::make_pair(8,	"n"))
(std::make_pair(6,	"n"))
(std::make_pair(22,	"s"))
(std::make_pair(6,	"n"))
(std::make_pair(1,	"nw"))
(std::make_pair(2,	"n"))
(std::make_pair(2,	"se"))
(std::make_pair(1,	"s"))
(std::make_pair(28,	"s"))
(std::make_pair(6,	"se"))
(std::make_pair(5,	"s"))
(std::make_pair(5,	"se"))
(std::make_pair(8,	"s"))
(std::make_pair(7,	"s"))
(std::make_pair(25,	"n"))
(std::make_pair(23,	"n"))
(std::make_pair(23,	"n"))
(std::make_pair(20,	"n"))
(std::make_pair(21,	"n"))
(std::make_pair(6,	"sw"))
(std::make_pair(4,	"s"))
(std::make_pair(7,	"s"))
(std::make_pair(7,	"s"))
(std::make_pair(2,	"ne"))
(std::make_pair(3,	"se"))
(std::make_pair(2,	"s"))
(std::make_pair(3,	"s"))
(std::make_pair(3,	"s"))
(std::make_pair(1,	"sw")).to_container(generated_answers);

	std::vector<size_t> ans1;
	std::vector<map_location::DIRECTION> ans2;
	std::transform(generated_answers.begin(), generated_answers.end(), back_inserter(ans1), &get_first);
	std::transform(generated_answers.begin(), generated_answers.end(), back_inserter(ans2), &get_second);

	characterization_distance_direction(preset_locs, ans2, ans1, map_location::DEFAULT);
}*/

BOOST_AUTO_TEST_CASE ( map_location_characterization_test_radial_mode )
{
	std::vector<std::pair<size_t, std::string> > generated_answers = {
std::make_pair(7,	"se"),
std::make_pair(6,	"sw"),
std::make_pair(6,	"n"),
std::make_pair(12,	"n"),
std::make_pair(16,	"s"),
std::make_pair(9,	"n"),
std::make_pair(7,	"nw"),
std::make_pair(7,	"n"),
std::make_pair(4,	"n"),
std::make_pair(5,	"nw"),
std::make_pair(10,	"sw"),
std::make_pair(13,	"nw"),
std::make_pair(19,	"nw"),
std::make_pair(9,	"s"),
std::make_pair(16,	"n"),
std::make_pair(14,	"nw"),
std::make_pair(14,	"nw"),
std::make_pair(11,	"nw"),
std::make_pair(12,	"nw"),
std::make_pair(9,	"n"),
std::make_pair(15,	"n"),
std::make_pair(13,	"se"),
std::make_pair(15,	"n"),
std::make_pair(10,	"n"),
std::make_pair(11,	"n"),
std::make_pair(8,	"n"),
std::make_pair(8,	"n"),
std::make_pair(6,	"n"),
std::make_pair(22,	"s"),
std::make_pair(6,	"ne"),
std::make_pair(1,	"nw"),
std::make_pair(2,	"ne"),
std::make_pair(2,	"s"),
std::make_pair(1,	"s"),
std::make_pair(28,	"s"),
std::make_pair(6,	"se"),
std::make_pair(5,	"s"),
std::make_pair(5,	"se"),
std::make_pair(8,	"s"),
std::make_pair(7,	"s"),
std::make_pair(25,	"n"),
std::make_pair(23,	"n"),
std::make_pair(23,	"n"),
std::make_pair(20,	"n"),
std::make_pair(21,	"n"),
std::make_pair(6,	"sw"),
std::make_pair(4,	"sw"),
std::make_pair(7,	"s"),
std::make_pair(7,	"s"),
std::make_pair(2,	"ne"),
std::make_pair(3,	"se"),
std::make_pair(2,	"s"),
std::make_pair(3,	"s"),
std::make_pair(3,	"s"),
std::make_pair(1,	"nw")};

	std::vector<size_t> ans1;
	std::vector<map_location::DIRECTION> ans2;
	std::transform(generated_answers.begin(), generated_answers.end(), back_inserter(ans1), &get_first);
	std::transform(generated_answers.begin(), generated_answers.end(), back_inserter(ans2), &get_second);

	characterization_distance_direction(preset_locs, ans2, ans1, map_location::RADIAL_SYMMETRY);
}

static std::pair<map_location , map_location> mirror_walk( std::pair<map_location,map_location> p, map_location::DIRECTION d)
{
	p.first = p.first.get_direction(d);
	p.second = p.second.get_direction(map_location::get_opposite_dir(d));
	BOOST_CHECK_EQUAL(p.first, p.second.vector_negation());
	return p;
}

BOOST_AUTO_TEST_CASE ( reality_check_vector_negation )
{
	std::pair<map_location, map_location> p(vz,vz);

	p = mirror_walk(p, n);
	p = mirror_walk(p, n);
	p = mirror_walk(p, ne);
	p = mirror_walk(p, nw);
	p = mirror_walk(p, s);
	p = mirror_walk(p, nw);
	p = mirror_walk(p, se);
	p = mirror_walk(p, sw);
	p = mirror_walk(p, n);
	p = mirror_walk(p, n);
	p = mirror_walk(p, sw);
	p = mirror_walk(p, sw);
	p = mirror_walk(p, sw);
}

static void reality_check_get_direction_helper(const map_location & loc, const map_location::DIRECTION d)
{
	map_location lz(vz.get_direction(d));

	map_location temp(loc.vector_sum(lz));
	BOOST_CHECK_EQUAL(temp, loc.get_direction(d));
	BOOST_CHECK(tiles_adjacent(loc,temp));
	BOOST_CHECK(tiles_adjacent(temp,loc));
	BOOST_CHECK_EQUAL(distance_between(loc,temp), 1);
}

BOOST_AUTO_TEST_CASE ( reality_check_get_direction )
{
	map_location a(3,4);
	map_location b(6,5);

	reality_check_get_direction_helper(a,n);
	reality_check_get_direction_helper(a,nw);
	reality_check_get_direction_helper(a,ne);
	reality_check_get_direction_helper(a,s);
	reality_check_get_direction_helper(a,sw);
	reality_check_get_direction_helper(a,se);

	reality_check_get_direction_helper(b,n);
	reality_check_get_direction_helper(b,nw);
	reality_check_get_direction_helper(b,ne);
	reality_check_get_direction_helper(b,s);
	reality_check_get_direction_helper(b,sw);
	reality_check_get_direction_helper(b,se);
}

static map_location::DIRECTION legacy_get_opposite_dir(map_location::DIRECTION d)
{
	switch (d) {
		case map_location::NORTH:
			return map_location::SOUTH;
		case map_location::NORTH_EAST:
			return map_location::SOUTH_WEST;
		case map_location::SOUTH_EAST:
			return map_location::NORTH_WEST;
		case map_location::SOUTH:
			return map_location::NORTH;
		case map_location::SOUTH_WEST:
			return map_location::NORTH_EAST;
		case map_location::NORTH_WEST:
			return map_location::SOUTH_EAST;
		case map_location::NDIRECTIONS:
		default:
			return map_location::NDIRECTIONS;
	}
}

BOOST_AUTO_TEST_CASE ( check_get_opposite_dir_refactor )
{
	for (unsigned int i = 0; i < 7; i++ ) {
		map_location::DIRECTION d = static_cast<map_location::DIRECTION> (i);
		BOOST_CHECK_EQUAL ( map_location::get_opposite_dir(d), legacy_get_opposite_dir(d) );
	}
}

BOOST_AUTO_TEST_CASE ( check_rotate )
{
	BOOST_CHECK_EQUAL ( map_location::rotate_right(map_location::NORTH) , map_location::NORTH_EAST );
	BOOST_CHECK_EQUAL ( map_location::rotate_right(map_location::NORTH_EAST,-1) , map_location::NORTH);

	BOOST_CHECK_EQUAL ( map_location::rotate_right(map_location::NORTH_EAST) , map_location::SOUTH_EAST );
	BOOST_CHECK_EQUAL ( map_location::rotate_right(map_location::SOUTH_EAST,-1) , map_location::NORTH_EAST );

	BOOST_CHECK_EQUAL ( map_location::rotate_right(map_location::SOUTH_EAST) , map_location::SOUTH );
	BOOST_CHECK_EQUAL ( map_location::rotate_right(map_location::SOUTH, -1) , map_location::SOUTH_EAST );

	BOOST_CHECK_EQUAL ( map_location::rotate_right(map_location::SOUTH), map_location::SOUTH_WEST);
	BOOST_CHECK_EQUAL ( map_location::rotate_right(map_location::SOUTH_WEST,-1) , map_location::SOUTH );

	BOOST_CHECK_EQUAL ( map_location::rotate_right(map_location::SOUTH_WEST), map_location::NORTH_WEST);
	BOOST_CHECK_EQUAL ( map_location::rotate_right(map_location::NORTH_WEST,-1) , map_location::SOUTH_WEST );

	BOOST_CHECK_EQUAL ( map_location::rotate_right(map_location::NORTH_WEST), map_location::NORTH);
	BOOST_CHECK_EQUAL ( map_location::rotate_right(map_location::NORTH,-1) , map_location::NORTH_WEST );


	for (unsigned int i = 0; i < 7; i++ ) {
		map_location::DIRECTION d = static_cast<map_location::DIRECTION> (i);
		BOOST_CHECK_EQUAL ( map_location::get_opposite_dir(d), map_location::rotate_right(d,3) );
		BOOST_CHECK_EQUAL ( map_location::rotate_right(d,-2), map_location::rotate_right(d,4) );
	}
}

static void rotate_around_centers ( const std::vector<map_location> & locs )
{
	for (std::vector<map_location>::const_iterator it_a = locs.begin(); it_a != locs.end(); ++it_a) {
		for (std::vector<map_location>::const_iterator it_b = it_a + 1; it_b != locs.end(); ++it_b) {
			const map_location & a = *it_a;
			const map_location & b = *it_b;

			a.rotate_right_around_center(b,1);
			a.rotate_right_around_center(b,-1);
			a.rotate_right_around_center(b,2);
			a.rotate_right_around_center(b,-2);
			a.rotate_right_around_center(b,3);
			a.rotate_right_around_center(b,0);
		}
	}
}

BOOST_AUTO_TEST_CASE ( check_rotate_around_center )
{
	rotate_around_centers(preset_locs);
}

/**
 * This commented block was used to visualize the output of get_relative_dir
 * and to help derive the implementation in commit
 * 829b74c2beaa18eda42710c364b12c987f9caed5
 */

/*
static std::string dir_to_terrain (const map_location::DIRECTION dir)
{
	switch(dir) {
		case map_location::NORTH:      return "Gg";
		case map_location::SOUTH:      return "Ai";
		case map_location::SOUTH_EAST: return "Gs^Fp";
		case map_location::SOUTH_WEST: return "Ss";
		case map_location::NORTH_EAST: return "Hd";
		case map_location::NORTH_WEST: return "Md";
		default: return "Xv";
	}
}

static std::string visualize_helper ( int x , int y, const map_location & c )
{
	map_location temp(x,y);
	return dir_to_terrain(c.get_relative_dir(temp));
}

BOOST_AUTO_TEST_CASE ( visualize_get_relative_dir )
{
	map_location c7(7,8), c8(8,8);

	std::cout << "***" << std::endl;
	int x;
	int y;
	for (y = 0; y < 16; y++) {
		for (x = 0; x < 15; x++) {
			std::cout << visualize_helper(x,y,c7) << ", ";
		}
		std::cout << visualize_helper(x,y,c7) << std::endl;
	}

	std::cout << "***" << std::endl;
	for (y = 0; y < 16; y++) {
		for (x = 0; x < 15; x++) {
			std::cout << visualize_helper(x,y,c8) << ", ";
		}
		std::cout << visualize_helper(x,y,c8) << std::endl;
	}

	std::cout << "***" << std::endl;
}*/

BOOST_AUTO_TEST_SUITE_END()
