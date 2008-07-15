/* $Id$ */
/*
   Copyright (C) 2008 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "editor_main.hpp"
#include "editor_common.hpp"
#include "editor_controller.hpp"

namespace editor2 {

EXIT_STATUS start(config& game_conf, CVideo& video)
{
	editor_controller editor(game_conf, video);
	return editor.main_loop();
}

} //end namespace editor2
