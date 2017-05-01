/*
   Copyright (C) 2015 - 2017 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "build_info.hpp"

#include "formatter.hpp"
#include "gettext.hpp"

#include <algorithm>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include <boost/version.hpp>

#include <pango/pangocairo.h>

#ifdef HAVE_LIBPNG
#include <png.h>
#endif

namespace game_config
{

namespace {

struct version_table_manager
{
	std::vector<std::string> compiled, linked, names;
	std::vector<optional_feature> features;

	version_table_manager();
};

const version_table_manager versions;

#if 0
std::string format_version(unsigned a, unsigned b, unsigned c)
{
	return formatter() << a << '.' << b << '.' << c;
}
#endif

std::string format_version(const SDL_version& v)
{
	return formatter() << unsigned(v.major) << '.'
			    		<< unsigned(v.minor) << '.'
						<< unsigned(v.patch);
}

version_table_manager::version_table_manager()
	: compiled(LIB_COUNT, "")
	, linked(LIB_COUNT, "")
	, names(LIB_COUNT, "")
	, features()
{
	SDL_version sdl_version;
	const SDL_version* sdl_rt_version = nullptr;


	//
	// SDL
	//

	SDL_VERSION(&sdl_version);
	compiled[LIB_SDL] = format_version(sdl_version);

	SDL_GetVersion(&sdl_version);
	linked[LIB_SDL] = format_version(sdl_version);

	names[LIB_SDL] = "SDL";

	//
	// SDL_image
	//

	SDL_IMAGE_VERSION(&sdl_version);
	compiled[LIB_SDL_IMAGE] = format_version(sdl_version);

	sdl_rt_version = IMG_Linked_Version();
	if(sdl_rt_version) {
		linked[LIB_SDL_IMAGE] = format_version(*sdl_rt_version);
	}

	names[LIB_SDL_IMAGE] = "SDL_image";

	//
	// SDL_mixer
	//

	SDL_MIXER_VERSION(&sdl_version);
	compiled[LIB_SDL_MIXER] = format_version(sdl_version);

	sdl_rt_version = Mix_Linked_Version();
	if(sdl_rt_version) {
		linked[LIB_SDL_MIXER] = format_version(*sdl_rt_version);
	}

	names[LIB_SDL_MIXER] = "SDL_mixer";

	//
	// SDL_ttf
	//

	SDL_TTF_VERSION(&sdl_version);
	compiled[LIB_SDL_TTF] = format_version(sdl_version);

	sdl_rt_version = TTF_Linked_Version();
	if(sdl_rt_version) {
		linked[LIB_SDL_TTF] = format_version(*sdl_rt_version);
	}

	names[LIB_SDL_TTF] = "SDL_ttf";

	//
	// Boost
	//

	compiled[LIB_BOOST] = BOOST_LIB_VERSION;
	std::replace(compiled[LIB_BOOST].begin(), compiled[LIB_BOOST].end(), '_', '.');
	names[LIB_BOOST] = "Boost";

	//
	// Cairo
	//

	compiled[LIB_CAIRO] = CAIRO_VERSION_STRING;
	linked[LIB_CAIRO] = cairo_version_string();
	names[LIB_CAIRO] = "Cairo";

	//
	// Pango
	//

	compiled[LIB_PANGO] = PANGO_VERSION_STRING;
	linked[LIB_PANGO] = pango_version_string();
	names[LIB_PANGO] = "Pango";

	//
	// libpng
	//

#ifdef HAVE_LIBPNG
	compiled[LIB_PNG] = PNG_LIBPNG_VER_STRING;
	linked[LIB_PNG] = png_get_libpng_ver(nullptr);
	names[LIB_PNG] = "libpng";
#endif

	//
	// Features table.
	//

	features.emplace_back(N_("feature^Experimental OpenMP support"));
#ifdef _OPENMP
	features.back().enabled = true;
#endif

	features.emplace_back(N_("feature^PNG screenshots"));
#ifdef HAVE_LIBPNG
	features.back().enabled = true;
#endif

	features.emplace_back(N_("feature^Lua console completion"));
#ifdef HAVE_HISTORY
	features.back().enabled = true;
#endif

	features.emplace_back(N_("feature^Legacy bidirectional rendering"));
#ifdef HAVE_FRIBIDI
	features.back().enabled = true;
#endif

#ifdef _X11

	features.emplace_back(N_("feature^D-Bus notifications back end"));
#ifdef HAVE_LIBDBUS
	features.back().enabled = true;
#endif

#endif /* _X11 */

