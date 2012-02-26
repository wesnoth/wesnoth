/* $Id$ */
/*
   Copyright (C) 2012 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "wesmage/options.hpp"

#include "foreach.hpp"
#include "wesmage/exit.hpp"
#include "wesmage/filter.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>

toptions::toptions()
	: input_filename()
	, output_filename()
{
}

/*
 * This function prints the option and its description in a nice fashion.
 *
 * * The description is indented at column tab_offset.
 * * If the option text is short enough the description starts after it,
 *   properly indented. Else starts at the next line, again properly
 *   indented.
 * * If the text of the description doesn't fit at a single line it split at
 *   space and continues on the next line, obviously indented again.
 */
static void
print_option(
		  std::ostream& stream
		, const std::string& option
		, std::string description)
{
	const unsigned line_length = 80;
	const unsigned tab_offset = 25;
	const unsigned description_length = line_length - tab_offset;
	const std::string tab_filler(tab_offset - 1, ' ');

	assert(!option.empty());
	assert(!description.empty());

	stream << option;
	if(option.length() < tab_offset - 1) {
		stream << std::string(tab_offset - 1 - option.length(), ' ');
	} else {
		stream << '\n' << tab_filler;
	}

	while(!description.empty()) {
		if(description.size() <= description_length) {
			stream << description;
			description.clear();
		} else {
			int offset = description_length + 1;
			while(description[offset] != ' ' && offset >= 0) {
				--offset;
			}
			assert(offset != 0);
			assert(description[offset] == ' ');

			stream << description.substr(0, offset);
			description.erase(0, offset + 1);
		}
		stream << '\n';
		if(!description.empty()) {
			stream << tab_filler;
		}
	}
}

static std::ostream&
operator<<(
		  std::ostream& stream
		, const tfilter_description& fd)
{
	print_option(stream, fd.name, fd.description);
	BOOST_FOREACH(const tfilter_description::tparameter& p, fd.parameters) {
		print_option(
				  stream
				, " * " + p.name + " (" + p.type + ")"
				, p.descripton);
	}

	return stream;
}

static void
print_help(const int exit_status)
{
	std::cout <<
"Usage wesmage [OPTION...] [FILE]\n"
"Helper programme to test image manipulation algorithms.\n"
"\n"
"The FILE is the name of the input file to be converted.\n"
"OPTIONS:\n"
"-o, --output FILE       The name of the output file to be written.\n"
"-f, --filter FILTER     Filters to be applied to the image. See FILTERS.\n"
"-h, --help              Show this help and terminate the programme.\n"
"\n"
"FILTERS:\n"
"A filter applies a modification to an image. The programme can handle\n"
"multiple filters. They are applied from the command line. The are applied\n"
"in the left to right order they appear on the command line.\n"
"A filter has the following syntax ID:PARAMETERS where:\n"
"ID                      The id of the filter.\n"
"PARAMETERS              Zero or more parameters. Multiple parameters are\n"
"                        separated by a comma. The number parameters required\n"
"                        depend on the filter.\n"
"\n"
"The following filters are currently implemented:\n"
;
	BOOST_FOREACH(const tfilter_description& filter, filter_list()) {
		std::cout << filter;
	}

	throw texit(exit_status);
}

#define VALIDATE_NOT_PAST_END                                                 \
	do {                                                                      \
		if(i >= argc) {                                                       \
			std::cerr << "Error: Required argument for the option »"          \
					<< option                                                 \
					<< "« is not supplied.\n";                                \
                                                                              \
			throw texit(EXIT_FAILURE);                                        \
		}                                                                     \
	} while(0)

const toptions&
toptions::parse(int argc, char* argv[])
{
	toptions& result = singleton(false);

	bool help = false;

	/* argv[0] is the name of the programme, not a command-line argument. */
	for(int i = 1; i < argc; ++i) {
		const std::string option(argv[i]);

		if(option == "-h" || option == "--help") {
			help = true;
		} else if(option == "-o" || option == "--output") {
			++i;
			VALIDATE_NOT_PAST_END;
			result.output_filename = argv[i];
		} else if(option.substr(0, 2) == "-o") {
			result.output_filename = option.substr(2);
		} else if(option == "-f" || option == "--filter") {
			++i;
			VALIDATE_NOT_PAST_END;
			result.filters.push_back(argv[i]);
		} else if(option.substr(0, 2) == "-f") {
			result.filters.push_back(option.substr(2));
		} else {
			if(!result.input_filename.empty()) {
				std::cerr << "Error: Command line argument »"
						<< option
						<< "« is not recognised.\n";

				print_help(EXIT_FAILURE);
			}
			result.input_filename = option;
		}
	}

	if(help) {
		print_help(EXIT_SUCCESS);
	}

	if(result.input_filename.empty()) {
		std::cerr << "Error: Input filename omitted.\n";
		print_help(EXIT_FAILURE);
	}

	if(result.output_filename.empty()) {
		std::cerr << "Error: Output filename omitted.\n";
		print_help(EXIT_FAILURE);
	}

	/*
	 * No filter implies a copy, or conversion to png, which is a valid
	 * way to use the programme, so do not complain.
	 */

	return result;
}

const toptions&
toptions::options()
{
	return singleton(true);
}

toptions&
toptions::singleton(const bool is_initialized)
{
	static bool initialized = false;
	assert(is_initialized == initialized);
	initialized = true;

	static toptions result;
	return result;
}
