/*
	Copyright (C) 2020 - 2025
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
#include <TargetConditionals.h>
#ifdef TARGET_OS_OSX

#include "apple_video.hpp"

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

namespace desktop {
namespace apple {
	CGFloat get_scale_factor(int display_index) {
		CGFloat scale_factor = 1.0f;

		NSArray *screens = [NSScreen screens];

		if ([screens[display_index] respondsToSelector:@selector(backingScaleFactor)]) {  // Mac OS X 10.7 and later
			scale_factor = [screens[display_index] backingScaleFactor];
		}

		return scale_factor;
	}
} // end namespace apple
} // end namespace desktop

#endif //end TARGET_OS_OSX
#endif //end __APPLE__
