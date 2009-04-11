/* $Id: ai_actions.cpp   $ */
/*
   Copyright (C) 2009 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * Managing the AI-Game interaction - AI actions and their results
 * @file ai_actions.cpp
 */

#include "ai_actions.hpp"

#define DBG_AI_ACTIONS LOG_STREAM(debug, ai_actions)
#define LOG_AI_ACTIONS LOG_STREAM(info, ai_actions)
#define WRN_AI_ACTIONS LOG_STREAM(warn, ai_actions)
#define ERR_AI_ACTIONS LOG_STREAM(err, ai_actions)

// =======================================================================
// 
// =======================================================================

