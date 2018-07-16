/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "help/topic.hpp"

class config;

namespace help
{
topic_list generate_ability_topics(bool sort_generated);
topic_list generate_weapon_special_topics(bool sort_generated);
topic_list generate_time_of_day_topics(bool sort_generated);
topic_list generate_trait_topics(bool sort_generated);
topic_list generate_unit_topics(bool sort_generated, const std::string& race);
topic_list generate_faction_topics(bool sort_generated, const config& era);
topic_list generate_era_topics(bool sort_generated, const std::string& era_id);

} // namespace help
