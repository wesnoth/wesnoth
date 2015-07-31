/*
   Copyright (C) 2012 - 2015 by Mark de Wever <koraq@xs4all.nl>
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

#include <boost/noncopyable.hpp>

#include <map>
#include <string>

/** Helper structure for gathering the tracing statistics. */
struct ttracer
	: private boost::noncopyable
{
	/**
	 * Helper structure to print the tracing statistics.
	 *
	 * When the constructor gets a valid @ref ttracer pointer it prints the
	 * tracing statistics in its destructor. This allows the structure to be
	 * initialised at the beginning of a scope and print the statistics once
	 * the scope is left. (This makes it easier to write the tracing macros.)
	 */
	struct tprint
		: private boost::noncopyable
	{
		explicit tprint(const ttracer* const tracer__);

		~tprint();

		/** The tracer, whose statistics to print. */
		const ttracer* const tracer;
	};

	explicit ttracer(const char* const function__);

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
	static ttracer tracer(__PRETTY_FUNCTION__);                              \
	ttracer::tprint print((++tracer.run % interval) == 0 ? &tracer : NULL)
#else
#define TRACER_ENTRY(interval)                                               \
	static ttracer tracer(__FUNCTION__);                                     \
	ttracer::tprint print((++tracer.run % interval) == 0 ? &tracer : NULL)
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
