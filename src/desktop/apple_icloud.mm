/*
	Copyright (C) 2026
	by Neil Hiddink
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

#include "apple_icloud.hpp"

#include <TargetConditionals.h>

#if TARGET_OS_IOS

#import <Foundation/Foundation.h>
#import <dispatch/dispatch.h>

namespace desktop {
namespace apple {

utils::optional<std::string> get_icloud_drive_documents_dir()
{
	__block NSString* documents_path = nil;

	@autoreleasepool {
		NSString* bundle_identifier = [[NSBundle mainBundle] bundleIdentifier];
		if(bundle_identifier.length == 0) {
			return utils::nullopt;
		}

		NSString* container_identifier = [@"iCloud." stringByAppendingString:bundle_identifier];

		dispatch_sync(dispatch_get_global_queue(QOS_CLASS_UTILITY, 0), ^{
			@autoreleasepool {
				NSURL* container_url =
					[[NSFileManager defaultManager] URLForUbiquityContainerIdentifier:container_identifier];
				if(container_url != nil) {
					NSURL* documents_url = [container_url URLByAppendingPathComponent:@"Documents" isDirectory:YES];
					documents_path = [documents_url path];
				}
			}
		});

		if(documents_path.length == 0) {
			return utils::nullopt;
		}

		return std::string([documents_path UTF8String]);
	}
}

} // end namespace apple
} // end namespace desktop

#else

namespace desktop {
namespace apple {

utils::optional<std::string> get_icloud_drive_documents_dir()
{
	return utils::nullopt;
}

} // end namespace apple
} // end namespace desktop

#endif
#endif
