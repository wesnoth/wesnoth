/*
	Copyright (C) 2016 - 2025
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
#include "font/error.hpp"

#include "config.hpp"
#include "log.hpp"
#include "tstring.hpp"

#include "filesystem.hpp"

#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"

#include <sstream>


#include <fontconfig/fontconfig.h>

static lg::log_domain log_font("font");
#define DBG_FT LOG_STREAM(debug, log_font)
#define LOG_FT LOG_STREAM(info, log_font)
#define WRN_FT LOG_STREAM(warn, log_font)
#define ERR_FT LOG_STREAM(err, log_font)

namespace font
{
namespace
{

/** Records the game's families for sans serif, script, and monospace fonts */
struct font_families
{
	font_families() = default;

	explicit font_families(const config& cfg)
		: sans(cfg["family_order"].t_str())
		, mono(cfg["family_order_monospace"].t_str())
		, script(cfg["family_order_script"].t_str())
	{
		if(mono.empty()) {
			ERR_FT << "No monospace font family defined, falling back to sans serif";
			mono = sans;
		}

		if(script.empty()) {
			ERR_FT << "No script font family defined, falling back to sans serif";
			script = sans;
		}
	}

	t_string sans;
	t_string mono;
	t_string script;
};

font_families families;

} //namespace

/***
 * Public interface
 */

bool load_font_config()
try {
	auto stream = preprocess_file(filesystem::get_wml_location("hardwired/fonts.cfg").value());
	families = font_families{ io::read(*stream).mandatory_child("fonts") };
	return true;

} catch(const utils::bad_optional_access&) {
	ERR_FT << "could not resolve path to fonts.cfg, file not found";
	return false;

} catch(const config::error& e) {
	ERR_FT << "could not read fonts.cfg:\n" << e.message;
	return false;
}

const t_string& get_font_families(family_class fclass)
{
	switch(fclass) {
	case family_class::monospace:
		return families.mono;
	case family_class::script:
		return families.script;
	default:
		return families.sans;
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
	std::string font_file_contents = filesystem::read_file(font_file);

// msys2 crosscompiling for windows for whatever reason makes the cache directory prefer using drives other than C:
// ie - D:\a\msys64\var\cache\fontconfig
// fontconfig also does not seem to provide a way to set the cachedir for a specific platform
// so load the fonts.conf file into memory and only for windows insert the cachedir configuration
#ifdef _WIN32
	font_file_contents.insert(font_file_contents.find("</fontconfig>"), "<cachedir>"+filesystem::get_cache_dir()+"</cachedir>\n");
#endif

	if(!FcConfigParseAndLoadFromMemory(FcConfigGetCurrent(),
							 reinterpret_cast<const FcChar8*>(font_file_contents.c_str()),
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
