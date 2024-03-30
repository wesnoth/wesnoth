/*
	Copyright (C) 2014 - 2024
	by Google Inc.
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

#include "desktop/apple_notification.hpp"

#import <Foundation/Foundation.h>

namespace apple_notifications {

bool available() {
	Class notificationClass = NSClassFromString(@"NSUserNotificationCenter");
	if(notificationClass) {
		return true;
	}
	return false;
}

void send_cocoa_notification(const std::string& owner, const std::string& message);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
void send_notification(const std::string& owner, const std::string& message, const desktop::notifications::type note_type) {
    @autoreleasepool {
        Class appleNotificationClass = NSClassFromString(@"NSUserNotificationCenter");
        if (appleNotificationClass) {
            send_cocoa_notification(owner, message);
        }
    }
}
#pragma clang diagnostic pop

void send_cocoa_notification(const std::string& owner, const std::string& message) {
    NSString *title = [NSString stringWithCString:owner.c_str() encoding:NSUTF8StringEncoding];
    NSString *description = [NSString stringWithCString:message.c_str() encoding:NSUTF8StringEncoding];
    NSUserNotification *notification = [[NSUserNotification alloc] init];
    notification.title = title;
    notification.informativeText = description;
    notification.deliveryDate = [NSDate date];

    [[NSUserNotificationCenter defaultUserNotificationCenter] scheduleNotification:notification];
}

}
#endif //end __APPLE__
