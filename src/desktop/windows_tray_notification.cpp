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

#include "desktop/windows_tray_notification.hpp"

#include "gettext.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"
#include "sdl/window.hpp"
#include "video.hpp" // for get_window

//forces to call Unicode winapi functions instead of ASCII (default)
#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN

// ShellAPI.h should be included after Windows.h only!
#include <windows.h>
#include <shellapi.h>

namespace windows_tray_notification
{
namespace implementation
{
constexpr int ICON_ID = 1007; // just a random number
constexpr unsigned int WM_TRAYNOTIFY = 32868; // WM_APP+100
constexpr std::size_t MAX_TITLE_LENGTH = 63; // 64 including the terminating null character
constexpr std::size_t MAX_MESSAGE_LENGTH = 255; // 256 including the terminating null character

NOTIFYICONDATA* nid = nullptr;
bool message_reset = false;

namespace helper
{
HWND get_window_handle()
{
	auto props = SDL_GetWindowProperties(video::get_window());
	return static_cast<HWND>(SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
}

std::wstring string_to_wstring(const std::string& string, std::size_t maxlength)
{
	std::u16string u16_string = unicode_cast<std::u16string>(string);
	if (u16_string.size() > maxlength) {
		if ((u16_string[maxlength - 1] & 0xDC00) == 0xD800)
			u16_string.resize(maxlength - 1);
		else
			u16_string.resize(maxlength);
	}
	return std::wstring(u16_string.begin(), u16_string.end());
}

} // helper

void destroy_tray_icon()
{
	if (nid == nullptr) {
		return;
	}

	if (!message_reset){
		Shell_NotifyIcon(NIM_DELETE, nid);
		delete nid;
		nid = nullptr;
	} else {
		message_reset = false;
	}
}

bool create_tray_icon()
{
	// getting handle to a 32x32 icon, contained in "WESNOTH_ICON" icon group of wesnoth.exe resources
	const HMODULE wesnoth_exe = GetModuleHandle(nullptr);
	if (wesnoth_exe == nullptr) {
		return false;
	}

	const HRSRC group_icon_info = FindResource(wesnoth_exe, TEXT("WESNOTH_ICON"), RT_GROUP_ICON);
	if (group_icon_info == nullptr) {
		return false;
	}

	HGLOBAL hGlobal = LoadResource(wesnoth_exe, group_icon_info);
	if (hGlobal == nullptr) {
		return false;
	}

	const PBYTE group_icon_res = static_cast<PBYTE>(LockResource(hGlobal));
	if (group_icon_res == nullptr) {
		return false;
	}

	const int nID = LookupIconIdFromDirectoryEx(group_icon_res, TRUE, 32, 32, LR_DEFAULTCOLOR);
	if (nID == 0) {
		return false;
	}

	const HRSRC icon_info = FindResource(wesnoth_exe, MAKEINTRESOURCE(nID), MAKEINTRESOURCE(3));
	if (icon_info == nullptr) {
		return false;
	}

	hGlobal = LoadResource(wesnoth_exe, icon_info);
	if (hGlobal == nullptr) {
		return false;
	}

	const PBYTE icon_res = static_cast<PBYTE>(LockResource(hGlobal));
	if (icon_res == nullptr) {
		return false;
	}

	const HICON icon = CreateIconFromResource(icon_res, SizeofResource(wesnoth_exe, icon_info), TRUE, 0x00030000);
	if (icon == nullptr) {
		return false;
	}

	const HWND window = helper::get_window_handle();
	if (window == nullptr) {
		return false;
	}

	const std::wstring& wtip = helper::string_to_wstring(_("The Battle for Wesnoth"), MAX_TITLE_LENGTH);

	// filling notification structure
	nid = new NOTIFYICONDATA;
	memset(nid, 0, sizeof(*nid));
	nid->cbSize = NOTIFYICONDATA_V2_SIZE;
	nid->hWnd = window;
	nid->uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	nid->dwInfoFlags = NIIF_USER;
	nid->uVersion = NOTIFYICON_VERSION;
	nid->uCallbackMessage = WM_TRAYNOTIFY;
	nid->uID = ICON_ID;
	nid->hIcon = icon;
	nid->hBalloonIcon = icon;
	lstrcpyW(nid->szTip, wtip.c_str());

	// creating icon notification
	return Shell_NotifyIcon(NIM_ADD, nid) != FALSE;
}

bool set_tray_message(const std::string& title, const std::string& message)
{
	// prevents deletion of icon when resetting already existing notification
	message_reset = (nid->uFlags & NIF_INFO) != 0;

	nid->uFlags |= NIF_INFO;
	lstrcpyW(nid->szInfoTitle, helper::string_to_wstring(title, MAX_TITLE_LENGTH).c_str());
	lstrcpyW(nid->szInfo, helper::string_to_wstring(message, MAX_MESSAGE_LENGTH).c_str());

	// setting notification
	return Shell_NotifyIcon(NIM_MODIFY, nid) != FALSE;
}

void adjust_length(std::string& title, std::string& message)
{
	static const int ELIPSIS_LENGTH = 3;

	// limitations set by winapi
	if (title.length() > MAX_TITLE_LENGTH) {
		utils::ellipsis_truncate(title, MAX_TITLE_LENGTH - ELIPSIS_LENGTH);
	}
	if (message.length() > MAX_MESSAGE_LENGTH) {
		utils::ellipsis_truncate(message, MAX_MESSAGE_LENGTH - ELIPSIS_LENGTH);
	}
}

void switch_to_wesnoth_window()
{
	const HWND window = helper::get_window_handle();
	if (window == nullptr) {
		return;
	}

	if (IsIconic(window)) {
		ShowWindow(window, SW_RESTORE);
	}
	SetForegroundWindow(window);
}

} // implementation

bool show(std::string title, std::string message)
{
	implementation::adjust_length(title, message);

	const bool tray_icon_exist = implementation::nid != nullptr;
	if (!tray_icon_exist) {
		const bool tray_icon_created = implementation::create_tray_icon();
		if (!tray_icon_created) {
			const bool memory_allocated = implementation::nid != nullptr;
			if (memory_allocated) {
				implementation::destroy_tray_icon();
			}
			return false;
		}
	}

	// at this point tray icon was just created or already existed before, so it's safe to call `set_tray_message`

	const bool result = implementation::set_tray_message(title, message);
	// the `destroy_tray_icon` will be called by event only if `set_tray_message` succeeded
	// if it doesn't succeed, we have to call `destroy_tray_icon` manually
	if (!result) {
		implementation::destroy_tray_icon();
	}
	return result;
}

bool message_hook(const MSG& msg)
{
	switch(msg.lParam) {
	case NIN_BALLOONUSERCLICK:
		implementation::switch_to_wesnoth_window();
		implementation::destroy_tray_icon();
		return true;

	case NIN_BALLOONTIMEOUT:
		implementation::destroy_tray_icon();
		return true;

	// Scenario: More than one notification arrives before the time-out triggers the tray icon destruction.
	// Problem: Events seem to be triggered differently in SDL 2.0. For the example of two notifications arriving at once:
	//	1. Balloon created for first notification
	//	2. Balloon created for second notification (message_reset set to true because of first notification already present)
	//	3. Balloon time-out for first notification (destroy_tray_icon skips tray icon destruction because of message_reset flag)
	//	4.	SDL 1.2: Balloon time-out for second notification (destroy_tray_icon destroys tray icon)
	//		SDL 2.0: Balloon time-out for second notification event is never received (tray icon remains indefinitely)
	// This results in the tray icon being 'stuck' until the user quits Wesnoth *and* hovers over the tray icon (and is only then killed off by the OS).
	// As a less-than-ideal-but-better-than-nothing-solution, call destroy_tray_icon when the user hovers mouse cursor over the tray icon. At least then the tray is 'reset'.
	// I could not find the matching definition for 0x0200 in the headers, but this message value is received when the mouse cursor is over the tray icon.
	// Drawback: The tray icon can still get 'stuck' if the user does not move the mouse cursor over the tray icon.
	//	Also, accidental destruction of the tray icon can occur if the user moves the mouse cursor over the tray icon before the balloon for a single notification has expired.
	case 0x0200:
		if(implementation::message_reset) {
			return false;
		}

		implementation::destroy_tray_icon();
		return true;

	default:
		return false;
	}
}

} // windows_tray_notification
