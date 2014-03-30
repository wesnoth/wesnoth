/*
   Copyright (C) 2013 - 2014 by Maxim Biro <nurupo.contributions@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "windows_tray_notification.hpp"

#include <SDL_syswm.h>

#include "gettext.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"

NOTIFYICONDATA* windows_tray_notification::nid = NULL;
bool windows_tray_notification::message_reset = false;

void windows_tray_notification::destroy_tray_icon()
{
	if (nid == NULL) {
		return;
	}

	if (!message_reset){
		Shell_NotifyIcon(NIM_DELETE, nid);
		delete nid;
		nid = NULL;
	} else {
		message_reset = false;
	}
}

void windows_tray_notification::handle_system_event(const SDL_Event& event)
{
	if (event.syswm.msg->msg != WM_TRAYNOTIFY) {
		return;
	}

	if (event.syswm.msg->lParam == NIN_BALLOONUSERCLICK) {
		switch_to_wesnoth_window();
		destroy_tray_icon();
	} else if (event.syswm.msg->lParam == NIN_BALLOONTIMEOUT) {
		destroy_tray_icon();
	}
}

bool windows_tray_notification::create_tray_icon()
{
	// getting handle to a 32x32 icon, contained in "WESNOTH_ICON" icon group of wesnoth.exe resources
	const HMODULE wesnoth_exe = GetModuleHandle(NULL);
	if (wesnoth_exe == NULL) {
		return false;
	}

	const HRSRC group_icon_info = FindResource(wesnoth_exe, L"WESNOTH_ICON", RT_GROUP_ICON);
	if (group_icon_info == NULL) {
		return false;
	}

	HGLOBAL hGlobal = LoadResource(wesnoth_exe, group_icon_info);
	if (hGlobal == NULL) {
		return false;
	}

	const PBYTE group_icon_res = static_cast<PBYTE>(LockResource(hGlobal));
	if (group_icon_res == NULL) {
		return false;
	}

	const int nID = LookupIconIdFromDirectoryEx(group_icon_res, TRUE, 32, 32, LR_DEFAULTCOLOR);
	if (nID == 0) {
		return false;
	}

	const HRSRC icon_info = FindResource(wesnoth_exe, MAKEINTRESOURCE(nID), MAKEINTRESOURCE(3));
	if (icon_info == NULL) {
		return false;
	}

	hGlobal = LoadResource(wesnoth_exe, icon_info);
	if (hGlobal == NULL) {
		return false;
	}

	const PBYTE icon_res = static_cast<PBYTE>(LockResource(hGlobal));
	if (icon_res == NULL) {
		return false;
	}

	const HICON icon = CreateIconFromResource(icon_res, SizeofResource(wesnoth_exe, icon_info), TRUE, 0x00030000);
	if (icon == NULL) {
		return false;
	}

	const HWND window = get_window_handle();
	if (window == NULL) {
		return false;
	}

	const std::wstring& wtip = string_to_wstring(_("The Battle for Wesnoth"), MAX_TITLE_LENGTH);

	// filling notification structure
	nid = new NOTIFYICONDATA;
	memset(nid, 0, sizeof(&nid));
	nid->cbSize = NOTIFYICONDATA_V2_SIZE;
	nid->hWnd = window;
	nid->uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	nid->dwInfoFlags = NIIF_USER;
	nid->uVersion = NOTIFYICON_VERSION;
	nid->uCallbackMessage = WM_TRAYNOTIFY;
	nid->uID = ICON_ID;
	nid->hIcon = icon;
#if _WIN32_WINNT >= 0x600
	nid->hBalloonIcon = icon;
#endif
	lstrcpy(nid->szTip, wtip.c_str());

	// creating icon notification
	return Shell_NotifyIcon(NIM_ADD, nid) != FALSE;
}

bool windows_tray_notification::set_tray_message(const std::string& title, const std::string& message)
{
	// prevents deletion of icon when resetting already existing notification
	message_reset = (nid->uFlags & NIF_INFO) != 0;

	nid->uFlags |= NIF_INFO;
	lstrcpy(nid->szInfoTitle, string_to_wstring(title, MAX_TITLE_LENGTH).c_str());
	lstrcpy(nid->szInfo, string_to_wstring(message, MAX_MESSAGE_LENGTH).c_str());

	// setting notification
	return Shell_NotifyIcon(NIM_MODIFY, nid) != FALSE;
}

void windows_tray_notification::adjust_length(std::string& title, std::string& message)
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

HWND windows_tray_notification::get_window_handle()
{
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	if (SDL_GetWMInfo(&wmInfo) != 1) {
		return NULL;
	}

	return wmInfo.window;
}

void windows_tray_notification::switch_to_wesnoth_window()
{
	const HWND window = get_window_handle();
	if (window == NULL) {
		return;
	}

	if (IsIconic(window)) {
		ShowWindow(window, SW_RESTORE);
	}
	SetForegroundWindow(window);
}

std::wstring windows_tray_notification::string_to_wstring(const std::string& string, size_t maxlength)
{
	utf16::string u16_string = unicode_cast<utf16::string>(string);
	if(u16_string.size() > maxlength) {
		if((u16_string[maxlength-1] & 0xDC00) == 0xD800)
			u16_string.resize(maxlength - 1);
		else
			u16_string.resize(maxlength);
	}
	return std::wstring(u16_string.begin(), u16_string.end());
}

bool windows_tray_notification::show(std::string title, std::string message)
{
	adjust_length(title, message);

	const bool tray_icon_exist = nid != NULL;
	if (!tray_icon_exist) {
		const bool tray_icon_created = create_tray_icon();
		if (!tray_icon_created) {
			const bool memory_allocated = nid != NULL;
			if (memory_allocated) {
				destroy_tray_icon();
			}
			return false;
		}
	}

	// at this point tray icon was just created or already existed before, so it's safe to call `set_tray_message`

	const bool result = set_tray_message(title, message);
	// the `destroy_tray_icon` will be called by event only if `set_tray_message` succeeded
	// if it doesn't succeed, we have to call `destroy_tray_icon` manually
	if (!result) {
		destroy_tray_icon();
	}
	return result;
}
