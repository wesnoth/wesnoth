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

#pragma once

namespace desktop {
namespace battery_info {

/// @return true if the device has a battery, false otherwise.
bool does_device_have_battery();
/// @return battery charge as a number between 0 and 100.
double get_battery_percentage();

} // end namespace battery_info
} // end namespace desktop
