/*
	Copyright (C) 2020 - 2024
	by CrawlCycle <73139676+CrawlCycle@users.noreply.github.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/**
 * @file
 * Tool to check if two WML string are equivalent. Specifically, the @ref
 * check_wml_equivalence function would first preprocess and parse the two WML
 * string to yield two syntax trees (defined by config objects). Subsequently,
 * the function would assert if the two syntax trees are identical.
 */

#pragma once
#include "serialization/preprocessor.hpp"

/**
 * Make a syntax tree by preprocessing and parsing a WML string
 * @param[in]  wml_str    The string
 * @param[out] macro_map  Store preprocessor macros found by the preprocessor
 * @return                The syntax tree.
 * @warning Create and delete a temporary file at the current directory.
 */
config preprocess_and_parse(const std::string& wml_str, preproc_map* macro_map = nullptr);

/**
 * Assert two WML strings are equivalent. The function performs two steps:
 * 1. Make two syntax trees (defined by config objects) by preprocessing and
 *    parsing the two WML strings.
 * 2. Assert the two syntax tree are equal with a macro in Boost Test.
 * @warning Create and delete two temporary files at the current directory.
 */
void check_wml_equivalence(const std::string& a, const std::string& b);
