/*
	Copyright (C) 2020 - 2024
	by Martin Hrubý <hrubymar10@gmail.com>
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

#include <CoreGraphics/CoreGraphics.h>

namespace desktop {
	namespace apple {
		CGFloat get_scale_factor(int display_index);
	} // end namespace apple
} // end namespace desktop
