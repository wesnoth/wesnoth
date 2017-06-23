/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Copyright (C) 2009 - 2017 by Ignacio R. Morelle <shadowm2006@gmail.com>
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
 * Storyscreen controller (implementation).
 */

#include "storyscreen/controller.hpp"
#include "storyscreen/part.hpp"

#include "log.hpp"
#include "resources.hpp"
#include "variable.hpp"

#include <cassert>

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)

namespace storyscreen
{
controller::controller(const vconfig& data, const std::string& scenario_name)
	: scenario_name_(scenario_name)
	, parts_()
{
	assert(resources::gamedata != nullptr && "Ouch: gamedata is nullptr when initializing storyscreen controller");
	resolve_wml(data);
}

bool controller::resolve_wml_helper(const std::string& key, const vconfig& node)
{
	bool found = false;

	if(key == "part" && !node.empty()) {
		part_pointer_type const story_part(new part(node));
		// Use scenario name as part title if the WML doesn't supply a custom one.
		if((*story_part).show_title() && (*story_part).title().empty()) {
			(*story_part).set_title(scenario_name_);
		}

		parts_.push_back(story_part);
		found = true;
	}

	return found;
}

} // end namespace storyscreen
