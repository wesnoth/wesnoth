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

/**
 * @file
 * Various server-statistics.
 */

#include "server/metrics.hpp"

#include <algorithm>
#include <iostream>

struct compare_samples_to_stringspan {
	bool operator()(const simple_wml::string_span& a, const simple_wml::string_span& b) const
	{
		return a < b;
	}
};

struct compare_samples_by_time {
	bool operator()(const metrics::sample& a, const metrics::sample& b) const {
		return a.processing_time < b.processing_time;
	}
};


metrics::metrics() :
	samples_(),
	most_consecutive_requests_(0),
	current_requests_(0),
	nrequests_(0),
	nrequests_waited_(0),
	started_at_(time(nullptr)),
	terminations_()
{}

metrics::~metrics()
{

	for(std::vector<sample>::iterator itor = samples_.begin();
			itor != samples_.end(); ++itor)
	{
		delete[] itor->name.begin();
	}
	samples_.clear();
}

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
	std::vector<sample>::iterator isample = std::lower_bound(samples_.begin(), samples_.end(), name,compare_samples_to_stringspan());
	if(isample == samples_.end()
		|| isample->name != name) {
		//protect against DoS with memory exhaustion
		if(samples_.size() > 30) {
			return;
		}
		int index = isample - samples_.begin();
		simple_wml::string_span dup_name(name.duplicate());
		sample new_sample;
		new_sample.name = dup_name;
		samples_.insert(isample, new_sample);

		isample = samples_.begin() + index;
	}

	isample->nsamples++;
	isample->parsing_time += parsing_time;
	isample->processing_time += processing_time;
	isample->max_parsing_time = std::max(parsing_time,isample->max_parsing_time);
	isample->max_processing_time = std::max(processing_time,isample->max_processing_time);
}

void metrics::game_terminated(const std::string& reason)
{
	terminations_[reason]++;
}

std::ostream& metrics::games(std::ostream& out) const
{
	if (terminations_.empty()) return out << "No game ended so far.";

	size_t n = 0;
	out << "Games have been terminated in the following ways:\n";
	for(std::map<std::string,int>::const_iterator i = terminations_.begin(); i != terminations_.end(); ++i) {
		out << i->first << ": " << i->second << "\n";
		n += i->second;
	}
	out << "Total number of games = " << n;

	return out;
}

std::ostream& metrics::requests(std::ostream& out) const
{
	if (samples_.empty()) return out;

	std::vector<metrics::sample> ordered_samples = samples_;
	std::sort(ordered_samples.begin(), ordered_samples.end(), compare_samples_by_time());

	out << "\nSampled request types:\n";

	size_t n = 0;
	size_t pa = 0;
	size_t pr = 0;
	for(std::vector<metrics::sample>::const_iterator s = ordered_samples.begin(); s != ordered_samples.end(); ++s) {
		out << "'" << s->name << "' called " << s->nsamples << " times "
			<< s->parsing_time << "("<< s->max_parsing_time <<") parsing time, "
			<< s->processing_time << "("<<s->max_processing_time<<") processing time\n";
		n += s->nsamples;
		pa += s->parsing_time;
		pr += s->processing_time;
	}
	out << "Total number of request samples = " << n << "\n"
		<< "Total parsing time = " << pa << "\n"
		<< "Total processing time = " << pr;

	return out;
}

std::ostream& operator<<(std::ostream& out, metrics& met)
{
	const time_t time_up = time(nullptr) - met.started_at_;
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
	    << "requests were serviced immediately.\n"
	    << "longest burst of requests was: " << met.most_consecutive_requests_;

	return out;
}
