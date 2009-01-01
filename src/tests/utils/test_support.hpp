/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of thie Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef TESTS_UTILS_TEST_SUPPORT_HPP_INCLUDED
#define TESTS_UTILS_TEST_SUPPORT_HPP_INCLUDED

#include <boost/version.hpp>

#if BOOST_VERSION < 103400
// 1.33 doesn't provide namespace for test suite so fix that
#include <boost/test/auto_unit_test.hpp>
#include "tests/utils/boost_unit_test_suite_1_34_0.hpp"
#else // BOOST_VERSION >= 103400
#include <boost/test/unit_test.hpp>


#endif

#endif
