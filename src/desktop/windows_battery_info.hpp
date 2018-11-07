/*
 Copyright (C) 2018 by Jyrki Vesterinen <sandgtx@gmail.com>
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

class windows_battery_info
{
public:
	static bool does_device_have_battery();
	static double get_battery_percentage();
};
