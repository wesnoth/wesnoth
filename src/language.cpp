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
#include "font.hpp"
#include "hotkeys.hpp"
#include "language.hpp"
#include "preferences.hpp"
#include "util.hpp"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace {
	CHARSET charset_used = CHARSET_LATIN1;
	std::string current_language;
	string_map strings_;
}

CHARSET charset() { return charset_used; }

symbol_table string_table;

const std::string& symbol_table::operator[](const std::string& key) const
{
	const string_map::const_iterator i = strings_.find(key);
	if(i != strings_.end()) {
		return i->second;
	} else {
		static std::string empty_string;
		return empty_string;
	}
}

const std::string& symbol_table::operator[](const char* key) const
{
	return (*this)[std::string(key)];
}

const std::string& translate_string(const std::string& str)
{
	return translate_string_default(str,str);
}

const std::string& translate_string_default(const std::string& str, const std::string& default_val)
{
	const string_map::const_iterator i = strings_.find(str);
	if(i != strings_.end() && i->second != "")
		return i->second;
	else
		return default_val;
}

std::vector<std::string> get_languages()
{
	std::vector<std::string> res;

	config cfg;

	try {
		cfg.read(preprocess_file("data/translations/",NULL,NULL));
	} catch(config::error& e) {
		std::cerr << "could not open translations: '" << e.message << "' -- defaulting to English only\n";
		res.push_back("English");
		return res;
	}

	const config::child_list& lang = cfg.get_children("language");
	for(config::child_list::const_iterator i = lang.begin(); i != lang.end(); ++i) {
		res.push_back((**i)["language"]);
	}

	return res;
}

namespace {
bool internal_set_language(const std::string& locale, config& cfg)
{
	const config::child_list& lang = cfg.get_children("language");
	for(config::child_list::const_iterator i = lang.begin(); i != lang.end(); ++i) {
		if((**i)["id"] == locale || (**i)["language"] == locale) {

			current_language = (**i)["language"];

			const std::string& enc = (**i)["encoding"];
			if(enc == "UTF-8") {
				charset_used = CHARSET_UTF8;
			} else if(enc == "LATIN1" || enc == "") {
				charset_used = CHARSET_LATIN1;
			} else {
				std::cerr << "Unrecognized character set: '" << enc
				          << "' (defaulting to LATIN1)\n";
				charset_used = CHARSET_LATIN1;
			}

			for(string_map::const_iterator j = (*i)->values.begin(); j != (*i)->values.end(); ++j) {
				strings_[j->first] = j->second;
			}

			hotkey::add_hotkeys(**i,false);

			font::set_font((**i)["font"]);

			return true;
		}
	}

	return false;
}
}

bool set_language(const std::string& locale)
{
	strings_.clear();

	std::string locale_lc;
	locale_lc.resize(locale.size());
	std::transform(locale.begin(),locale.end(),locale_lc.begin(),tolower);

	config cfg;
	if(locale_lc == "en" || locale_lc == "english") {
		try {
			cfg.read(preprocess_file("data/translations/english.cfg"));
		} catch(config::error& e) {
			std::cerr << "Could not read english.cfg\n";
			throw e;
		}
	} else {
		try {
			cfg.read(preprocess_file("data/translations/"));
			
			//default to English locale first, then set desired locale
			internal_set_language("en",cfg);
		} catch(config::error& e) {
			std::cerr << "error opening translations: '" << e.message << "' Defaulting to English\n";
			return set_language("english");
		}
	}

	return internal_set_language(locale,cfg);
}

const std::string& get_language() { return current_language; }

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

class invalid_utf8_exception : public std::exception {
};

namespace 
{
std::string wstring_to_utf8(const std::wstring &src)
{
    wchar_t ch;
	std::wstring::const_iterator i;
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

std::wstring utf8_to_wstring(const std::string &src)
{
	std::wstring ret;	
    wchar_t ch;
	std::string::const_iterator i;
	
	try {
		for(i = src.begin(); i != src.end(); ++i ) {
			ch = (unsigned char)*i;
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
		
			if(i + count - 1 == src.end())
				throw invalid_utf8_exception();

			/* Convert the first character */
			if (count != 1) {
				ch &= 0xFF >> (count + 1);
			}
			
			/* Convert the continuation bytes */
			for(std::string::const_iterator j = i+1; j != i+count; ++j) {
				if((*j & 0xC0) != 0x80)
					throw invalid_utf8_exception();
				
				ch = (ch << 6) | (*j & 0x3F);
			}
			i += (count - 1);
			
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

std::string wstring_to_string(const std::wstring &src)
{
	if(charset() == CHARSET_UTF8) {
		return wstring_to_utf8(src);
	}
	
	std::string ret;
	for(std::wstring::const_iterator itor = src.begin(); itor != src.end(); ++itor) {
		if(*itor <= 0xff) {
			push_back(ret,*itor);
		} else {
			push_back(ret,'?');
		}
	}
	return ret;
}

std::wstring string_to_wstring(const std::string &src)
{
	if(charset() == CHARSET_UTF8) {
		return utf8_to_wstring(src);
	}

	std::wstring ret;
	ret.resize(src.size());
	std::copy(src.begin(),src.end(),ret.begin());

	return ret;
}


