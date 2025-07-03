/*
	Copyright (C) 2003 - 2025
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

class game_config_view;

//this module controls internationalization.

struct language_def
{
	/** Creates the 'System Default Language' definition. */
	language_def();

	explicit language_def(const config& cfg);

	std::string localename;
	std::vector<std::string> alternates;
	t_string language;
	std::string sort_name;

	/** A right to left language? (e.g: Hebrew) */
	bool rtl = false;

	/** % of translated text in core po-s */
	int percent = 100;

	bool operator==(const language_def&) const;
	bool operator<(const language_def& a) const
	{
		return sort_name < a.sort_name;
	}
};

struct symbol_table
{
	/**
	 * Look up the string mappings given in [language] tags. If the key is not
	 * found, fall back to returning a string that's only meant for developers
	 * to see.
	 */
	const t_string& operator[](const std::string& key) const;
	const t_string& operator[](const char* key) const;
	/**
	 * Look up the string mappings given in [language] tags. If the key is not
	 * found, returns symbol_table::end().
	 */
	utils::string_map::const_iterator find(const std::string& key) const;
	utils::string_map::const_iterator end() const;
};

//table of strings which are displayed to the user. Maps ids -> text.
inline auto string_table = symbol_table{};

bool& time_locale_correct();

/**
 * Return a list of available translations.
 *
 * The list will normally be filtered with incomplete (according to
 * min_translation_percent) translations removed.
 *
 *@param all if true, include incomplete translations
 *@pre load_language_list() has already been called
 */
std::vector<language_def> get_languages(bool all=false);

//function which, given the main configuration object, and a locale,
//will set string_table to be populated with data from that locale.
//locale may be either the full name of the language, like 'English',
//or the 2-letter version, like 'en'.
void set_language(const language_def& locale);

//function which returns the name of the language currently used
const language_def& get_language();

//function which attempts to query and return the locale on the system
const language_def& get_locale();

/** Initializes the list of textdomains from a configuration object */
void init_textdomains(const game_config_view& cfg);

/** Initializes certain English strings */
bool init_strings(const game_config_view& cfg);

bool load_language_list();

int get_min_translation_percent();
void set_min_translation_percent(int percent);
