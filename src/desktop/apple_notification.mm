/*
 Copyright (C) 2014 - 2016 by Google Inc.
 Part of the Battle for Wesnoth Project http://www.wesnoth.org/
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.
 
 See the COPYING file for more details.
*/

#ifdef __APPLE__

#include "apple_notification.hpp"

#import <Foundation/Foundation.h>

#if (defined MAC_OS_X_VERSION_10_8) && (MAC_OS_X_VERSION_10_8 <= MAC_OS_X_VERSION_MAX_ALLOWED)
#define HAVE_NS_USER_NOTIFICATION
#endif

#ifdef HAVE_GROWL
#import "Growl/Growl.h"

@interface WesnothGrowlDelegate : NSObject <GrowlApplicationBridgeDelegate>
@end

@implementation WesnothGrowlDelegate
// Empty delegate to interact with Growl. Implement protocol if we want to handle messages from Growl.
@end
#endif

namespace apple_notifications {

#ifdef HAVE_NS_USER_NOTIFICATION
void send_cocoa_notification(const std::string& owner, const std::string& message);
#endif
#ifdef HAVE_GROWL
void send_growl_notification(const std::string& owner, const std::string& message, const desktop::notifications::type note_type);
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#ifdef HAVE_NS_USER_NOTIFICATION
void send_notification(const std::string& owner, const std::string& message, const desktop::notifications::type note_type) {
    @autoreleasepool {
        Class appleNotificationClass = NSClassFromString(@"NSUserNotificationCenter");
        if (appleNotificationClass) {
            send_cocoa_notification(owner, message);
        } else {
#ifdef HAVE_GROWL
            send_growl_notification(owner, message, note_type);
#endif
        }
    }
}
#else // HAVE_NS_USER_NOTIFICATION
void send_notification(const std::string& owner, const std::string& message, const desktop::notifications::type note_type) {
    @autoreleasepool {
#ifdef HAVE_GROWL
        send_growl_notification(owner, message, note_type);
#endif
    }
}
#endif //end else HAVE_NS_USER_NOTIFICATION
#pragma clang diagnostic pop
  
#ifdef HAVE_NS_USER_NOTIFICATION
void send_cocoa_notification(const std::string& owner, const std::string& message) {
    NSString *title = [NSString stringWithCString:owner.c_str() encoding:NSUTF8StringEncoding];
    NSString *description = [NSString stringWithCString:message.c_str() encoding:NSUTF8StringEncoding];
    NSUserNotification *notification = [[NSUserNotification alloc] init];
    notification.title = title;
    notification.informativeText = description;
    notification.deliveryDate = [NSDate date];
    
    [[NSUserNotificationCenter defaultUserNotificationCenter] scheduleNotification:notification];
}
#endif //end HAVE_NS_USER_NOTIFICATION

#ifdef HAVE_GROWL
void send_growl_notification(const std::string& owner, const std::string& message, const desktop::notifications::type note_type) {
    static WesnothGrowlDelegate *delegate = nil;
    if (!delegate) {
        delegate = [[WesnothGrowlDelegate alloc] init];
        [GrowlApplicationBridge setGrowlDelegate:delegate];
    }
    
    NSString *notificationName = @"";
    switch (note_type) {
        case desktop::notifications::CHAT:
            notificationName = @"Chat Message";
            break;
        case desktop::notifications::TURN_CHANGED:
            notificationName = @"Turn Changed";
            break;
        case desktop::notifications::OTHER:
            notificationName = @"Wesnoth";
            break;
    }
    NSString *title = [NSString stringWithCString:owner.c_str() encoding:NSUTF8StringEncoding];
    NSString *description = [NSString stringWithCString:message.c_str() encoding:NSUTF8StringEncoding];
    [GrowlApplicationBridge notifyWithTitle:title
                                description:description
                           notificationName:notificationName
                                   iconData:nil
                                   priority:0
                                   isSticky:NO
                               clickContext:nil];
}
#endif //end HAVE_GROWL

}
#endif //end __APPLE__
