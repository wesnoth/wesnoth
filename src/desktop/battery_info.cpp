/*
 Copyright (C) 2018 by Martin Hrubý <hrubymar10@gmail.com>
 Part of the Battle for Wesnoth Project https://www.wesnoth.org/
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.
 
 See the COPYING file for more details.
 */

#include "battery_info.hpp"

#ifdef _WIN32
#include "desktop/windows_battery_info.hpp"
#endif

#ifdef __APPLE__
#include "apple_battery_info.hpp"
#endif

namespace desktop {
namespace battery_info {

bool does_device_have_battery()
{
#if defined(_WIN32)
	return windows_battery_info::does_device_have_battery();
#elif defined(__APPLE__)
	return desktop::battery_info::apple::does_device_have_battery();
#else
	return false;
#endif
}

double get_battery_percentage()
{
#if defined(_WIN32)
	return windows_battery_info::get_battery_percentage();
#elif defined(__APPLE__)
	return desktop::battery_info::apple::get_battery_percentage();
#else
	return -1;
#endif
}

} // end namespace battery_info
} // end namespace desktop
