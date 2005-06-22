/* $Id$ */
/* vim:set encoding=utf-8: */
/*
   Copyright (C) 2003 by David White <davidnwhite@comcast.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "config.hpp"
#include "filesystem.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "language.hpp"
#include "preferences.hpp"
#include "util.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "wesconfig.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace {
	language_def current_language;
	string_map strings_;
}

language_def known_languages[] = {
	language_def("", N_("System default language")),
	language_def("bg_BG", "Български (Bulgarski)"),
	language_def("ca_ES", "Català"),
	language_def("cs_CZ", "Čeština"),
	language_def("da_DK", "Dansk"),
	language_def("de_DE", "Deutsch"),
	language_def("el_GR", "Ελληνικά (Ellinika)"),
	language_def("et_EE", "Eesti"),
	language_def("en_GB", "English [GB]"),
	language_def("C",     "English [US]"),
	language_def("es_ES", "Español"),
	language_def("eu_ES", "Euskera"),
	language_def("fr_FR", "Français"),
	language_def("it_IT", "Italiano"),
	language_def("la_IT", "Latina"),
	language_def("hu_HU", "Magyar"),
	language_def("nl_NL", "Nederlands"),
	language_def("ja_JP", "日本語 (Nihongo)"),
	language_def("no_NO", "Norsk"),
	language_def("pl_PL", "Polski"),
	language_def("pt_BR", "Português do Brasil"),
	language_def("ru_RU", "Русский (Russkij)"),
	language_def("sk_SK", "Slovenčina"),
	language_def("sl_SI", "Slovenščina"),
	language_def("sr_CS", "Srpski"),
	language_def("tr_TR", "Türkçe"),
	language_def("fi_FI", "Suomi"),
	language_def("sv_SE", "Svenska"),
	language_def("zh_CN", "中文 (Zhongwen)"),

	// end of list marker, do not remove
	language_def("", "")
};

std::string languagedef_name (const language_def& def)
{
	return def.language;
}

bool languagedef_lessthan_p (const language_def& def1, const language_def& def2)
{
	return (def1.language < def2.language);
}

bool language_def::operator== (const language_def& a)
{
	return ((language == a.language) /* && (localename == a.localename) */ );
}

symbol_table string_table;

const t_string& symbol_table::operator[](const std::string& key) const
{
	const string_map::const_iterator i = strings_.find(key);
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

std::vector<language_def> get_languages()
{
	std::vector<language_def> res;

	for(int i = 0; known_languages[i].language[0] != '\0'; i++) {
		res.push_back(known_languages[i]);
	}

	return res;
}

static void wesnoth_setlocale(int category, std::string const &slocale)
{
	char const *locale = slocale.c_str();
#ifdef __BEOS__
	if(setenv ("LANG", locale, 1) == -1)
		std::cerr << "setenv LANG failed: " << strerror(errno);
	if(setenv ("LC_ALL", locale, 1) == -1)
		std::cerr << "setenv LC_ALL failed: " << strerror(errno);
#endif
#ifdef __APPLE__
	if(setenv ("LANGUAGE", locale, 1) == -1)
		std::cerr << "setenv LANGUAGE failed: " << strerror(errno);
	if(setenv ("LC_ALL", locale, 1) == -1)
		std::cerr << "setenv LC_ALL failed: " << strerror(errno);
#endif

#ifdef _WIN32
	const std::string env = "LANG=" + slocale;
	putenv(env.c_str());
#endif

#ifdef USE_DUMMYLOCALES
	static enum { UNINIT, NONE, PRESENT } status = UNINIT;
	static std::string locpath;
	if (status == UNINIT)
		if (char const *p = getenv("LOCPATH")) {
			locpath = p;
			status = PRESENT;
		} else status = NONE;
	if (slocale.empty())
		if (status == NONE)
			unsetenv("LOCPATH");
		else
			setenv("LOCPATH", locpath.c_str(), 1);
	else setenv("LOCPATH", (game_config::path + "/locales").c_str(), 1);
	std::string xlocale;
	if (!slocale.empty()) {
		// dummy suffix to prevent locale aliasing from kicking in
		xlocale = slocale + "@wesnoth";
		locale = xlocale.c_str();
	}
#endif
	char* res = setlocale (category, locale);
	if (res == NULL)
		std::cerr << "WARNING: setlocale() failed for "
			  << locale << ".\n";
}

bool set_language(const language_def& locale)
{
	strings_.clear();

	std::string locale_lc;
	locale_lc.resize(locale.localename.size());
	std::transform(locale.localename.begin(),locale.localename.end(),locale_lc.begin(),tolower);

	config cfg;

	current_language = locale;
	wesnoth_setlocale(LC_MESSAGES, locale.localename);
	known_languages[0].language = gettext("System default language");

	// fill string_table (should be moved somwhere else some day)
	try {
		scoped_istream stream = preprocess_file("data/translations/english.cfg");
		read(cfg, *stream);
	} catch(config::error& e) {
		std::cerr << "Could not read english.cfg\n";
		throw e;
	}

	config* langp = cfg.child("language");
	if (langp == NULL) {
		std::cerr << "No [language] block found in english.cfg\n";
		return false;
	}

	for(string_map::const_iterator j = langp->values.begin(); j != langp->values.end(); ++j) {
		strings_[j->first] = j->second;
	}
	// end of string_table fill

	return true;
}

const language_def& get_language() { return current_language; }

const language_def& get_locale()
{
	//TODO: Add in support for querying the locale on Windows

	const std::string& prefs_locale = preferences::language();
	if(prefs_locale.empty() == false) {
		wesnoth_setlocale(LC_MESSAGES, prefs_locale);
		for(int i = 0; known_languages[i].language[0] != '\0'; i++) {
			if (prefs_locale == known_languages[i].localename)
				return known_languages[i];
		}
		std::cerr << "locale not found in known array; defaulting to system locale\n";
		return known_languages[0];
	}

#if 0
	const char* const locale = getenv("LANG");
	if(locale != NULL && strlen(locale) >= 2) {
		//we can't pass pointers into the string to the std::string
		//constructor because some STL implementations don't support
		//it (*cough* MSVC++6)
		std::string res(2,'z');
		res[0] = tolower(locale[0]);
		res[1] = tolower(locale[1]);
		return res;
	}
#endif

	std::cerr << "locale could not be determined; defaulting to system locale\n";
	return known_languages[0];
}

void init_textdomains(const config& cfg)
{
	config::const_child_itors t = cfg.child_range("textdomain");

	for(;t.first != t.second; ++t.first) {
		const std::string name = (**t.first)["name"];
		const std::string path = (**t.first)["path"];

		if(path.empty()) {
			t_string::add_textdomain(name, get_intl_dir());
		} else {
			const std::string& location = get_binary_file_location(path, ".");

			//if location is empty, this causes a crash on Windows, so we
			//disallow adding empty domains
			if(location.empty()) {
				std::cerr << "no location found for '" << path << "', not adding textdomain\n";
			} else {
				t_string::add_textdomain(name, location);
			}
		}
	}
}

