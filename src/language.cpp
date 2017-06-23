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

#include "filesystem.hpp"
#include "gettext.hpp"
#include "language.hpp"
#include "log.hpp"
#include "preferences/general.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"

#include <stdexcept>
#include <clocale>

#ifdef _WIN32
#include <windows.h>
#if !defined(_MSC_VER) && !defined(__MINGW32__)
extern "C" int _putenv(const char*);
#endif
#endif

#ifdef __APPLE__
#include <cerrno>
#endif

#define DBG_G LOG_STREAM(debug, lg::general())
#define LOG_G LOG_STREAM(info, lg::general())
#define WRN_G LOG_STREAM(warn, lg::general())
#define ERR_G LOG_STREAM(err, lg::general())

namespace {
	language_def current_language;
	std::vector<config> languages_;
	utils::string_map strings_;
}

static language_list known_languages;

bool load_strings(bool complain);

bool current_language_rtl()
{
	return get_language().rtl;
}

bool language_def::operator== (const language_def& a) const
{
	return ((language == a.language) /* && (localename == a.localename) */ );
}

symbol_table string_table;

bool& time_locale_correct()
{
	static bool result = true;
	return result;
}

const t_string& symbol_table::operator[](const std::string& key) const
{
	const utils::string_map::const_iterator i = strings_.find(key);
	if(i != strings_.end()) {
		return i->second;
	} else {
		static t_string empty_string;
		// Let's do it the painful way (untlb means untranslatABLE).
		// It will cause problem if somebody stores more than one reference at once
		// but I don't really care since this path is an error path and it should
		// not have been taken in the first place. -- silene
		empty_string = "UNTLB " + key;
		return empty_string;
	}
}

const t_string& symbol_table::operator[](const char* key) const
{
	return (*this)[std::string(key)];
}

bool load_language_list()
{
	config cfg;
	try {
		filesystem::scoped_istream stream = preprocess_file(filesystem::get_wml_location("hardwired/language.cfg"));
		read(cfg, *stream);
	} catch(config::error &) {
		return false;
	}

	known_languages.clear();
	known_languages.emplace_back("", t_string(N_("System default language"), "wesnoth"), "ltr", "", "A");

	for (const config &lang : cfg.child_range("locale"))
	{
		known_languages.emplace_back(
			lang["locale"], lang["name"], lang["dir"],
			lang["alternates"], lang["sort_name"]);
	}

	return true;
}

language_list get_languages()
{
	// We sort every time, the local might have changed which can modify the
	// sort order.
	std::sort(known_languages.begin(), known_languages.end());
	return known_languages;
}

static void wesnoth_setlocale(int category, const std::string& slocale,
	std::vector<std::string> const *alternates)
{
	std::string locale = slocale;
	// FIXME: ideally we should check LANGUAGE and on first invocation
	// use that value, so someone with es would get the game in Spanish
	// instead of en_US the first time round
	// LANGUAGE overrides other settings, so for now just get rid of it
	// FIXME: add configure check for unsetenv

	//category is never LC_MESSAGES since that case was moved to gettext.cpp to remove the dependency to libintl.h in this file
	//that's why code like if (category == LC_MESSAGES) is outcommented here.
#ifndef _WIN32
	unsetenv ("LANGUAGE"); // void so no return value to check
#endif
#ifdef __APPLE__
	//if (category == LC_MESSAGES && setenv("LANG", locale.c_str(), 1) == -1) {
	//	ERR_G << "setenv LANG failed: " << strerror(errno);
	//}
#endif

#ifdef _WIN32
	std::string win_locale(locale, 0, 2);
	#include "language_win32.ii"
	//if(category == LC_MESSAGES) {
	//	SetEnvironmentVariableA("LANG", win_locale.c_str());
	//	std::string env = "LANGUAGE=" + locale;
	//	_putenv(env.c_str());
	//	return;
	//}
	locale = win_locale;
#endif

	char *res = nullptr;
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
		char const *encoding[] { ".utf-8", ".UTF-8", "" };
		for (int j = 0; j != 3; ++j)
		{
			locale = lang + encoding[j] + extra;
			res = std::setlocale(category, locale.c_str());
			if (res) {
				LOG_G << "Set locale to '" << locale << "' result: '" << res << "'.\n";
				goto done;
			}
		}

		if (!alternates || i == alternates->end()) break;
		locale = *i;
		++i;
	}

	WRN_G << "setlocale() failed for '" << slocale << "'." << std::endl;

	if (category == LC_TIME) {
		time_locale_correct() = false;
	}

