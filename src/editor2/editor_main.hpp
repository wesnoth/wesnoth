/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef EDITOR2_EDITOR_MAIN_H_INCLUDED
#define EDITOR2_EDITOR_MAIN_H_INCLUDED

class config;
class CVideo;

namespace editor2 {
	
enum EXIT_STATUS {
	EXIT_NORMAL,
	EXIT_QUIT_TO_DESKTOP,
	EXIT_ERROR
};

/** 
 * Main interface for launching the editor from the title screen.
 * @returns How the editor was exited (whether the intent was to 
 *          go back to the titlescreen or quit to desktop altogeter)
 */

EXIT_STATUS start(config& game_config, CVideo& video, const std::string& filename = "");
	
} //end namespace editor2

#endif
