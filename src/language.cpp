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

#include "global.hpp"

#include "config.hpp"
#include "font.hpp"
#include "language.hpp"
#include "preferences.hpp"
#include "util.hpp"

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
	language_def("en_GB", "British English"),
	language_def("bg_BG", "Български"),
	language_def("es_ES", "Castellano"),
	language_def("ca_ES", "Català"),
	language_def("cs_CZ", "Čeština"),
	language_def("da_DK", "Dansk"),
	language_def("de_DE", "Deutsch"),
 	language_def("C", "English"),
	language_def("eu_ES", "Euskera"),
 	language_def("fr_FR", "Français"),
 	language_def("el_GR", "Greek"),
	language_def("hu_HU", "Hungarian"),
	language_def("it_IT", "Italiano"),
	language_def("la_IT", "Latina"),
	language_def("nl_NL", "Nederlands"),
	language_def("no_NO", "Norsk"),
	language_def("pl_PL", "Polski"),
	language_def("pt_BR", "Português do Brasil"),
	language_def("ru_RU", "Русский (Russian)"),
	language_def("sk_SK", "Slovenčina"),
	language_def("sl_SL", "Slovenščina"),
	language_def("fi_FI", "Suomi"),
	language_def("sv_SE", "Svenska"),

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

const std::string& symbol_table::operator[](const std::string& key) const
{
	const string_map::const_iterator i = strings_.find(key);
	if(i != strings_.end()) {
		return i->second;
	} else {
		static std::string empty_string;
		// Let's do it the painful way (untlb means untranslatABLE).
		// It will cause problem if somebody stores more than one reference at once
		// but I don't really care since this path is an error path and it should
		// not have been taken in the first place. -- silene
		empty_string = "UNTLB " + key;
		return empty_string;
	}
}

const std::string& symbol_table::operator[](const char* key) const
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

char* wesnoth_setlocale(int category, const char *locale)
{
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
	const std::string env = std::string("LANG=") + locale;
	putenv(env.c_str());
#endif

	char* res = setlocale (category, locale);
	if (res == NULL)
		std::cerr << "WARNING: setlocale() failed for "
			  << locale << ".\n";
	return res;
}

bool set_language(const language_def& locale)
{
	strings_.clear();

	std::string locale_lc;
	locale_lc.resize(locale.localename.size());
	std::transform(locale.localename.begin(),locale.localename.end(),locale_lc.begin(),tolower);

	config cfg;

	current_language = locale;
	wesnoth_setlocale (LC_MESSAGES, locale.localename.c_str());
	font::set_font();
	known_languages[0].language = gettext("System default language");

	// fill string_table (should be moved somwhere else some day)
	try {
		cfg.read(preprocess_file("data/translations/english.cfg"));
	} catch(config::error& e) {
		std::cerr << "Could not read english.cfg\n";
		throw e;
	}

	config* langp = cfg.child("language");
	if (langp == NULL) {
	 	std::cerr << "No [language] block found in english.cfg";	
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
		char* setlocaleres = wesnoth_setlocale (LC_MESSAGES, prefs_locale.c_str());
		if(setlocaleres == NULL)
			std::cerr << "call to setlocale() failed for " << prefs_locale.c_str() << "\n";
		for(int i = 0; known_languages[i].language[0] != '\0'; i++) {
		  	if (prefs_locale == known_languages[i].localename)
			  	return known_languages[i];
		}
		
		std::cerr << "setlocale succeeded but locale not found in known array; defaulting to system locale\n";
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

class invalid_utf8_exception : public std::exception {
};

namespace 
{
std::string wstring_to_utf8(const wide_string &src)
{
	wchar_t ch;
	wide_string::const_iterator i;
	int j;
	Uint32 bitmask;
	std::string ret;

	try {

		for(i = src.begin(); i != src.end(); ++i) {
			int count;
			ch = *i;
			
			/* Determine the bytes required */
			count = 1;
			if(ch >= 0x80)
				count++;

			bitmask = 0x800;
			for(j = 0; j < 5; ++j) {
				if(ch >= bitmask)
					count++;
				bitmask <<= 5;
			}
			
			if(count > 6)
				throw invalid_utf8_exception();

			if(count == 1) {
				push_back(ret,ch);
			} else {
				for(j = count-1; j >= 0; --j) {
					unsigned char c = (ch >> (6*j)) & 0x3f;
					c |= 0x80;
					if(j == count-1)
						c |= 0xff << (8 - count);
					push_back(ret,c);
				}
			}

		}

		return ret;
	}
	catch(invalid_utf8_exception e) {
		std::cerr << "Invalid wide character string\n";
		return ret;
	}
}

int byte_size_from_utf8_first(unsigned char ch)
{
	int count;

	if ((ch & 0x80) == 0)
		count = 1;
	else if ((ch & 0xE0) == 0xC0)
		count = 2;
	else if ((ch & 0xF0) == 0xE0)
		count = 3;
	else if ((ch & 0xF8) == 0xF0)
		count = 4;
	else if ((ch & 0xFC) == 0xF8)
		count = 5;
	else if ((ch & 0xFE) == 0xFC)
		count = 6;
	else
		throw invalid_utf8_exception(); /* stop on invalid characters */

	return count;
}
	
wide_string utf8_to_wstring(const std::string &src)
{
	wide_string ret;	
	
	try {
		for(size_t i = 0, l = src.size(); i != l;) {
			wchar_t ch = (unsigned char)src[i];
			const int count = byte_size_from_utf8_first(ch);

			if (i + count > l)
				throw invalid_utf8_exception();

			/* Convert the first character */
			if (count != 1) {
				ch &= 0xFF >> (count + 1);
			}

			/* Convert the continuation bytes */
			for(size_t j = i + 1; j != i + count; ++j) {
				unsigned char ch2 = src[j];
				if ((ch2 & 0xC0) != 0x80)
					throw invalid_utf8_exception();

				ch = (ch << 6) | (ch2 & 0x3F);
			}
			i += count;

			push_back(ret,ch);
		}
	}
	catch(invalid_utf8_exception e) {
		std::cerr << "Invalid UTF-8 string: \"" << src << "\"\n";
		return ret;
	}

	return ret;
}

}

std::vector<std::string> split_utf8_string(const std::string &src)
{
	std::vector<std::string> ret;
	
	try {
		for(size_t i = 0; i < src.size(); /* nop */) {
			const int size = byte_size_from_utf8_first(src[i]);
			if(i + size > src.size())
				throw invalid_utf8_exception();

			ret.push_back(src.substr(i, size));
			i += size;
		}
	}
	
	catch(invalid_utf8_exception e) {
		std::cerr << "Invalid UTF-8 string: \"" << src << "\"\n";
		return ret;
	}

	return ret;
}

std::string wstring_to_string(const wide_string &src)
{
	return wstring_to_utf8(src);
}

std::string wchar_to_string(const wchar_t c) {
	wide_string s;
	s.push_back(c);
	return wstring_to_utf8(s);
}

wide_string string_to_wstring(const std::string &src)
{
	return utf8_to_wstring(src);
}
