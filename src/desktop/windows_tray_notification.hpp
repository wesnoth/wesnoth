/*
	Copyright (C) 2013 - 2025
	by Maxim Biro <nurupo.contributions@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include <string>

// Win32 API forward declaration (full definition in WinUser.h)
typedef struct tagMSG MSG;

namespace windows_tray_notification
{
	/**
	* Displays a tray notification.
	* When user clicks on the notification popup, the user switches to the wesnoth window.
	*
	* @param title Title of a notification. Gets truncated if longer than 64 characters, including
	*           the terminating null character.
	* @param message Message of a notification. Gets truncated if longer than 256 characters, including
	*           the terminating null character.
	*
	* @return True if message was shown successfully, False otherwise.
	*/
	bool show(std::string title, std::string message);

	/**
	* Frees resources when a notification disappears, switches user to the wesnoth
	* window if the notification popup was clicked by user.
	*
	* @param msg Windows message payload.
	*
	* @return True if the message was handled.
	*/
	bool message_hook(const MSG& msg);
};
