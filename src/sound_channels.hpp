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

struct sound_channels_defines
{
    static constexpr const char* music_tag = "music";
    static constexpr const char* sound_bell_tag = "bell";
    static constexpr const char* sound_timer_tag = "sound_timer";
    static constexpr const char* sound_source_tag = "sound_source";
    static constexpr const char* sound_ui_tag = "sound_ui";
    static constexpr const char* sound_fx_tag = "sound_fx";

	ENUM_AND_ARRAY(music_tag, sound_bell_tag, sound_timer_tag, sound_source_tag, sound_ui_tag, sound_fx_tag)
};
using sound_channels = string_enums::enum_base<sound_channels_defines>;
