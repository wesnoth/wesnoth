/*
	Copyright (C) 2018 - 2025
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

#include "battery_info.hpp"

#ifdef _WIN32
#include "desktop/windows_battery_info.hpp"
#endif

#ifdef __APPLE__
#include "apple_battery_info.hpp"
#endif

#ifdef HAVE_LIBDBUS
#include "desktop/dbus_features.hpp"
#endif

#ifdef __ANDROID__
#include <SDL2/SDL_system.h> // For SDL Android functions
#include <jni.h>
#endif

namespace desktop {
namespace battery_info {

bool does_device_have_battery()
{
#if defined(_WIN32)
	return windows_battery_info::does_device_have_battery();
#elif defined(__APPLE__)
	return desktop::battery_info::apple::does_device_have_battery();
#elif defined(HAVE_LIBDBUS)
	return dbus::does_device_have_battery();
#elif defined(__ANDROID__)
	return true;
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
#elif defined(HAVE_LIBDBUS)
	return dbus::get_battery_percentage();
#elif defined(__ANDROID__)
	// call the helper method WesnothActivity.getBatteryPercentage() using JNI
	JNIEnv* env = reinterpret_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
	jobject wesnoth_instance = reinterpret_cast<jobject>(SDL_AndroidGetActivity());
	jclass wesnoth_activity(env->GetObjectClass(wesnoth_instance));
	jmethodID percentage = env->GetMethodID(wesnoth_activity, "getBatteryPercentage", "()D");
	return env->CallDoubleMethod(wesnoth_instance, percentage);
#else
	return -1;
#endif
}

} // end namespace battery_info
} // end namespace desktop
