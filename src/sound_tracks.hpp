/*
	Copyright (C) 2008 - 2025
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

#include "enum_base.hpp"

struct sound_tracks_defines
{
    static constexpr const char* music = "music";
    static constexpr const char* sound_bell = "bell";
    static constexpr const char* sound_timer = "sound_timer";
    static constexpr const char* sound_source = "sound_source";
    static constexpr const char* sound_ui = "sound_ui";
    static constexpr const char* sound_fx = "sound_fx";

	ENUM_AND_ARRAY(music, sound_bell, sound_timer, sound_source, sound_ui, sound_fx)
};
using sound_tracks = string_enums::enum_base<sound_tracks_defines>;
