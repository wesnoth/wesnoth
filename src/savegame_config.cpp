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

#include "sound.hpp"
#include "soundsource.hpp"
#include "game_events.hpp"

namespace savegame {

void write_music_play_list(config& snapshot) {
	sound::write_music_play_list(snapshot);
}

void write_events(config& cfg) {
	game_events::write_events(cfg);
}

}
