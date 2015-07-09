/*
   Copyright (C) 2015 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "build_info.hpp"

#include "formatter.hpp"

#include <algorithm>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <SDL_net.h>

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

	version_table_manager();
};

const version_table_manager versions;

#if 0
std::string format_version(unsigned a, unsigned b, unsigned c)
{
	return (formatter() << a << '.' << b << '.' << c).str();
}
#endif

std::string format_version(const SDL_version& v)
{
	return (formatter() << unsigned(v.major) << '.'
						<< unsigned(v.minor) << '.'
						<< unsigned(v.patch)).str();
}

version_table_manager::version_table_manager()
	: compiled(LIB_COUNT, "")
	, linked(LIB_COUNT, "")
	, names(LIB_COUNT, "")
{
	SDL_version sdl_version;
	const SDL_version* sdl_rt_version = NULL;

	//
	// SDL
	//

	SDL_VERSION(&sdl_version);
	compiled[LIB_SDL] = format_version(sdl_version);

	sdl_rt_version = SDL_Linked_Version();
	if(sdl_rt_version) {
		linked[LIB_SDL] = format_version(*sdl_rt_version);
	}

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
	// SDL_net
	//

	SDL_NET_VERSION(&sdl_version);
	compiled[LIB_SDL_NET] = format_version(sdl_version);

	sdl_rt_version = SDLNet_Linked_Version();
	if(sdl_rt_version) {
		linked[LIB_SDL_NET] = format_version(*sdl_rt_version);
	}

	names[LIB_SDL_NET] = "SDL_net";

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
	linked[LIB_PNG] = png_get_libpng_ver(NULL);
	names[LIB_PNG] = "libpng";
#endif
}

const std::string empty_version = "";

} // end anonymous namespace 1

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

} // end namespace game_config
