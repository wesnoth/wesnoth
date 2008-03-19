/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file server/metrics.hpp
//!

#ifndef METRICS_HPP_INCLUDED
#define METRICS_HPP_INCLUDED

#include <iosfwd>

#include <map>
#include <string>

#include "simple_wml.hpp"

class metrics
{
public:
	metrics();

	void service_request();
	void no_requests();

	void record_sample(const simple_wml::string_span& name,
	                   clock_t parsing_time, clock_t processing_time);

	void game_terminated(const std::string& reason);

	friend std::ostream& operator<<(std::ostream& out, metrics& met);

	struct sample {
		simple_wml::string_span name;
		int nsamples;
		clock_t parsing_time, processing_time;
	};

private:
	std::vector<sample> samples_;

	int most_consecutive_requests_;
	int current_requests_;
	int nrequests_;
	int nrequests_waited_;
	const time_t started_at_;
	std::map<std::string,int> terminations_;
};

std::ostream& operator<<(std::ostream& out, metrics& met);

#endif
