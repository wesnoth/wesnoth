/*
   Copyright (C) 2015 - 2016 by the Battle for Wesnoth Project

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
#include "../scripting/lua_kernel_base.hpp"

#include "lua/lualib.h"
#include "lua/lauxlib.h"
#include "lua/llimits.h"
#ifndef _WIN32
# include <fenv.h>
#endif

BOOST_AUTO_TEST_SUITE( lua )

static int lua_round(lua_State * L, double d)
{
	lua_pushnumber(L, d);
	int res = luaL_checkinteger(L, -1);
	lua_pop(L, 1);
	return res;
}

static int lua_round2(double d)
{
	int res;
	lua_number2int(res, d);
	return res;
}

static int lua_round3(double d)
{
	LUAI_EXTRAIEEE
    volatile union luai_Cast u; u.l_d = (d) + 6755399441055744.0;
    return u.l_p[LUA_IEEEENDIANLOC];
}

BOOST_AUTO_TEST_CASE(fpu_rounding)
{
#ifndef _WIN32
	BOOST_CHECK_EQUAL(fegetround(), FE_TONEAREST);
#endif
	double d1 = 0.6;
	double d2 = 6755399441055744.0;
	BOOST_CHECK_EQUAL(d1 + d2, 6755399441055745.0);
}

BOOST_AUTO_TEST_CASE(lua_rounding)
{
	lua_kernel_base kernel(NULL);
	lua_State * L = kernel.get_state();

	BOOST_CHECK_EQUAL(lua_round(L, -2.5),  -2);
	BOOST_CHECK_EQUAL(lua_round(L, -1.5),  -2);
	BOOST_CHECK_EQUAL(lua_round(L, -1  ),  -1);
	BOOST_CHECK_EQUAL(lua_round(L, -0.5),   0);
	BOOST_CHECK_EQUAL(lua_round(L,  0.4),   0);
	BOOST_CHECK_EQUAL(lua_round(L,  0.5),   0);
	BOOST_CHECK_EQUAL(lua_round(L,  0.6),   1);
	BOOST_CHECK_EQUAL(lua_round(L,  1.5),   2);
	BOOST_CHECK_EQUAL(lua_round(L,  1.6),   2);
	BOOST_CHECK_EQUAL(lua_round(L,  2  ),   2);
	BOOST_CHECK_EQUAL(lua_round(L,  2.5),   2);
	BOOST_CHECK_EQUAL(lua_round(L,  2.6),   3);
	BOOST_CHECK_EQUAL(lua_round(L,  3.5),   4);

	
	BOOST_CHECK_EQUAL(lua_round2(-1.5),  -2);
	BOOST_CHECK_EQUAL(lua_round2(-1.5),  -2);
	BOOST_CHECK_EQUAL(lua_round3( 0.6),   1);
	BOOST_CHECK_EQUAL(lua_round3( 0.6),   1);
}

BOOST_AUTO_TEST_SUITE_END()
