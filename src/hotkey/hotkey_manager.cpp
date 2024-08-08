/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "hotkey/hotkey_manager.hpp"

#include "hotkey/hotkey_command.hpp"
#include "hotkey/hotkey_item.hpp"

namespace hotkey
{

manager::manager()
{
	init_hotkey_commands();
}

manager::~manager()
{
	clear_hotkeys();
}

} // namespace hotkey
