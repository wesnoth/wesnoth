/*
   Copyright (C) 2009 - 2013 by Eugen Jiresch
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SAVEGAME_CONFIG_HPP_INCLUDED
#define SAVEGAME_CONFIG_HPP_INCLUDED

/* interface for building a config from savegame related objects */
//FIXME: move to gamestate.hpp once dependencies between team and game_state are sorted
class config;

namespace savegame {

class savegame_config
{
public:
	virtual ~savegame_config() {};
	virtual config to_config() const = 0;
};

void write_music_play_list(config& snapshot);

void write_events(config& cfg);

}
#endif
