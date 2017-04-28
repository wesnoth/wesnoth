/*
   Copyright (C) 2012 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
 * Contains code for tracing the code.
 */

#ifndef TRACER_HPP_INCLUDED
#define TRACER_HPP_INCLUDED

#include <map>
#include <string>

/** Helper structure for gathering the tracing statistics. */
struct tracer
{
	/**
	 * Helper structure to print the tracing statistics.
	 *
	 * When the constructor gets a valid @ref tracer pointer it prints the
	 * tracing statistics in its destructor. This allows the structure to be
	 * initialised at the beginning of a scope and print the statistics once
	 * the scope is left. (This makes it easier to write the tracing macros.)
	 */
	struct printer
	{
		printer(const printer&) = delete;
		printer& operator=(const printer&) = delete;

		explicit printer(const tracer* const tracer__);

		~printer();

		/** The tracer, whose statistics to print. */
		const tracer* const tracer_;
	};

	tracer(const tracer&) = delete;
	tracer& operator=(const tracer&) = delete;

	explicit tracer(const char* const function__);

	/** The total number of runs. */
	int run;

	/** The function being traced. */
	const char* const function;

	/**
	 * The tracer counters.
	 *
	 * The first pair contains a line number and a name of the marker.
	 * This has two advantages:
	 * * A line number is always unique, thus if using markers with the same
	 *   name, they are not the same marker.
	 * * The markers are sorted in order of appearance and not in order of
	 * their names.
	 *
	 * The second pair contains the number of times a marker is hit.
	 */
	std::map<std::pair<int, std::string>, int> counters;
};

/**
 * The beginning of a traced scope.
 *
 * It is not intended that tracer scopes are nested, but it should work at the
 * moment.
 *
 * @param interval                The interval between printing the statistics.
 */
#ifdef __GNUC__
#define TRACER_ENTRY(interval)                                               \
	static struct tracer tracer(__PRETTY_FUNCTION__);                              \
	tracer::printer print((++tracer.run % interval) == 0 ? &tracer : nullptr)
#else
#define TRACER_ENTRY(interval)                                               \
	static struct tracer tracer(__FUNCTION__);                                     \
	tracer::printer print((++tracer.run % interval) == 0 ? &tracer : nullptr)
#endif

/**
 * A trace count point.
 *
 * When this macro is reached the counter for this marker is increased.
 *
 * @param marker                  A string with the name of the marker.
 */
#define TRACER_COUNT(marker)                                                 \
	do {                                                                     \
		++tracer.counters[std::make_pair(__LINE__, marker)];                 \
	} while(0)

#endif
