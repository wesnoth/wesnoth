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

#ifdef __APPLE__

#include "apple_version.hpp"

#import "../version.hpp"

#if defined(__APPLE__) && defined(__MACH__) && defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__)
#define __IPHONEOS__ (__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__*1000)
#endif

#if defined(__IPHONEOS__)
//TODO: Implement iOS version detection
#else
#import <Foundation/Foundation.h>
#endif

namespace desktop {
	namespace apple {
		std::string os_version() {
#if defined(__IPHONEOS__)
			//TODO: Implement iOS version detection
#else
			std::string version_string = "Apple";
			NSArray *version_array = [[[NSProcessInfo processInfo] operatingSystemVersionString] componentsSeparatedByString:@" "];
			
			const version_info version_info([[version_array objectAtIndex:1] UTF8String]);
			
			if (version_info.major_version() == 10 && version_info.minor_version() < 12) {
				version_string += " OS X ";
			} else {
				version_string += " macOS ";
			}
			
			version_string += [[version_array objectAtIndex:1] UTF8String];
			version_string += " (";
			version_string += [[version_array objectAtIndex:3] UTF8String];
			
			return version_string;
#endif
		}
		
	} // end namespace apple
} // end namespace desktop

#endif //end __APPLE__
