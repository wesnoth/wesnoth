/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "hotkey/hotkey_manager.hpp"

#include "hotkey/hotkey_item.hpp"
#include "hotkey/hotkey_command.hpp"


namespace hotkey {

manager::manager()
{
	init();
}

void manager::init()
{
	init_hotkey_commands();
}

void manager::wipe()
{
	clear_hotkey_commands();
	clear_hotkeys();
	delete_all_wml_hotkeys();
}

manager::~manager()
{
	wipe();
}


}
