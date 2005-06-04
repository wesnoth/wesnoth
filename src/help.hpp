/*
   Copyright (C) 2003 by David White <davidnwhite@comcast.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef HELP_HPP_INCLUDED
#define HELP_HPP_INCLUDED

#include <string>
#include <vector>

class config;
class display;
struct game_data;
class gamemap;

namespace help {

struct help_manager {
	help_manager(const config *game_config, const game_data *game_info, gamemap *map);
	~help_manager();
};

struct section;

/// Open a help dialog showing the topics with ids topics_to_show and
/// the sections with ids sections_to_show. Subsections and subtopics of
/// the sections will be added recursively according to the help config.
void show_help(display &disp, const std::vector<std::string> &topics_to_show,
			   const std::vector<std::string> &sections_to_show, const std::string show_topic="",
			   int xloc=-1, int yloc=-1);

/// Open a help dialog using a toplevel other than the default. This
/// allows for complete customization of the contents, although not in a
/// very easy way.
void show_help(display &disp, const section &toplevel, const std::string show_topic="",
			   int xloc=-1, int yloc=-1);

/// Open the help browser. The help browser will have the topic with id
/// show_topic open if it is not the empty string. The default topic
/// will be shown if show_topic is the empty string.
void show_help(display &disp, const std::string show_topic="", int xloc=-1, int yloc=-1);

} // End namespace help.

#endif