#ifndef _WIN32
		//if(category == LC_MESSAGES) {
		//	WRN_G << "Setting LANGUAGE to '" << slocale << "'." << std::endl;
		//	setenv("LANGUAGE", slocale.c_str(), 1);
		//	std::setlocale(LC_MESSAGES, "");
		//}
#endif

	done:
	DBG_G << "Numeric locale: " << std::setlocale(LC_NUMERIC, nullptr) << '\n';
	DBG_G << "Full locale: " << std::setlocale(LC_ALL, nullptr) << '\n';
}

void set_language(const language_def& locale)
{
	strings_.clear();

	std::string locale_lc;
	locale_lc.resize(locale.localename.size());
	std::transform(locale.localename.begin(),locale.localename.end(),locale_lc.begin(),tolower);

	current_language = locale;
	time_locale_correct() = true;

	wesnoth_setlocale(LC_COLLATE, locale.localename, &locale.alternates);
	wesnoth_setlocale(LC_TIME, locale.localename, &locale.alternates);
	translation::set_language(locale.localename, &locale.alternates);
	load_strings(false);
}

bool load_strings(bool complain)
{
	DBG_G << "Loading strings\n";
	config cfg;

	LOG_G << "There are " << languages_.size() << " [language] blocks\n";
	if (complain && languages_.empty()) {
		std::cerr << "No [language] block found\n";
		return false;
	}
	for (const config &lang : languages_) {
		DBG_G << "[language]\n";
		for (const config::attribute &j : lang.attribute_range()) {
			DBG_G << j.first << "=\"" << j.second << "\"\n";
			strings_[j.first] = j.second;
		}
		DBG_G << "[/language]\n";
	}
	DBG_G << "done\n";

	return true;
}

const language_def& get_language() { return current_language; }

const language_def& get_locale()
{
	//TODO: Add in support for querying the locale on Windows

	assert(!known_languages.empty());

	const std::string& prefs_locale = preferences::language();
	if(prefs_locale.empty() == false) {
		translation::set_language(prefs_locale, nullptr);
		for(language_list::const_iterator i = known_languages.begin();
				i != known_languages.end(); ++i) {
			if (prefs_locale == i->localename)
				return *i;
		}
		LOG_G << "'" << prefs_locale << "' locale not found in known array; defaulting to system locale\n";
		return known_languages[0];
	}

#if 0
	const char* const locale = getenv("LANG");
	#ifdef _WIN32
	    std::string win_locale = locale
		#include "language_win32.ii"
		return win_locale;
	#endif
	if(locale != nullptr && strlen(locale) >= 2) {
		//we can't pass pointers into the string to the std::string
		//constructor because some STL implementations don't support
		//it (*cough* MSVC++6)
		std::string res(2,'z');
		res[0] = tolower(locale[0]);
		res[1] = tolower(locale[1]);
		return res;
	}
#endif

	LOG_G << "locale could not be determined; defaulting to system locale\n";
	return known_languages[0];
}

void init_textdomains(const config& cfg)
{
	for (const config &t : cfg.child_range("textdomain"))
	{
		const std::string &name = t["name"];
		const std::string &path = t["path"];

		if(path.empty()) {
			t_string::add_textdomain(name, filesystem::get_intl_dir());
		} else {
			std::string location = filesystem::get_binary_dir_location("", path);

			if (location.empty()) {
				//if location is empty, this causes a crash on Windows, so we
				//disallow adding empty domains
				WRN_G << "no location found for '" << path << "', skipping textdomain" << std::endl;
			} else {
				t_string::add_textdomain(name, location);
			}
		}
	}
}

bool init_strings(const config& cfg)
{
	languages_.clear();
	for (const config &l : cfg.child_range("language")) {
		languages_.push_back(l);
	}
	return load_strings(true);
}

/* vim:set encoding=utf-8: */
