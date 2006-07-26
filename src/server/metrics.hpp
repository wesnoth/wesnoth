/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef METRICS_HPP_INCLUDED
#define METRICS_HPP_INCLUDED

#include <iosfwd>

#include <map>
#include <string>

class metrics
{
public:
	metrics();

	void service_request();
	void no_requests();

	void game_terminated(const std::string& reason);

	friend std::ostream& operator<<(std::ostream& out, metrics& met);

private:
	int most_consecutive_requests_;
	int current_requests_;
	int nrequests_;
	int nrequests_waited_;
	const time_t started_at_;
	std::map<std::string,int> terminations_;
};

std::ostream& operator<<(std::ostream& out, metrics& met);

#endif