#ifdef _WIN32
	// Always compiled in.
	features.emplace_back(N_("feature^Win32 notifications back end"));
	features.back().enabled = true;
#endif

#ifdef __APPLE__

	features.emplace_back(N_("feature^Cocoa notifications back end"));
#ifdef HAVE_NS_USER_NOTIFICATION
	features.back().enabled = true;
#endif

	features.emplace_back(N_("feature^Growl notifications back end"));
#ifdef HAVE_GROWL
	features.back().enabled = true;
#endif

#endif /* __APPLE__ */
}

const std::string empty_version = "";

} // end anonymous namespace 1

std::vector<optional_feature> optional_features_table()
{
	std::vector<optional_feature> res = versions.features;

	for(size_t k = 0; k < res.size(); ++k) {
		res[k].name = _(res[k].name.c_str());
	}
	return res;
}

const std::string& library_build_version(LIBRARY_ID lib)
{
	if(lib >= LIB_COUNT) {
		return empty_version;
	}

	return versions.compiled[lib];
}

const std::string& library_runtime_version(LIBRARY_ID lib)
{
	if(lib >= LIB_COUNT) {
		return empty_version;
	}

	return versions.linked[lib];
}

const std::string& library_name(LIBRARY_ID lib)
{
	if(lib >= LIB_COUNT) {
		return empty_version;
	}

	return versions.names[lib];
}

namespace {

bool strlen_comparator(const std::string& a, const std::string& b)
{
	return a.length() < b.length();
}

size_t max_strlen(const std::vector<std::string>& strs)
{
	const std::vector<std::string>::const_iterator it =
			std::max_element(strs.begin(), strs.end(), strlen_comparator);

	return it != strs.end() ? it->length() : 0;
}

} // end anonymous namespace 2

std::string library_versions_report()
{
	std::ostringstream o;

	const size_t col2_start = max_strlen(versions.names) + 2;
	const size_t col3_start = max_strlen(versions.compiled) + 1;

	for(unsigned n = 0; n < LIB_COUNT; ++n)
	{
		const std::string& name = versions.names[n];
		const std::string& compiled = versions.compiled[n];
		const std::string& linked = versions.linked[n];

		if(name.empty()) {
			continue;
		}

		o << name << ": ";

		const size_t pos2 = name.length() + 2;
		if(pos2 < col2_start) {
			o << std::string(col2_start - pos2, ' ');
		}

		o << compiled;

		if(!linked.empty()) {
			const size_t pos3 = compiled.length() + 1;
			if(pos3 < col3_start) {
				o << std::string(col3_start - pos3, ' ');
			}
			o << " (runtime " << linked << ")";
		}

		o << '\n';
	}

	return o.str();
}

std::string optional_features_report()
{
	// Yes, it's for stdout/stderr but we still want the localized version so
	// that the context prefixes are stripped.
	const std::vector<optional_feature>& features = optional_features_table();

	size_t col2_start = 0;

	for(size_t k = 0; k < features.size(); ++k)
	{
		col2_start = std::max(col2_start, features[k].name.length() + 2);
	}

	std::ostringstream o;

	for(size_t k = 0; k < features.size(); ++k)
	{
		const optional_feature& f = features[k];

		o << f.name << ": ";

		const size_t pos2 = f.name.length() + 2;
		if(pos2 < col2_start) {
			o << std::string(col2_start - pos2, ' ');
		}

		o << (f.enabled ? "yes" : "no") << '\n';
	}

	return o.str();
}

} // end namespace game_config
