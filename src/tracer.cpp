/*
   Copyright (C) 2012 - 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "tracer.hpp"

#include <boost/foreach.hpp>

#include <iomanip>
#include <iostream>

ttracer::tprint::tprint(const ttracer* const tracer__)
	: tracer(tracer__)
{
}

ttracer::tprint::~tprint()
{
	if(!tracer) {
		return;
	}

	std::cerr << "Run statistics for " << tracer->function << ":\n"
			<< "Runs:\t" << std::dec << tracer->run << "\n";

	typedef std::pair<std::pair<int, std::string>, int> thack;
	size_t maximum_length = 0;
	BOOST_FOREACH(const thack& counter, tracer->counters) {
		maximum_length = std::max(
				  maximum_length
				, counter.first.second.length());
	}

	std::ios_base::fmtflags original_flag = std::cerr.setf(
			  std::ios_base::left
			, std::ios_base::adjustfield);

	BOOST_FOREACH(const thack& counter, tracer->counters) {
		std::cerr << "Marker: "
				<< std::left
				<< std::setw(maximum_length) << counter.first.second
				<< std::right
				<< " [" << std::setw(5) << counter.first.first << ']'
				<< "    hits " << counter.second << "\n";
	}

	std::cerr.setf(original_flag, std::ios_base::adjustfield);
}

ttracer::ttracer(const char* const function__)
	: run(0)
	, function(function__)
	, counters()
{
}
