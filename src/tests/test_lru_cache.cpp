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
	

struct t_gen_cache_item {
	int operator()(int const & in) const {
		return in + STEP_SIZE ; }
};

typedef  n_lru_cache::t_lru_cache<int, int, t_gen_cache_item> t_cache;

void check_equal(int l, int r) {
	int ll(l + STEP_SIZE);
	BOOST_CHECK_MESSAGE(ll == r, "\nLeft \""<< ll <<"\"" << "should equal right   \""<<r<<"\"\n"); }
void check_not_equal(int l, int r) {
	int ll(l + STEP_SIZE);
	BOOST_CHECK_MESSAGE(ll != r, "\nLeft \""<< ll <<"\"" << "should not equal right   \""<<r<<"\"\n"); }
 void check_present(int k, t_cache const & cache) {
	 bool t = cache.debugging_test_cache(k);
	BOOST_CHECK_MESSAGE(t, "\nExpected Key \""<< k <<"\"" << " is missing\n"); }
 void check_not_present(int k, t_cache  const &cache) {
	 bool t = cache.debugging_test_cache(k);
	BOOST_CHECK_MESSAGE(!t, "\nUnexpected Key \""<< k <<"\"" << " is present\n"); }
 void check_empty(t_cache const & cache){
	 std::pair<bool, int> newest = cache.debugging_newest_item();
	 bool t = ! newest.first ;
	 BOOST_CHECK_MESSAGE(t, "\nExpected an empty cache, but it isn't ");}
 void check_newest(int k, t_cache const & cache) {
	 std::pair<bool, int> newest = cache.debugging_newest_item();
	 bool t = newest.first && k == newest.second;
	 BOOST_CHECK_MESSAGE(t, "\nExpected newest item to be \""<< k <<"\"" << "but found  \""<<newest.second<<"\"\n"); }
 void check_oldest(int k, t_cache const & cache) {
	 std::pair<bool, int>  oldest = cache.debugging_oldest_item();
	 bool t = oldest.first && k == oldest.second;
	 BOOST_CHECK_MESSAGE(t, "\nExpected oldest item to be \""<< k <<"\"" << "but found  \""<<oldest.second<<"\"\n"); }

}

BOOST_AUTO_TEST_CASE( test_lru_cache1 ) {



static t_cache cache( t_gen_cache_item(), CACHE_SIZE);


 check_empty(cache);
 cache.invalidate();
 check_empty(cache);
 for (unsigned int i = 1; i<20; ++i){
	 int from_cache(cache.check( i ));
	 check_newest(i, cache);

	 int hit_me(cache.check( 1 ));
	 check_newest(1, cache);
	 
	 check_oldest(
				  (i<2) ? i :
				  (i<=CACHE_SIZE + 1) ?  2 : 
				  ( i<=CACHE_SIZE + 2) ? (i - CACHE_SIZE + 1) : (i - CACHE_SIZE + 2) 
				  , cache);
	
	 cache.invalidate(4);
	 check_not_present(4, cache);
	 
	 check_equal(1, hit_me);
	 check_equal(i, from_cache);
 }
 cache.invalidate();
 check_empty(cache);
 
} 

/* vim: set ts=4 sw=4: */
BOOST_AUTO_TEST_SUITE_END()

