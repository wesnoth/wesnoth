/*
	Copyright (C) 2008 - 2025
	by Pauli Nieminen <paniemin@cc.hut.fi>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-test"

#include "tests/utils/fake_display.hpp"

#include "video.hpp"

namespace test_utils
{
void set_test_resolution(const int width, const int height)
{
	static bool video_initialized = false;

	if(!video_initialized) {
		video_initialized = true;
		video::init(video::fake::no_draw);
	}

	if(width >= 0 && height >= 0) {
		video::set_resolution({width, height});
	}
}

} // namespace test_utils
