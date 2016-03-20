/*
   Copyright (C) 2009 - 2016 by Yurii Chernyi <terraninfo@terraninfo.net>
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
 * Defensive fallback, to be used during unfavorable conditions
 */

#include "ca_global_fallback.hpp"

#include "ai/composite/ai.hpp"

#include "game_display.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "map/label.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "sdl/utils.hpp"
#include "team.hpp"
#include "terrain/filter.hpp"
#include "tod_manager.hpp"
#include "units/unit.hpp"

#include <boost/foreach.hpp>

namespace ai {

namespace testing_ai_default {

static lg::log_domain log_ai_testing_ca_global_fallback("ai/ca/global_fallback");
#define DBG_AI LOG_STREAM(debug, log_ai_testing_ca_global_fallback)
#define LOG_AI LOG_STREAM(info, log_ai_testing_ca_global_fallback)
#define WRN_AI LOG_STREAM(warn, log_ai_testing_ca_global_fallback)
#define ERR_AI LOG_STREAM(err, log_ai_testing_ca_global_fallback)

//==================================
// aux utils
//

static void display_label(int /*side*/, const map_location& location, const std::string& text, bool surrounded)
{
	display* gui = display::get_singleton();
	std::string team_name;

	SDL_Color color = int_to_color(team::get_side_rgb(surrounded ? 2 : 1 ) );//@fixme: for tests

	const terrain_label *res;
	res = gui->labels().set_label(location, text, surrounded, team_name, color);
	if (res && resources::recorder)
		resources::recorder->add_label(res);
}


//==================================

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
	LOG_AI << "start" << std::endl;
	const int ticks = SDL_GetTicks();
	double res_sum = 0;
	BOOST_FOREACH( unit &u, *resources::units)
	{
		if (u.side()!=get_side())
		{
			continue;
		}
		double res = 0.1; //@todo: how badly the unit 'u' will be hurt by enemy
		res_sum += res;
		display_label(get_side(),u.get_location(),str_cast(res),false);
	}
	LOG_AI << "sum is " << res_sum << std::endl;

	LOG_AI << "end in [" << (SDL_GetTicks()-ticks) << "] ticks" << std::endl;
}

} // end of namespace testing_ai_default

} // end of namespace ai
