/*
   Copyright (C) 2009 by Eugen Jiresch
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "sound.hpp"

namespace savegame {

void write_music_play_list(config& snapshot) {
	sound::write_music_play_list(snapshot);
}

}
