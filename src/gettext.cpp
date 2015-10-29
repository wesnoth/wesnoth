/*
   Copyright (C) 2003 - 2015 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include <stdlib.h>

#include <libintl.h>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef setlocale
// Someone in libintl world decided it was a good idea to define a "setlocale" macro.
// Note: This is necessary to compile on OS X, it fixes bug #16649
#undef setlocale
#endif

#define DBG_G LOG_STREAM(debug, lg::general)
#define LOG_G LOG_STREAM(info, lg::general)
#define WRN_G LOG_STREAM(warn, lg::general)
#define ERR_G LOG_STREAM(err, lg::general)
namespace translation
{
std::string dgettext(const char* domain, const char* msgid)
{
	return ::dgettext(domain, msgid);
}
std::string egettext(char const *msgid)
{
	return msgid[0] == '\0' ? msgid : (::gettext)(msgid);
}

std::string dsgettext (const char * domainname, const char *msgid)
{
	bind_textdomain_codeset(domainname, "UTF-8");
	const char *msgval = ::dgettext (domainname, msgid);
	if (msgval == msgid) {
		msgval = std::strrchr (msgid, '^');
		if (msgval == NULL)
			msgval = msgid;
		else
			msgval++;
	}
	return msgval;
}

#if 0

const char* sgettext (const char *msgid)
{
	const char *msgval = gettext (msgid);
	if (msgval == msgid) {
		msgval = std::strrchr (msgid, '^');
		if (msgval == NULL)
			msgval = msgid;
		else
			msgval++;
	}
	return msgval;
}

const char* sngettext (const char *singular, const char *plural, int n)
{
	const char *msgval = ngettext (singular, plural, n);
	if (msgval == singular) {
		msgval = std::strrchr (singular, '^');
		if (msgval == NULL)
			msgval = singular;
		else
			msgval++;
	}
	return msgval;
}

#endif 
std::string dsngettext (const char * domainname, const char *singular, const char *plural, int n)
{
	bind_textdomain_codeset(domainname, "UTF-8");
	const char *msgval = ::dngettext (domainname, singular, plural, n);
	if (msgval == singular) {
		msgval = std::strrchr (singular, '^');
		if (msgval == NULL)
			msgval = singular;
		else
			msgval++;
	}
	return msgval;
}

void bind_textdomain(const char* domain, const char* directory, const char* encoding)
{
	if(domain != NULL && strchr(domain, '/') != NULL) {
		// For compatibility with Boost.Locale implementation, which interprets
		// slashes in domain names in a special fashion.
		ERR_G << "illegal textdomain name '" << domain
			  << "', skipping textdomain\n";
		return;
	}

	if(directory != NULL)
		bindtextdomain(domain, directory);
	if(encoding != NULL)
		bind_textdomain_codeset(domain, encoding);
}

void set_default_textdomain(const char* domain)
{
	textdomain(domain);
}

void set_language(const std::string& slocale, const std::vector<std::string>* alternates)
{

	//Code copied from language.cpp::wesnoth_setlocale()
	std::string locale = slocale;
	// FIXME: ideally we should check LANGUAGE and on first invocation
	// use that value, so someone with es would get the game in Spanish
	// instead of en_US the first time round
	// LANGUAGE overrides other settings, so for now just get rid of it
	
#ifdef _WIN32
	(void)alternates;
	std::string win_locale(locale, 0, 2);
	#include "language_win32.ii"
	SetEnvironmentVariableA("LANG", win_locale.c_str());
	std::string env = "LANGUAGE=" + locale;
	_putenv(env.c_str());
	return;
#else
	// FIXME: add configure check for unsetenv
	unsetenv ("LANGUAGE"); // void so no return value to check
#ifdef __APPLE__
	if (setenv("LANG", locale.c_str(), 1) == -1) {
		ERR_G << "setenv LANG failed: " << strerror(errno);
	}
#endif

	char *res = NULL;
	std::vector<std::string>::const_iterator i;
	if (alternates) i = alternates->begin();

	for (;;)
	{
		std::string lang = locale, extra;
		std::string::size_type pos = locale.find('@');
		if (pos != std::string::npos) {
			lang.erase(pos);
			extra = locale.substr(pos);
		}

		/*
		 * The "" is the last item to work-around a problem in glibc picking
		 * the non utf8 locale instead an utf8 version if available.
		 */
		char const *encoding[] = { ".utf-8", ".UTF-8", "" };
		for (int j = 0; j != 3; ++j)
		{
			locale = lang + encoding[j] + extra;
			res = std::setlocale(LC_MESSAGES, locale.c_str());
			if (res) {
				LOG_G << "Set locale to '" << locale << "' result: '" << res << "'.\n";
				return;
			}
		}

		if (!alternates || i == alternates->end()) break;
		locale = *i;
		++i;
	}
	WRN_G << "setlocale() failed for '" << slocale << "'." << std::endl;
#endif //win32
}

void init()
{
#ifndef _WIN32
	std::setlocale(LC_MESSAGES, "");
#endif
}

}
