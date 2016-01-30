/*
   Copyright (C) 2012 - 2016 by Mark de Wever <koraq@xs4all.nl>
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
 * Command line parameters for wesmage.
 */

#ifndef WESMAGE_OPTIONS_HPP_INCLUDED
#define WESMAGE_OPTIONS_HPP_INCLUDED

#include <boost/noncopyable.hpp>

#include <string>
#include <vector>

/** A singleton class containing the parsed command line parameters. */
struct toptions
	: private boost::noncopyable
{
private:

	toptions();

public:

	/**
	 * Parses the command line.
	 *
	 * This function shall be called once at the beginning of the program.
	 *
	 * @param argc                The @p argc to @ref main.
	 * @param argv                The @p argv to @ref main.
	 *
	 * @returns                   The parsed options.
	 */
	static const toptions&
	parse(int argc, char* argv[]);

	/**
	 * Returns the cached parsed command line parameters.
	 *
	 * This function shall only be called after @ref toptions::parse has
	 * been called.
	 *
	 * @returns                   The parsed options.
	 */
	static const toptions&
	options();

	/** The filename of the input file. */
	std::string input_filename;

	/** The filename of the output file. */
	std::string output_filename;

	/** The filters to apply to the input file. */
	std::vector<std::string> filters;

	/** Display the time that applying the filters took. */
	bool time;

	/**
	 * The number of times the filter has to be applied.
	 *
	 * This feature is for performance testing only.
	 */
	int count;

private:

	/**
	 * Helper which contains the single instance of this class.
	 *
	 * @param is_initialized      Helper variable to track whether
	 *                            @ref toptions::parse is only called once
	 *                            and whether @ref toptions::options isn't
	 *                            called before @ref toptions::parse.
	 *
	 * @returns                   The single instance of this class.
	 */
	static toptions&
	singleton(const bool is_initialized);
};

#endif
