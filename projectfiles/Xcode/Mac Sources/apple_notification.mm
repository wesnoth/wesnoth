/*
 Copyright (C) 2014 by Google Inc.
 Part of the Battle for Wesnoth Project http://www.wesnoth.org/
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.
 
 See the COPYING file for more details.
*/

#include "apple_notification.hpp"

#import <Foundation/Foundation.h>
#if HAVE_GROWL
#import "Growl/Growl.h"

@interface WesnothGrowlDelegate : NSObject <GrowlApplicationBridgeDelegate>
@end

@implementation WesnothGrowlDelegate
// Empty delegate to interact with Growl. Implement protocol if we want to handle messages from Growl.
@end
#endif

namespace apple_notifications {

void send_cocoa_notification(const std::string& owner, const std::string& message);
void send_growl_notification(const std::string& owner, const std::string& message, const std::string& note_name);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
void send_notification(const std::string& owner, const std::string& message, const std::string& note_name) {
    Class appleNotificationClass = NSClassFromString(@"NSUserNotificationCenter");
    if (appleNotificationClass) {
        send_cocoa_notification(owner, message);
    } else {
#if HAVE_GROWL
        send_growl_notification(owner, message, note_name);
#endif
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

#if HAVE_GROWL
void send_growl_notification(const std::string& owner, const std::string& message, const std::string& note_name) {
    static WesnothGrowlDelegate *delegate = nil;
    if (!delegate) {
        delegate = [[WesnothGrowlDelegate alloc] init];
        [GrowlApplicationBridge setGrowlDelegate:delegate];
    }
    
    NSString *title = [NSString stringWithCString:owner.c_str() encoding:NSUTF8StringEncoding];
    NSString *description = [NSString stringWithCString:message.c_str() encoding:NSUTF8StringEncoding];
    NSString *notificationName = [NSString stringWithCString:note_name.c_str() encoding:NSUTF8StringEncoding];
    [GrowlApplicationBridge notifyWithTitle:title
                                description:description
                           notificationName:notificationName
                                   iconData:nil
                                   priority:0
                                   isSticky:NO
                               clickContext:nil];
}
#endif

}