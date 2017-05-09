/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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

class config;
class display;
class terrain_type;
class unit;
class unit_type;
class CVideo;
#include <string>

namespace help {

struct help_manager {
	help_manager(const config *game_config);
	~help_manager();
};

struct section;
/// Open a help dialog using a toplevel other than the default. This
/// allows for complete customization of the contents, although not in a
/// very easy way.
void show_help(CVideo& video, const section &toplevel, const std::string& show_topic="",
			   int xloc=-1, int yloc=-1);

/// Open the help browser. The help browser will have the topic with id
/// show_topic open if it is not the empty string. The default topic
/// will be shown if show_topic is the empty string.
void show_help(CVideo& video, const std::string& show_topic="", int xloc=-1, int yloc=-1);

/// wrapper to add unit prefix and hiding symbol
void show_unit_help(CVideo& video, const std::string& unit_id, bool has_variations=false,
				bool hidden = false, int xloc=-1, int yloc=-1);

/// wrapper to add variation prefix and hiding symbol
void show_variation_help(CVideo& video, const std::string &unit_id, const std::string &variation,
				bool hidden = false, int xloc=-1, int yloc=-1);

/// wrapper to add terrain prefix and hiding symbol
void show_terrain_help(CVideo& video, const std::string& unit_id, bool hidden = false,
				int xloc = -1, int yloc = -1);

void show_unit_description(CVideo& video, const unit_type &t);
void show_unit_description(CVideo& video, const unit &u);
void show_terrain_description(CVideo& video, const terrain_type& t);

} // End namespace help.
