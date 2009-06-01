/* $Id$ */
/*
   Copyright (C) 2005 - 2009 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playlevel Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file config_adapter.cpp
 * Construct objects like 'team' or 'unit' out of WML-based config-infos.
 */

#include "global.hpp"
#include "config_adapter.hpp"

#include "game_preferences.hpp"
#include "gamestatus.hpp"
#include "log.hpp"
#include "map.hpp"
#include "unit.hpp"
#include "wml_exception.hpp"
#include "formula_string_utils.hpp"

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)
