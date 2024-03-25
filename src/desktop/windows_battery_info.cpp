/*
	Copyright (C) 2018 - 2024
	by Jyrki Vesterinen <sandgtx@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "desktop/windows_battery_info.hpp"

#include <windows.h>

bool windows_battery_info::does_device_have_battery()
{
	SYSTEM_POWER_STATUS power_status;
	BOOL success = GetSystemPowerStatus(&power_status);
	if(success) {
		return !(power_status.BatteryFlag & 128);
	} else {
		return false;
	}
}

double windows_battery_info::get_battery_percentage()
{
	SYSTEM_POWER_STATUS power_status;
	BOOL success = GetSystemPowerStatus(&power_status);
	if(success) {
		return power_status.BatteryLifePercent;
	} else {
		return 0.0;
	}
}
