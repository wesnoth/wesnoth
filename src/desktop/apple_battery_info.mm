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

#include "apple_battery_info.hpp"

#if defined(__APPLE__) && defined(__MACH__) && defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__)
#define __IPHONEOS__ (__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__*1000)
#endif

#if defined(__IPHONEOS__)

#import <UIKit/UIDevice.h>

#else

#import <Foundation/Foundation.h>
#import <IOKit/ps/IOPowerSources.h>
#import <IOKit/ps/IOPSKeys.h>
#import <IOKit/pwr_mgt/IOPM.h>

#endif

namespace apple_battery_info {

bool does_device_have_battery() {
#if defined(__IPHONEOS__)
    UIDevice.currentDevice.batteryMonitoringEnabled = YES;
    if (UIDevice.currentDevice.batteryState == UIDeviceBatteryStateUnknown) {
        return false;
    } else {
        return true;
    }
#else
    if (get_battery_percentage() == -1) {
        return false;
    } else {
        return true;
    }
#endif
}

#if !defined(__IPHONEOS__)
inline NSDictionary* get_iops_battery_info() {
    //Code taken from https://github.com/Hammerspoon/hammerspoon/blob/master/extensions/battery/internal.m
    CFTypeRef info = IOPSCopyPowerSourcesInfo();

    if (info == NULL)
        return NULL;


    CFArrayRef list = IOPSCopyPowerSourcesList(info);

    if (list == NULL || !CFArrayGetCount(list)) {
        if (list)
            CFRelease(list);

        CFRelease(info);
        return NULL;
    }

    CFDictionaryRef battery = CFDictionaryCreateCopy(NULL, IOPSGetPowerSourceDescription(info, CFArrayGetValueAtIndex(list, 0)));

    CFRelease(list);
    CFRelease(info);

    return (NSDictionary*) battery;
}
#endif

double get_battery_percentage() {
#if defined(__IPHONEOS__)
    return UIDevice.currentDevice.batteryLevel * 100;
#else
    //Code taken from https://github.com/Hammerspoon/hammerspoon/blob/master/extensions/battery/internal.m
    NSDictionary* battery = get_iops_battery_info();

    NSNumber *maxCapacity = [battery objectForKey:@kIOPSMaxCapacityKey];
    NSNumber *currentCapacity = [battery objectForKey:@kIOPSCurrentCapacityKey];
    
    if (maxCapacity && currentCapacity) {
        return ([currentCapacity doubleValue] / [maxCapacity doubleValue]) * 100;
    } else {
        return -1;
    }
#endif
}

} // end namespace apple_battery_info

#endif //end __APPLE__
