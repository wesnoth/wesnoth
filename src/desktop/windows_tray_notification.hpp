/*
   Copyright (C) 2013 - 2017 by Maxim Biro <nurupo.contributions@gmail.com>
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

#include <SDL.h>
#include <string>
//forces to call Unicode winapi functions instead of ASCII (default)
#ifndef UNICODE
#define UNICODE
#endif
//defines that mingw misses
#ifndef _WIN32_IE
    #define _WIN32_IE 0x0600 //specifying target platform to be Windows XP and higher
#endif
#ifndef NIIF_USER
    #define NIIF_USER 0x00000004
#endif
#ifndef NIN_BALLOONTIMEOUT
    #define NIN_BALLOONTIMEOUT (WM_USER + 4)
#endif
#ifndef NIN_BALLOONUSERCLICK
    #define NIN_BALLOONUSERCLICK (WM_USER + 5)
#endif
// ShellAPI.h should be included after Windows.h only!
#include <windows.h>
#include <shellapi.h>

class windows_tray_notification {
public:
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
	static bool show(std::string title, std::string message);

	/**
	* Frees resources when a notification disappears, switches user to the wesnoth
	* window if the notification popup was clicked by user.
	*
	* @param event System event.
	*/
	static void handle_system_event(const SDL_Event& event);

private:
	static NOTIFYICONDATA* nid;
	static bool message_reset;
	static const int ICON_ID = 1007; // just a random number
	static const unsigned int WM_TRAYNOTIFY = 32868; // WM_APP+100
	static const size_t MAX_TITLE_LENGTH = 63; // 64 including the terminating null character
	static const size_t MAX_MESSAGE_LENGTH = 255; // 256 including the terminating null character

	static bool create_tray_icon();
	static void destroy_tray_icon();
	static bool set_tray_message(const std::string& title, const std::string& message);
	static void adjust_length(std::string& title, std::string& message);
	static HWND get_window_handle();
	static void switch_to_wesnoth_window();
	static std::wstring string_to_wstring(const std::string& string, size_t maxlength);

	explicit windows_tray_notification();
	windows_tray_notification(const windows_tray_notification& w);
	windows_tray_notification& operator=(const windows_tray_notification& w);
};
