/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "server/common/simple_wml.hpp"

#include <chrono>
#include <iosfwd>
#include <map>
#include <string>

class metrics
{
public:
	metrics();
	~metrics();

	void service_request();
	void no_requests();

	/** @todo: Currently unused. Use for something? */
	void record_sample(const simple_wml::string_span& name,
		const std::chrono::steady_clock::duration& parsing_time,
		const std::chrono::steady_clock::duration& processing_time);

	void game_terminated(const std::string& reason);

	std::ostream& games(std::ostream& out) const;
	std::ostream& requests(std::ostream& out) const;
	friend std::ostream& operator<<(std::ostream& out, metrics& met);

	struct sample
	{
		simple_wml::string_span name{};
		int nsamples = 0;
		std::chrono::steady_clock::duration parsing_time{0};
		std::chrono::steady_clock::duration processing_time{0};
		std::chrono::steady_clock::duration max_parsing_time{0};
		std::chrono::steady_clock::duration max_processing_time{0};

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
	const std::chrono::steady_clock::time_point started_at_;
	std::map<std::string, int> terminations_;
};

std::ostream& operator<<(std::ostream& out, metrics& met);
