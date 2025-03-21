/*
	Copyright (C) 2015 - 2025
	by Iris Morelle <shadowm2006@gmail.com>
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

#include <string>
#include <vector>

namespace game_config
{

enum LIBRARY_ID
{
	LIB_BOOST,
	LIB_LUA,

	LIB_CRYPTO,
	LIB_CURL,

	LIB_CAIRO,
	LIB_PANGO,

	LIB_SDL,
	LIB_SDL_IMAGE,
	LIB_SDL_MIXER,

	LIB_COUNT
};

struct optional_feature
{
	std::string name;
	bool enabled;

	optional_feature(const char* n) : name(n), enabled(false) {}
};

/**
 * Obtain the processor architecture for this build.
 */
std::string build_arch();

/**
 * Retrieve the features table.
 */
std::vector<optional_feature> optional_features_table(bool localize = true);

/**
 * Produce a plain-text report of features suitable for stdout/stderr.
 */
std::string optional_features_report();

/**
 * Return the distribution channel identifier, or "Default" if missing.
 */
std::string dist_channel_id();

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

/**
 * Produce a bug report-style info dump.
 */
std::string full_build_report();

} // end namespace game_config
