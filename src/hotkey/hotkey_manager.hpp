/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

/* The hotkey system allows hotkey definitions to be loaded from
 * configuration objects, and then detect if a keyboard event
 * refers to a hotkey command being executed.
 */
namespace hotkey {

/// this class is initialized once at game start
/// put all initialization and wipe code in the methods here.
class manager {
public:
	manager();
	static void init();
	static void wipe();
	~manager();
};

}
