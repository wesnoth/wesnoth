/*
   Copyright (C) 2009 - 2017 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef EDITOR_PREFERENCES_HPP_INCLUDED
#define EDITOR_PREFERENCES_HPP_INCLUDED

#include <string>
#include <vector>

namespace preferences {

namespace editor {

	namespace TransitionUpdateMode {
		const int off = 0;
		const int on = 1;
		const int partial = 2;
		const int count = 3;
	}

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


#endif
