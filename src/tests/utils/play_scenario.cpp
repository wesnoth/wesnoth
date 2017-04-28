/*
   Copyright (C) 2008 - 2017 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of thie Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-test"

#include "tests/utils/play_scenario.hpp"
#include "tests/utils/game_config_manager.hpp"
#include "tests/utils/fake_display.hpp"

#include "config_assign.hpp"
#include "game_display.hpp"
#include "saved_game.hpp"
#include "game_initialization/playcampaign.hpp"
#include "terrain/type_data.hpp"
#include "units/unit.hpp"
#include "units/map.hpp"
