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
#if TARGET_OS_OSX

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

#elif TARGET_OS_IOS

#include "apple_video.hpp"

#import <UIKit/UIKit.h>

namespace desktop {
namespace apple {
	CGFloat get_scale_factor(int display_index) {
		NSArray<UIScreen*>* screens = [UIScreen screens];
		if(display_index >= 0 && static_cast<NSUInteger>(display_index) < screens.count) {
			return screens[display_index].scale;
		}
		return UIScreen.mainScreen.scale;
	}
} // end namespace apple
} // end namespace desktop

#endif //end TARGET_OS_OSX / TARGET_OS_IOS
#endif //end __APPLE__
