/*
	Copyright (C) 2017 - 2024
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

#include <cstdint>
#include <string>

/** See https://wiki.wesnoth.org/CompatibilityStandards for more info. */
enum class DEP_LEVEL : uint8_t { INDEFINITE = 1, PREEMPTIVE, FOR_REMOVAL, REMOVED };

/**
 * Prints a message to the deprecation log domain informing players that a given feature
 * has been deprecated.
 *
 * @param elem_name    The name of the feature to be deprecated.
 * @param level        The deprecation level. This indicates how long the feature will
 *                     remain supported before removal.
 * @param version      If @a level is PREEMPTIVE or FOR_REMOVAL, this should be the first
 *                     version in which the feature could be removed. If it's INDEFINITE
 *                     or REMOVED, this is unused.
 * @param detail       Optional extra message elaborating on the deprecation. This can be
 *                     used to specify which feature to use instead, for example.
 *
 * @returns            The final translated deprecation message in case you want to output
 *                     it elsewhere as well.
 *
 * @todo               @a version should probably be made optional to handle INDEFINITE
 *                     and REMOVED deprecation, but I don't think we can do that without
 *                     including version_info.hpp in this header.
 */
std::string deprecated_message(const std::string& elem_name,
		DEP_LEVEL level,
		const class version_info& version,
		const std::string& detail = "");
