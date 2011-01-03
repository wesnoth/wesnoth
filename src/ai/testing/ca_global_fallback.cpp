/* $Id$ */
/*
   Copyright (C) 2009 - 2011 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Defensive fallback, to be used during unfavourable conditions
 */

#include "ca_global_fallback.hpp"

#include "../composite/ai.hpp"

#include "../../log.hpp"

/*
#include "../actions.hpp"
#include "../../foreach.hpp"
#include "../../map.hpp"
#include "../../resources.hpp"
#include "../../team.hpp"

#include <deque>
*/

namespace ai {

namespace testing_ai_default {

static lg::log_domain log_ai_testing_ca_global_fallback("ai/ca/global_fallback");
#define DBG_AI LOG_STREAM(debug, log_ai_testing_ca_global_fallback)
#define LOG_AI LOG_STREAM(info, log_ai_testing_ca_global_fallback)
#define WRN_AI LOG_STREAM(warn, log_ai_testing_ca_global_fallback)
#define ERR_AI LOG_STREAM(err, log_ai_testing_ca_global_fallback)


global_fallback_phase::global_fallback_phase( rca_context &context, const config &cfg )
	: candidate_action(context,cfg)
{
}


global_fallback_phase::~global_fallback_phase()
{
}


double global_fallback_phase::evaluate()
{
	return get_score();
}


void global_fallback_phase::execute()
{
	//@todo: implement
}

} // end of namespace testing_ai_default

} // end of namespace ai
