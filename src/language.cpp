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

#include "filesystem.hpp"
#include "gettext.hpp"
#include "language.hpp"
#include "log.hpp"
#include "preferences/preferences.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "game_config_manager.hpp"

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

#ifdef __ANDROID__
#include <SDL3/SDL_system.h> // For SDL Android functions
#include <jni.h>
#endif

#define DBG_G LOG_STREAM(debug, lg::general())
#define LOG_G LOG_STREAM(info, lg::general())
#define WRN_G LOG_STREAM(warn, lg::general())
#define ERR_G LOG_STREAM(err, lg::general())

namespace {
	language_def current_language;
	std::vector<config> languages;
	std::vector<language_def> known_languages;
	utils::string_map strings_;
	int min_translation_percent = 80;
	// a storage for looking up active translation name
	// corresponding to a locale id for locale ids that Wesnoth supports.
	std::map<std::string, std::string> translation_names;
}

bool load_strings(bool complain);

bool language_def::operator== (const language_def& a) const
{
	return ((language == a.language) /* && (localename == a.localename) */ );
}

std::string language_def::short_localename() const {
	std::string::size_type index = localename.find(
#ifdef _WIN32
		'-'
#else
		'_'
#endif
	);
	return index == std::string::npos ? localename : localename.substr(0, index);
}

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

utils::string_map::const_iterator symbol_table::find(const std::string& key) const
{
	return strings_.find(key);
}

utils::string_map::const_iterator symbol_table::end() const
{
	return strings_.end();
}

language_def::language_def()
	: language(t_string(N_("System default language"), "wesnoth"))
	, sort_name("A")
{
}

language_def::language_def(const config& cfg)
#ifndef _WIN32
	: localename(cfg["locale"])
	, alternates(utils::split(cfg["alternates"]))
#else
	: localename(cfg["windows_locale"].str("C"))
	, alternates(utils::split(cfg["windows_alternates"]))
#endif
	, language(cfg["name"].t_str())
	, sort_name(cfg["sort_name"].str(language))
	, rtl(cfg["dir"] == "rtl")
	, percent(cfg["percent"].to_int())
{
	// entry for main language in the locale id and name map
	translation_names.emplace(localename, language);

	for(const auto& alternate : alternates) {
		translation_names.emplace(alternate, language);
	}
}

bool load_language_list()
{
	try {
		config cfg = io::read(*preprocess_file(filesystem::get_wml_location("hardwired/language.cfg").value()));

		known_languages.clear();
		known_languages.emplace_back(); // System default language

		for(const config& lang : cfg.child_range("locale")) {
			known_languages.emplace_back(lang);
		}

		return true;

	} catch(const utils::bad_optional_access&) {
		return false;
	} catch(const config::error&) {
		return false;
	}
}

std::vector<language_def> get_languages(bool all)
{
	// We sort every time, the local might have changed which can modify the
	// sort order.
	std::sort(known_languages.begin(), known_languages.end());

	if(all || min_translation_percent == 0) {
		LOG_G << "Found " << known_languages.size() << " known languages";
		return known_languages;
	}

	std::vector<language_def> result;
	std::copy_if(known_languages.begin(), known_languages.end(), std::back_inserter(result),
		[](const language_def& lang) { return lang.percent >= min_translation_percent; });

	LOG_G << "Found " << result.size() << " sufficiently translated languages";
	return result;
}

std::string get_translation_name(const std::string& locale_id)
{
	auto itor = translation_names.find(locale_id);
	return itor != translation_names.end() ? itor->second : "";
}

int get_min_translation_percent()
{
	return min_translation_percent;
}

void set_min_translation_percent(int percent) {
	min_translation_percent = percent;
}


