/*
   Copyright (C) 2009 - 2018 by Tomasz Sniatowski <kailoran@gmail.com>
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

#include <string>
#include <vector>

namespace preferences {

namespace editor {

	enum TRANSITION_UPDATE_MODE : int {
		TRANSITION_UPDATE_OFF = 0,
		TRANSITION_UPDATE_ON = 1,
		TRANSITION_UPDATE_PARTIAL = 2,
		TRANSITION_UPDATE_COUNT = 3
	};

	int auto_update_transitions();
	void set_auto_update_transitions(int value);

	//std::vector<std::string>* get_editor_history();

	std::string default_dir();

	bool draw_terrain_codes();
	void set_draw_terrain_codes(bool value);

	bool draw_hex_coordinates();
	void set_draw_hex_coordinates(bool value);

	bool draw_num_of_bitmaps();
	void set_draw_num_of_bitmaps(bool value);

	/** Retrieves the list of recently opened files. */
	std::vector<std::string> recent_files();
	/** Adds an entry to the recent files list. */
	void add_recent_files_entry(const std::string& path);
	/** Removes a single entry from the recent files list. */
	void remove_recent_files_entry(const std::string& path);

} //end namespace editor

} //end namespace preferences
