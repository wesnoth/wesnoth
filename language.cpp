/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "config.hpp"
#include "hotkeys.hpp"
#include "language.hpp"
#include "preferences.hpp"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>

std::map<std::string,std::string> string_table;

std::vector<std::string> get_languages(config& cfg)
{
	std::vector<std::string> res;

	const std::vector<config*>& lang = cfg.children["language"];
	for(std::vector<config*>::const_iterator i = lang.begin();
	    i != lang.end(); ++i) {
		res.push_back((*i)->values["language"]);
	}

	return res;
}

namespace {
bool internal_set_language(const std::string& locale, config& cfg)
{
	const std::vector<config*>& lang = cfg.children["language"];
	for(std::vector<config*>::const_iterator i = lang.begin();
	    i != lang.end(); ++i) {
		if((*i)->values["id"] == locale || (*i)->values["language"] == locale) {

			for(std::map<std::string,std::string>::const_iterator j =
			    (*i)->values.begin(); j != (*i)->values.end(); ++j) {
				string_table[j->first] = j->second;
			}

			add_hotkeys(**i);
			return true;
		}
	}

	return false;
}
}

bool set_language(const std::string& locale, config& cfg)
{
	string_table.clear();

	//default to English locale first, then set desired locale
	internal_set_language("en",cfg);
	return internal_set_language(locale,cfg);
}

std::string get_locale()
{
	//TODO: Add in support for querying the locale on Windows

	const std::string& prefs_locale = preferences::locale();
	if(prefs_locale.empty() == false) {
		return prefs_locale;
	}

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

	std::cerr << "locale could not be determined; defaulting to locale 'en'\n";
	return "en";
}
