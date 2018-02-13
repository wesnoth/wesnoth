/*
   Copyright (C) 2016 - 2018 by Chris Beck<render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "font/font_config.hpp"
#include "font/font_description.hpp"
#include "font/error.hpp"
#include "font/sdl_ttf.hpp"

#include "config.hpp"
#include "log.hpp"
#include "tstring.hpp"

#include "filesystem.hpp"
#include "game_config.hpp"

#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"
#include "preferences/general.hpp"

#include <list>
#include <set>
#include <stack>
#include <sstream>
#include <vector>

#include <cairo-features.h>

#ifdef CAIRO_HAS_WIN32_FONT
#include <windows.h>
#undef CAIRO_HAS_FT_FONT
#endif

#ifdef CAIRO_HAS_FT_FONT
#include <fontconfig/fontconfig.h>
#endif

#if !defined(CAIRO_HAS_FT_FONT) && !defined(CAIRO_HAS_WIN32_FONT)
#error unable to find font loading tools.
#endif

static lg::log_domain log_font("font");
#define DBG_FT LOG_STREAM(debug, log_font)
#define LOG_FT LOG_STREAM(info, log_font)
#define WRN_FT LOG_STREAM(warn, log_font)
#define ERR_FT LOG_STREAM(err, log_font)

namespace font {


bool check_font_file(std::string name) {
	if(game_config::path.empty() == false) {
		if(!filesystem::file_exists(game_config::path + "/fonts/" + name)) {
			if(!filesystem::file_exists("fonts/" + name)) {
				if(!filesystem::file_exists(name)) {
				WRN_FT << "Failed opening font file '" << name << "': No such file or directory" << std::endl;
				return false;
				}
			}
		}
	} else {
		if(!filesystem::file_exists("fonts/" + name)) {
			if(!filesystem::file_exists(name)) {
				WRN_FT << "Failed opening font file '" << name << "': No such file or directory" << std::endl;
				return false;
			}
		}
	}
	return true;
}

static bool add_font_to_fontlist(const config &fonts_config,
	std::vector<font::subset_descriptor>& fontlist, const std::string& name)
{
	const config &font = fonts_config.find_child("font", "name", name);
	if (!font) {
		return false;
	}
	//DBG_FT << "Adding a font record: " << font.debug() << std::endl;

	fontlist.push_back(font::subset_descriptor(font));

	return true;
}

// Current font family for sanserif and monospace fonts in the game

t_string family_order_sans;
t_string family_order_mono;
t_string family_order_light;

/***
 * Public interface
 */

bool load_font_config()
{
	//read font config separately, so we do not have to re-read the whole
	//config when changing languages
	config cfg;
	try {
		const std::string& cfg_path = filesystem::get_wml_location("hardwired/fonts.cfg");
		if(cfg_path.empty()) {
			ERR_FT << "could not resolve path to fonts.cfg, file not found\n";
			return false;
		}

		filesystem::scoped_istream stream = preprocess_file(cfg_path);
		read(cfg, *stream);
	} catch(config::error &e) {
		ERR_FT << "could not read fonts.cfg:\n"
		       << e.message << '\n';
		return false;
	}

	const config &fonts_config = cfg.child("fonts");
	if (!fonts_config)
		return false;

	std::set<std::string> known_fonts;
	for (const config &font : fonts_config.child_range("font")) {
		known_fonts.insert(font["name"]);
		if (font.has_attribute("bold_name")) {
			known_fonts.insert(font["bold_name"]);
		}
		if (font.has_attribute("italic_name")) {
			known_fonts.insert(font["italic_name"]);
		}
	}

	family_order_sans = fonts_config["family_order"];
	family_order_mono = fonts_config["family_order_monospace"];
	family_order_light = fonts_config["family_order_light"];

	if(family_order_mono.empty()) {
		ERR_FT << "No monospace font family order defined, falling back to sans serif order\n";
		family_order_mono = family_order_sans;
	}

	if(family_order_light.empty()) {
		ERR_FT << "No light font family order defined, falling back to sans serif order\n";
		family_order_light = family_order_sans;
	}

	std::vector<font::subset_descriptor> fontlist;

	for(auto font : utils::split(fonts_config["order"])) {
		add_font_to_fontlist(fonts_config, fontlist, font);
		known_fonts.erase(font);
	}

	for(auto kfont : known_fonts) {
		add_font_to_fontlist(fonts_config, fontlist, kfont);
	}

	if(fontlist.empty())
		return false;

	sdl_ttf::set_font_list(fontlist);
	return true;
}

const t_string& get_font_families(family_class fclass)
{
	switch(fclass) {
	case FONT_MONOSPACE:
		return family_order_mono;
	case FONT_LIGHT:
		return family_order_light;
	default:
		return family_order_sans;
	}
}

/***
 * Manager member functions
 */

manager::manager()
{
#ifdef CAIRO_HAS_FT_FONT
	std::string font_path = game_config::path + "/fonts";
	if (!FcConfigAppFontAddDir(FcConfigGetCurrent(),
		reinterpret_cast<const FcChar8 *>(font_path.c_str())))
	{
		ERR_FT << "Could not load the true type fonts" << std::endl;
		throw font::error("font config lib failed to add the font path: '" + font_path + "'");
	}

	std::string font_file = font_path + "/fonts.conf";
	if(!FcConfigParseAndLoad(FcConfigGetCurrent(),
							 reinterpret_cast<const FcChar8*>(font_file.c_str()),
							 FcFalse))
	{
		ERR_FT << "Could not load local font configuration\n";
		throw font::error("font config lib failed to find font.conf: '" + font_file + "'");
	}
	else
	{
		LOG_FT << "Local font configuration loaded\n";
	}
#endif

#if CAIRO_HAS_WIN32_FONT
	for(const std::string& path : filesystem::get_binary_paths("fonts")) {
		std::vector<std::string> files;
		if(filesystem::is_directory(path)) {
			filesystem::get_files_in_dir(path, &files, nullptr, filesystem::ENTIRE_FILE_PATH);
		}
		for(const std::string& file : files) {
			if(file.substr(file.length() - 4) == ".ttf" || file.substr(file.length() - 4) == ".ttc")
			{
				const std::wstring wfile = unicode_cast<std::wstring>(file);
				AddFontResourceExW(wfile.c_str(), FR_PRIVATE, nullptr);
			}
		}
	}
#endif
}

manager::~manager()
{
#ifdef CAIRO_HAS_FT_FONT
	FcConfigAppFontClear(FcConfigGetCurrent());
#endif

#if CAIRO_HAS_WIN32_FONT
	for(const std::string& path : filesystem::get_binary_paths("fonts")) {
		std::vector<std::string> files;
		if(filesystem::is_directory(path))
			filesystem::get_files_in_dir(path, &files, nullptr, filesystem::ENTIRE_FILE_PATH);
		for(const std::string& file : files) {
			if(file.substr(file.length() - 4) == ".ttf" || file.substr(file.length() - 4) == ".ttc")
			{
				const std::wstring wfile = unicode_cast<std::wstring>(file);
				RemoveFontResourceExW(wfile.c_str(), FR_PRIVATE, nullptr);
			}
		}
	}
#endif
}


} // end namespace font
