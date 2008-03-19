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

//! @file server/metrics.cpp
//! Various server-statistics.

#include "../global.hpp"

#include "metrics.hpp"

#include <algorithm>
#include <time.h>
#include <iostream>

bool operator<(const metrics::sample& s, const simple_wml::string_span& name)
{
	return s.name < name;
}

bool operator<(const simple_wml::string_span& name, const metrics::sample& s)
{
	return name < s.name;
}

struct compare_samples_by_time {
	bool operator()(const metrics::sample& a, const metrics::sample& b) const {
		return a.processing_time < b.processing_time;
	}
};


metrics::metrics() : most_consecutive_requests_(0),
                     current_requests_(0), nrequests_(0),
                     nrequests_waited_(0), started_at_(time(NULL))
{}

void metrics::service_request()
{
	if(current_requests_ > 0) {
		++nrequests_waited_;
	}

	++nrequests_;
	++current_requests_;
	if(current_requests_ > most_consecutive_requests_) {
		most_consecutive_requests_ = current_requests_;
	}
}

void metrics::no_requests()
{
	current_requests_ = 0;
}

void metrics::record_sample(const simple_wml::string_span& name,
                            clock_t parsing_time, clock_t processing_time)
{
	std::pair<std::vector<sample>::iterator,std::vector<sample>::iterator>
	    range = std::equal_range(samples_.begin(), samples_.end(), name);
	if(range.first == range.second) {
		//protect against DoS with memory exhaustion
		if(samples_.size() > 30) {
			return;
		}
		int index = range.first - samples_.begin();
		simple_wml::string_span dup_name(name.duplicate());
		sample new_sample;
		new_sample.name = dup_name;
		new_sample.nsamples = 0;
		new_sample.parsing_time = 0;
		new_sample.processing_time = 0;
		samples_.insert(range.first, new_sample);

		range.first = samples_.begin() + index;
	}

	range.first->nsamples++;
	range.first->parsing_time += parsing_time;
	range.first->processing_time += processing_time;
}

void metrics::game_terminated(const std::string& reason)
{
	terminations_[reason]++;
}

std::ostream& operator<<(std::ostream& out, metrics& met)
{
	const time_t time_up = time(NULL) - met.started_at_;
	const int seconds = time_up%60;
	const int minutes = (time_up/60)%60;
	const int hours = (time_up/(60*60))%24;
	const int days = time_up/(60*60*24);
	const int requests_immediate = met.nrequests_ - met.nrequests_waited_;
	const int percent_immediate = (requests_immediate*100)/(met.nrequests_ > 0 ? met.nrequests_ : 1);
	out << "METRICS\nUp " << days << " days, " << hours << " hours, "
	    << minutes << " minutes, " << seconds << " seconds\n"
	    << met.nrequests_ << " requests serviced. " << requests_immediate
	    << " (" << percent_immediate << "%) "
	    << " requests were serviced immediately\n"
	    << "longest burst of requests was " << met.most_consecutive_requests_ << "\n";

	if(met.terminations_.empty() == false) {
		out << "Games have been terminated in the following ways: \n";
		for(std::map<std::string,int>::const_iterator i = met.terminations_.begin(); i != met.terminations_.end(); ++i) {
			out << i->first << ": " << i->second << "\n";
		}
	}

	std::vector<metrics::sample> ordered_samples = met.samples_;
	std::sort(ordered_samples.begin(), ordered_samples.end(), compare_samples_by_time());

	out << "\n\nRequest types:\n";

	for(std::vector<metrics::sample>::const_iterator s = ordered_samples.begin(); s != ordered_samples.end(); ++s) {
		out << "'" << s->name << "' called " << s->nsamples << " times " << s->parsing_time << " parsing time, " << s->processing_time << " processing time\n";
	}

	return out;
}
