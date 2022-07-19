/*
	Copyright (C) 2016 - 2022
	by Chris Beck<render787@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include <fontconfig/fontconfig.h>

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
				WRN_FT << "Failed opening font file '" << name << "': No such file or directory";
				return false;
				}
			}
		}
	} else {
		if(!filesystem::file_exists("fonts/" + name)) {
			if(!filesystem::file_exists(name)) {
				WRN_FT << "Failed opening font file '" << name << "': No such file or directory";
				return false;
			}
		}
	}
	return true;
}

namespace
{

// Current font family for sanserif and monospace fonts in the game

t_string family_order_sans;
t_string family_order_mono;
t_string family_order_light;
t_string family_order_script;

} // end anon namespace

/***
 * Public interface
 */

bool load_font_config()
{
	config cfg;
	try {
		const std::string& cfg_path = filesystem::get_wml_location("hardwired/fonts.cfg");
		if(cfg_path.empty()) {
			ERR_FT << "could not resolve path to fonts.cfg, file not found";
			return false;
		}

		filesystem::scoped_istream stream = preprocess_file(cfg_path);
		read(cfg, *stream);
	} catch(const config::error &e) {
		ERR_FT << "could not read fonts.cfg:\n" << e.message;
		return false;
	}

	const config &fonts_config = cfg.child("fonts");
	if (!fonts_config)
		return false;

	family_order_sans = fonts_config["family_order"];
	family_order_mono = fonts_config["family_order_monospace"];
	family_order_light = fonts_config["family_order_light"];
	family_order_script = fonts_config["family_order_script"];

	if(family_order_mono.empty()) {
		ERR_FT << "No monospace font family order defined, falling back to sans serif order";
		family_order_mono = family_order_sans;
	}

	if(family_order_light.empty()) {
		ERR_FT << "No light font family order defined, falling back to sans serif order";
		family_order_light = family_order_sans;
	}

	if(family_order_script.empty()) {
		ERR_FT << "No script font family order defined, falling back to sans serif order";
		family_order_script = family_order_sans;
	}

	return true;
}

const t_string& get_font_families(family_class fclass)
{
	switch(fclass) {
	case FONT_MONOSPACE:
		return family_order_mono;
	case FONT_LIGHT:
		return family_order_light;
	case FONT_SCRIPT:
		return family_order_script;
	default:
		return family_order_sans;
	}
}

/***
 * Manager member functions
 */

manager::manager()
{
	std::string font_path = game_config::path + "/fonts";
	if (!FcConfigAppFontAddDir(FcConfigGetCurrent(),
		reinterpret_cast<const FcChar8 *>(font_path.c_str())))
	{
		ERR_FT << "Could not load the true type fonts";
		throw font::error("font config lib failed to add the font path: '" + font_path + "'");
	}

	std::string font_file = font_path + "/fonts.conf";
	if(!FcConfigParseAndLoad(FcConfigGetCurrent(),
							 reinterpret_cast<const FcChar8*>(font_file.c_str()),
							 FcFalse))
	{
		ERR_FT << "Could not load local font configuration";
		throw font::error("font config lib failed to find font.conf: '" + font_file + "'");
	}
	else
	{
		LOG_FT << "Local font configuration loaded";
	}
}

manager::~manager()
{
	FcConfigAppFontClear(FcConfigGetCurrent());
}

} // end namespace font
