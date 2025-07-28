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

#ifdef __APPLE__

#include "apple_version.hpp"

#import "game_version.hpp"

#if defined(__APPLE__) && defined(__MACH__) && defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__)
#define __IPHONEOS__ (__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__*1000)
#endif

#import <Foundation/Foundation.h>

#if defined(__IPHONEOS__)
#import <UIKit/UIKit.h>
#endif

namespace desktop {
namespace apple {
	std::string os_version() {

		//
		// Standard Apple version
		//

		std::string version_string;
#if defined(__IPHONEOS__)
		version_string = "Apple iOS ";
#else
		if (@available(macOS 10.12, *)) {
			version_string = "Apple macOS ";
		} else {
			version_string = "Apple OS X ";
		}
#endif
		NSOperatingSystemVersion os_ver = [[NSProcessInfo processInfo] operatingSystemVersion];
		version_string += version_info(os_ver.majorVersion, os_ver.minorVersion, os_ver.patchVersion);

		return version_string;
	}

} // end namespace apple
} // end namespace desktop

#endif //end __APPLE__
