/*
   Copyright (C) 2008 - 2017 by Pauli Nieminen <paniemin@cc.hut.fi>
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

#include <boost/test/unit_test.hpp>

#include <boost/test/unit_test_suite.hpp>


#include "key.hpp"
#include "filesystem.hpp"
#include "savegame.hpp"
#include "units/types.hpp"

#include <SDL.h>

#include "tests/utils/fake_event_source.hpp"
#include "tests/utils/fake_display.hpp"
#include "tests/utils/auto_parameterized.hpp"


// Linker workarounds end here

