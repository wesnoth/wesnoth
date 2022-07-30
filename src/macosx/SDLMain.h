/*
	Copyright (C) 2010 - 2021
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/*
	SDLMain.m - main entry point for our Cocoa-ized SDL app
	Initial Version: Darrell Walisser <dwaliss1@purdue.edu>
	Non-NIB-Code & other changes: Max Horn <max@quendi.de>
	Edited a lot for Wesnoth by Ben Anderman <ben@happyspork.com>
*/

#import <Cocoa/Cocoa.h>

@interface SDLMain : NSObject
{
}
- (IBAction)openHomepage:(id)sender;
- (IBAction)openChangelog:(id)sender;
@end