static void wesnoth_setlocale(int category, const std::string& slocale,
	std::vector<std::string> const *alternates)
{
	std::string locale = slocale;

	//category is never LC_MESSAGES since that case was moved to gettext.cpp to remove the dependency to libintl.h in this file
	//that's why code like if (category == LC_MESSAGES) is outcommented here.
#ifdef __APPLE__
	//if (category == LC_MESSAGES && setenv("LANG", locale.c_str(), 1) == -1) {
	//	ERR_G << "setenv LANG failed: " << strerror(errno);
	//}
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
				LOG_G << "Set locale to '" << locale << "' result: '" << res << "'.";
				goto done;
			}
		}

		if (!alternates || i == alternates->end()) break;
		locale = *i;
		++i;
	}

	WRN_G << "setlocale() failed for '" << slocale << "'.";

	if (category == LC_TIME) {
		time_locale_correct() = false;
	}

#ifndef _WIN32
		//if(category == LC_MESSAGES) {
		//	WRN_G << "Setting LANGUAGE to '" << slocale << "'.";
		//	setenv("LANGUAGE", slocale.c_str(), 1);
		//	std::setlocale(LC_MESSAGES, "");
		//}
#endif

	done:
	DBG_G << "Numeric locale: " << std::setlocale(LC_NUMERIC, nullptr);
	DBG_G << "Full locale: " << std::setlocale(LC_ALL, nullptr);
}

void set_language(const language_def& locale)
{
	strings_.clear();

	current_language = locale;
	time_locale_correct() = true;

	std::string localename = locale.localename;

#ifdef __ANDROID__
	if (locale.localename.empty()) {
		JNIEnv* env = reinterpret_cast<JNIEnv*>(SDL_GetAndroidJNIEnv());
		jobject wesnoth_instance = reinterpret_cast<jobject>(SDL_GetAndroidActivity());
		jclass wesnoth_activity(env->GetObjectClass(wesnoth_instance));
		jmethodID locale = env->GetMethodID(wesnoth_activity, "getLocaleCode", "()Ljava/lang/String;");
		jstring lcode = reinterpret_cast<jstring>(env->CallObjectMethod(wesnoth_instance, locale));
		localename = env->GetStringUTFChars(lcode, nullptr);

		if(env->ExceptionCheck() == JNI_TRUE) {
			env->ExceptionDescribe();
			env->ExceptionClear();
		}

		env->DeleteLocalRef(wesnoth_instance);
		env->DeleteLocalRef(wesnoth_activity);
	}
#endif

	wesnoth_setlocale(LC_COLLATE, localename, &locale.alternates);
	wesnoth_setlocale(LC_TIME, localename, &locale.alternates);
	translation::set_language(localename, &locale.alternates);
	load_strings(false);
}

bool load_strings(bool complain)
{
	if(complain && languages.empty()) {
		PLAIN_LOG << "No [language] block found";
		return false;
	}
	for(const config& lang : languages) {
		for(const auto& [key, value] : lang.attribute_range()) {
			strings_[key] = value.t_str();
		}
	}

	return true;
}

const language_def& get_language() { return current_language; }

const language_def& get_locale()
{
	assert(!known_languages.empty());

	const std::string& prefs_locale = prefs::get().locale();
	if(prefs_locale.empty() == false) {
		translation::set_language(prefs_locale, nullptr);
		for(const language_def& def : known_languages) {
			if(prefs_locale == def.localename) {
				return def;
			}
		}
		LOG_G << "'" << prefs_locale << "' locale not found in known array; defaulting to system locale";
		return known_languages[0];
	}

	LOG_G << "locale could not be determined; defaulting to system locale";
	return known_languages[0];
}

void init_textdomains(const game_config_view& cfg)
{
	for (const config &t : cfg.child_range("textdomain"))
	{
		const std::string &name = t["name"];
		const std::string &path = t["path"];

		if(path.empty()) {
			t_string::add_textdomain(name, filesystem::get_intl_dir());
		} else if(auto location = filesystem::get_binary_dir_location("", path)) {
			t_string::add_textdomain(name, location.value());
		} else {
			// If location is empty, this causes a crash on Windows, so we disallow adding empty domains
			WRN_G << "no location found for '" << path << "', skipping textdomain";
		}
	}
}

bool init_strings(const game_config_view& cfg)
{
	languages.clear();
	for (const config &l : cfg.child_range("language")) {
		languages.push_back(l);
	}
	return load_strings(true);
}
