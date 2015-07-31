/*
   Copyright (C) 2009 - 2015 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "editor/editor_preferences.hpp"
#include "game_preferences.hpp"
#include "serialization/string_utils.hpp"
#include "util.hpp"

namespace preferences {

namespace editor {

	int auto_update_transitions() {
		return lexical_cast_default<int>(preferences::get("editor_auto_update_transitions"), TransitionUpdateMode::partial);
	}

	void set_auto_update_transitions(int value) {
		preferences::set("editor_auto_update_transitions", lexical_cast<std::string>(value));
	}

	std::string default_dir() {
		return preferences::get("editor_default_dir");
	}

	bool draw_terrain_codes() {
		return preferences::get("editor_draw_terrain_codes", false);
	}

	void set_draw_terrain_codes(bool value) {
		preferences::set("editor_draw_terrain_codes", value);
	}

	bool draw_hex_coordinates() {
		return preferences::get("editor_draw_hex_coordinates", false);
	}

	void set_draw_hex_coordinates(bool value) {
		preferences::set("editor_draw_hex_coordinates", value);
	}

	namespace {
		void normalize_editor_rgb(int rval)
		{
			if (rval < -255) {
				rval = -255;
			}
			else if (rval > 255) {
				rval = 255;
			}
		}
	}

	void set_tod_r(int value)
	{
		normalize_editor_rgb(value);
		preferences::set("editor_r",lexical_cast<std::string>(value));
	}

	void set_tod_g(int value)
	{
		normalize_editor_rgb(value);
		preferences::set("editor_g",lexical_cast<std::string>(value));
	}

	void set_tod_b(int value)
	{
		normalize_editor_rgb(value);
		preferences::set("editor_b",lexical_cast<std::string>(value));
	}

	int tod_r()
	{
		return lexical_cast_in_range<int>(preferences::get("editor_r"), 0, -255, 255);
	}

	int tod_g()
	{
		return lexical_cast_in_range<int>(preferences::get("editor_g"), 0, -255, 255);
	}

	int tod_b()
	{
		return lexical_cast_in_range<int>(preferences::get("editor_b"), 0, -255, 255);
	}

} //end namespace editor

} //end namespace preferences

