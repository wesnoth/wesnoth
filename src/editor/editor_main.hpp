/*
   Copyright (C) 2008 - 2017 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef EDITOR_EDITOR_MAIN_H_INCLUDED
#define EDITOR_EDITOR_MAIN_H_INCLUDED

#include <string>

class config;
class CVideo;

namespace editor {

enum EXIT_STATUS {
	EXIT_NORMAL,
	EXIT_QUIT_TO_DESKTOP,
	EXIT_RELOAD_DATA,
	EXIT_ERROR
};

/**
 * Main interface for launching the editor from the title screen.
 * @returns How the editor was exited (whether the intent was to
 *          go back to the titlescreen or quit to desktop altogether)
 */

EXIT_STATUS start(const config& game_config, CVideo& video, const std::string& filename = "", bool take_screenshot = false, const std::string& screenshot_filename = "map_screenshot.bmp");

} //end namespace editor

#endif
