/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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

#include "tstring.hpp"
#include "serialization/string_utils.hpp"

#include <iterator>

//this module controls internationalization.

class config;

struct language_def
{
	language_def() :
		localename(),
		alternates(),
		language(),
		sort_name(),
		rtl(false),
		percent(100)
		{}

	language_def(const std::string& name, const t_string& lang, const std::string& dir,
		    const std::string &salternates = "", const std::string& sort_name = "", const std::string& percent = "100") :
		localename(name),
		alternates(utils::split(salternates)),
		language(lang),
		sort_name(sort_name.empty() ? std::string(lang) : sort_name),
		rtl(dir == "rtl")
	{
		try {
			this->percent = percent.empty() ? 100 : std::stoi(percent);
		} catch(std::invalid_argument&) {
			this->percent = 100;
		}
    }

	std::string localename;
	std::vector<std::string> alternates;
	t_string language;
	std::string sort_name;
	bool rtl;		// A right to left language? (e.g: Hebrew)
	/** % of translated text in core po-s */
	int percent;
	bool operator== (const language_def&) const;
	bool operator< (const language_def& a) const
		{ return sort_name < a.sort_name; }
};

typedef std::vector<language_def> language_list;

struct symbol_table
{
	const t_string& operator[](const std::string& key) const;
	const t_string& operator[](const char* key) const;
};

//table of strings which are displayed to the user. Maps ids -> text.
//this table should be consulted whenever something is to be
//displayed on screen.
extern symbol_table string_table;

bool& time_locale_correct();

//function which, given the main configuration object, will return
//a list of the translations of the game available.
std::vector<language_def> get_languages();

//function which, given the main configuration object, and a locale,
//will set string_table to be populated with data from that locale.
//locale may be either the full name of the language, like 'English',
//or the 2-letter version, like 'en'.
void set_language(const language_def& locale);

//function which returns the name of the language currently used
const language_def& get_language();
bool current_language_rtl();

//function which attempts to query and return the locale on the system
const language_def& get_locale();

/** Initializes the list of textdomains from a configuration object */
void init_textdomains(const config& cfg);

/** Initializes certain English strings */
bool init_strings(const config& cfg);

bool load_language_list();
