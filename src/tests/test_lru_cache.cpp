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

#include "utils/lru_cache.hpp"
#include "log.hpp"

/*
./test --report_level=detailed --log_level=all --run_test=lru_cache_suite

 */

BOOST_AUTO_TEST_SUITE( lru_cache_suite )

namespace{
static const uint CACHE_SIZE = 10;
static const uint STEP_SIZE = 1000;
	
void check_equal(int l, int r) {
	int ll(l + STEP_SIZE);
	BOOST_CHECK_MESSAGE(ll == r, "\nLeft \""<< ll <<"\"" << "should equal right   \""<<r<<"\"\n"); }
void check_not_equal(int l, int r) {
	int ll(l + STEP_SIZE);
	BOOST_CHECK_MESSAGE(ll != r, "\nLeft \""<< ll <<"\"" << "should not equal right   \""<<r<<"\"\n"); }

struct t_gen_cache_item {
	int operator()(int const & in) const {
		return in + STEP_SIZE ; }
};
}

BOOST_AUTO_TEST_CASE( test_lru_cache1 ) {


typedef  n_lru_cache::t_lru_cache<int, int, t_gen_cache_item> t_cache;

static t_cache cache( t_gen_cache_item(), CACHE_SIZE);


 for (int i = 1; i<20; ++i){
	 int from_cache(cache.check( i ));
	 int hit_me(cache.check( 1 ));
	
	 cache.invalidate(4);
	 
	 check_equal(1, hit_me);
	 check_equal(i, from_cache);
 }
 
} 

/* vim: set ts=4 sw=4: */
BOOST_AUTO_TEST_SUITE_END()

