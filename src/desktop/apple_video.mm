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

	void install_keyboard_dismiss_toolbar()
	{
		// iOS-only, no-op on macOS.
	}
} // end namespace apple
} // end namespace desktop

#elif TARGET_OS_IOS

#include "apple_video.hpp"

#include <SDL2/SDL_keyboard.h>

#import <UIKit/UIKit.h>

static __weak UIResponder* first_responder = nil;

@interface UIResponder (WesnothFirstResponderCapture)
- (void)wesnoth_capture_first_responder:(id)sender;
@end

@implementation UIResponder (WesnothFirstResponderCapture)
- (void)wesnoth_capture_first_responder:(id)sender
{
	(void)sender;
	first_responder = self;
}
@end

@interface wesnoth_keyboard_toolbar : NSObject
+ (instancetype)shared;
- (void)attach_toolbar_to_first_responder;
@end

@implementation wesnoth_keyboard_toolbar

+ (instancetype)shared
{
	static wesnoth_keyboard_toolbar* instance = nil;
	static dispatch_once_t once_token;
	dispatch_once(&once_token, ^{
		instance = [[wesnoth_keyboard_toolbar alloc] init];
	});
	return instance;
}

- (instancetype)init
{
	self = [super init];
	if(self) {
		[[NSNotificationCenter defaultCenter]
			addObserver:self
			   selector:@selector(on_keyboard_will_show:)
				   name:UIKeyboardWillShowNotification
				 object:nil];
	}
	return self;
}

- (UIResponder*)current_first_responder
{
	first_responder = nil;
	[[UIApplication sharedApplication]
		sendAction:@selector(wesnoth_capture_first_responder:)
			   to:nil
			 from:nil
		 forEvent:nil];
	return first_responder;
}

- (UIToolbar*)dismiss_toolbar
{
	static UIToolbar* toolbar = nil;
	if(!toolbar) {
		toolbar = [[UIToolbar alloc] initWithFrame:CGRectMake(0.0, 0.0, 0.0, 44.0)];
		UIBarButtonItem* flex = [[UIBarButtonItem alloc]
			initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
								 target:nil
								 action:nil];
		UIBarButtonItem* dismiss = [[UIBarButtonItem alloc]
			initWithTitle:NSLocalizedString(@"Dismiss", @"Dismiss keyboard button")
					style:UIBarButtonItemStyleDone
				   target:self
				   action:@selector(on_dismiss_keyboard:)];
		[toolbar setItems:@[flex, dismiss]];
		[toolbar sizeToFit];
	}
	return toolbar;
}

- (void)on_keyboard_will_show:(NSNotification*)notification
{
	(void)notification;
	[self attach_toolbar_to_first_responder];
}

- (void)attach_toolbar_to_first_responder
{
	UIResponder* responder = [self current_first_responder];
	if([responder isKindOfClass:[UITextField class]]) {
		UITextField* text_field = (UITextField*)responder;
		UIToolbar* toolbar = [self dismiss_toolbar];
		if(text_field.inputAccessoryView != toolbar) {
			text_field.inputAccessoryView = toolbar;
			[text_field reloadInputViews];
		}
	} else if([responder isKindOfClass:[UITextView class]]) {
		UITextView* text_view = (UITextView*)responder;
		UIToolbar* toolbar = [self dismiss_toolbar];
		if(text_view.inputAccessoryView != toolbar) {
			text_view.inputAccessoryView = toolbar;
			[text_view reloadInputViews];
		}
	}
}

- (void)on_dismiss_keyboard:(id)sender
{
	(void)sender;
	SDL_StopTextInput();
}

@end

namespace desktop {
namespace apple {
	CGFloat get_scale_factor(int display_index) {
		NSArray<UIScreen*>* screens = [UIScreen screens];
		if(display_index >= 0 && static_cast<NSUInteger>(display_index) < screens.count) {
			return screens[display_index].scale;
		}
		return UIScreen.mainScreen.scale;
	}

	void install_keyboard_dismiss_toolbar()
	{
		if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone) {
			[wesnoth_keyboard_toolbar shared];
		}
	}
} // end namespace apple
} // end namespace desktop

#endif //end TARGET_OS_OSX / TARGET_OS_IOS
#endif //end __APPLE__
