/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#pragma once

#include <iosfwd>

#include <map>
#include <string>
#include <ctime>

#include "server/simple_wml.hpp"

class metrics
{
public:
	metrics();
	~metrics();

	void service_request();
	void no_requests();

	void record_sample(const simple_wml::string_span& name,
	                   clock_t parsing_time, clock_t processing_time);

	void game_terminated(const std::string& reason);

	std::ostream& games(std::ostream& out) const;
	std::ostream& requests(std::ostream& out) const;
	friend std::ostream& operator<<(std::ostream& out, metrics& met);

	struct sample {

		sample() :
			name(),
			nsamples(0),
			parsing_time(0),
			processing_time(0),
			max_parsing_time(0),
			max_processing_time(0)
		{
		}

		simple_wml::string_span name;
		int nsamples;
		clock_t parsing_time, processing_time;
		clock_t max_parsing_time, max_processing_time;

		operator const simple_wml::string_span&()
		{
			return name;
		}
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
