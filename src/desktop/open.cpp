/*
	Copyright (C) 2013 - 2025
	by Iris Morelle <shadowm2006@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "desktop/open.hpp"

#include "log.hpp"
#include "serialization/unicode.hpp"

#if defined(_X11) || defined(__APPLE__)

#include <thread>

#include <sys/wait.h>
#include <unistd.h> // fork(), exec family

#elif defined(_WIN32)

#ifndef UNICODE
#define UNICODE
#endif
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <shellapi.h> // ShellExecute()

#elif defined(__ANDROID__)
#include <SDL2/SDL_system.h> // For SDL Android functions
#include <jni.h>

#endif

static lg::log_domain log_desktop("desktop");
#define ERR_DU LOG_STREAM(err, log_desktop)
#define LOG_DU LOG_STREAM(info, log_desktop)

namespace desktop {

bool open_object([[maybe_unused]] const std::string& path_or_url)
{
	LOG_DU << "open_object(): requested object: " << path_or_url;

#if defined(_X11) || defined(__APPLE__)

#ifndef __APPLE__
	LOG_DU << "open_object(): on X11, will use xdg-open";
	const char launcher[] = "xdg-open";
#else
	LOG_DU << "open_object(): on OS X, will use open";
	const char launcher[] = "open";
#endif

	const pid_t child = fork();

	if(child == -1) {
		ERR_DU << "open_object(): fork() failed";
		return false;
	} else if(child == 0) {
		execlp(launcher, launcher, path_or_url.c_str(), nullptr);
		_exit(1); // This shouldn't happen.
	}

	std::thread t { [child](){ int status; waitpid(child, &status, 0); } };
	t.detach();

	return true;

#elif defined(_WIN32)

	LOG_DU << "open_object(): on Win32, will use ShellExecute()";

	std::wstring u16path = unicode_cast<std::wstring>(path_or_url);

	const ptrdiff_t res = reinterpret_cast<ptrdiff_t>(ShellExecute(nullptr, L"open", u16path.c_str(), nullptr, nullptr, SW_SHOW));
	if(res <= 32) {
		ERR_DU << "open_object(): ShellExecute() failed (" << res << ")";
		return false;
	}

	return true;

#elif defined(__ANDROID__)
	LOG_DU << "open_object(): on Android, will use Intent.GET_ACTION_VIEW with path as extra data. URL: " << path_or_url;
	// call the helper method WesnothActivity.open(String url) using JNI
	JNIEnv* env = reinterpret_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
	jobject wesnoth_instance = reinterpret_cast<jobject>(SDL_AndroidGetActivity());
	jclass wesnoth_activity(env->GetObjectClass(wesnoth_instance));
	jmethodID open = env->GetMethodID(wesnoth_activity, "open", "(Ljava/lang/String;)V");
	env->CallVoidMethod(wesnoth_instance, open, env->NewStringUTF(path_or_url.c_str()));

	if(env->ExceptionCheck() == JNI_TRUE) {
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	env->DeleteLocalRef(wesnoth_instance);
	env->DeleteLocalRef(wesnoth_activity);

	return true;
#else

	ERR_DU << "open_object(): unsupported platform";
	return false;

#endif
}

}
