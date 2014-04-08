/*
   Copyright (C) 2012 - 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

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
 * Filters for wesmage
 */

#ifndef WESMAGE_FILTER_HPP_INCLUDED
#define WESMAGE_FILTER_HPP_INCLUDED

#include "sdl_utils.hpp"

#include <string>
#include <vector>

void
filter_apply(surface& surf, const std::string& filter);

/**
 * Helper structure to describe what a filter does.
 *
 * This structure should make it easier to create help messages.
 */
struct tfilter_description
{
	/**
	 * Constructor.
	 *
	 * Creates an object from a specially formated string. The string
	 * contains of a number of fields seperated by a pipe-symbol. The number
	 * of fields should be 2 + 3 * params, where params is the number of
	 * parameters of the filter. The fields are:
	 * * 1 The @ref tfilter_description::name
	 * * 2 The @ref tfilter_description::description
	 * After these two fields there are three fields per parameter, the
	 * fields are:
	 * * 1 The @ref tfilter_description::tparameter::name
	 * * 2 The @ref tfilter_description::tparameter::type
	 * * 3 The @ref tfilter_description::tparameter::description
	 *
	 * @param fmt                 The format string as described above.
	 */
	explicit tfilter_description(const std::string& fmt);

	/**
	 * Name of the filter.
	 *
	 * This is the ID parameter given on the command line.
	 */
	std::string name;

	/**
	 * Description of the filter.
	 *
	 * Shortly describes what the filter does.
	 */
	std::string description;

	/** Describes a filter parameter. */
	struct tparameter
	{
		/** The name of the parameter. */
		std::string name;

		/** The C type of the parameter. */
		std::string type;

		/** Describes what the parameter does. */
		std::string descripton;
	};

	/** The list of filter parameters. */
	std::vector<tparameter> parameters;
};

/** Returns the list of available filters. */
std::vector<tfilter_description>
filter_list();

#endif
