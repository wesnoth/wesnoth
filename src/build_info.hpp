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

#ifndef BUILD_CONFIG_HPP_INCLUDED
#define BUILD_CONFIG_HPP_INCLUDED

#include <string>
#include <vector>

namespace game_config
{

enum LIBRARY_ID
{
	LIB_BOOST,

	LIB_CAIRO,
	LIB_PANGO,

	LIB_SDL,
	LIB_SDL_IMAGE,
	LIB_SDL_MIXER,
	LIB_SDL_TTF,
	LIB_SDL_NET,
	LIB_PNG,

	LIB_COUNT
};

struct optional_feature
{
	std::string name;
	bool enabled;

	optional_feature(const char* n) : name(n), enabled(false) {}
};

/**
 * Return a localized features table.
 */
std::vector<optional_feature> optional_features_table();

/**
 * Produce a plain-text report of features suitable for stdout/stderr.
 */
std::string optional_features_report();

/**
 * Retrieve the build-time version number of the given library.
 */
const std::string& library_build_version(LIBRARY_ID lib);

/**
 * Retrieve the runtime version number of the given library.
 */
const std::string& library_runtime_version(LIBRARY_ID lib);

/**
 * Retrieve the user-visible name for the given library.
 */
const std::string& library_name(LIBRARY_ID lib);

/**
 * Produce a plain-text report of library versions suitable for stdout/stderr.
 */
std::string library_versions_report();

} // end namespace game_config

#endif
