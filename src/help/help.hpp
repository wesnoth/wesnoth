/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include <string>

class terrain_type;
class unit;
class unit_type;

namespace help
{
/** Flags the help manager's contents for regeneration. */
void reset();

/**
 * Open the help browser, show topic with id show_topic.
 *
 * If show_topic is the empty string, the default topic will be shown.
 */
void show_help(const std::string& show_topic = "");

/**
 * Open the help browser, show unit with id unit_id.
 *
 * If show_topic is the empty string, the default topic will be shown.
 */
void show_unit_help(const std::string& unit_id, bool has_variations = false, bool hidden = false);

/**
 * Open the help browser, show the variation of the unit matching.
 */
void show_variation_help(const std::string& unit_id, const std::string& variation, bool hidden = false);

/**
 * Open the help browser, show terrain with id terrain_id.
 *
 * If show_topic is the empty string, the default topic will be shown.
 */
void show_terrain_help(const std::string& unit_id, bool hidden = false);

void show_unit_description(const unit_type& t);
void show_unit_description(const unit& u);
void show_terrain_description(const terrain_type& t);

} // End namespace help.
