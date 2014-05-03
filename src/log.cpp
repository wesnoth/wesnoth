/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
                 2004 - 2014 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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
 * Standard logging facilities (implementation).
 * See also the command line switches --logdomains and --log-@<level@>="domain".
 */

#include "global.hpp"

#include "SDL.h"

#include "log.hpp"

#include <boost/foreach.hpp>
#include <boost/date_time.hpp>

#include <map>
#include <sstream>
#include <ctime>

namespace {

class null_streambuf : public std::streambuf
{
	virtual int overflow(int c) { return std::char_traits< char >::not_eof(c); }
public:
	null_streambuf() {}
};

} // end anonymous namespace

static std::ostream null_ostream(new null_streambuf);
static int indent = 0;
static bool timestamp = true;
static bool precise_timestamp = false;

static boost::posix_time::time_facet facet("%Y%m%d %H:%M:%S%F ");
static std::ostream *output_stream = NULL;

static std::ostream& output()
{
	if(output_stream) {
		return *output_stream;
	}
	return std::cerr;
}

namespace lg {

tredirect_output_setter::tredirect_output_setter(std::ostream& stream)
	: old_stream_(output_stream)
{
	output_stream = &stream;
}

tredirect_output_setter::~tredirect_output_setter()
{
	output_stream = old_stream_;
}

typedef std::map<std::string, int> domain_map;
static domain_map *domains;
void timestamps(bool t) { timestamp = t; }
void precise_timestamps(bool pt) { precise_timestamp = pt; }

logger err("error", 0), warn("warning", 1), info("info", 2), debug("debug", 3);
log_domain general("general");

log_domain::log_domain(char const *name)
	: domain_(NULL)
{
	// Indirection to prevent initialization depending on link order.
	if (!domains) domains = new domain_map;
	domain_ = &*domains->insert(logd(name, 1)).first;
}

bool set_log_domain_severity(std::string const &name, int severity)
{
	std::string::size_type s = name.size();
	if (name == "all") {
		BOOST_FOREACH(logd &l, *domains) {
			l.second = severity;
		}
	} else if (s > 2 && name.compare(s - 2, 2, "/*") == 0) {
		BOOST_FOREACH(logd &l, *domains) {
			if (l.first.compare(0, s - 1, name, 0, s - 1) == 0)
				l.second = severity;
		}
	} else {
		domain_map::iterator it = domains->find(name);
		if (it == domains->end())
			return false;
		it->second = severity;
	}
	return true;
}
bool set_log_domain_severity(std::string const &name, const logger &lg) {
	return set_log_domain_severity(name, lg.get_severity());
}

std::string list_logdomains(const std::string& filter)
{
	std::ostringstream res;
	BOOST_FOREACH(logd &l, *domains) {
		if(l.first.find(filter) != std::string::npos)
			res << l.first << "\n";
	}
	return res.str();
}

std::string get_timestamp(const time_t& t, const std::string& format) {
	char buf[100] = {0};
	tm* lt = localtime(&t);
	if (lt) {
		strftime(buf, 100, format.c_str(), lt);
	}
	return buf;
}
std::string get_timespan(const time_t& t) {
	char buf[100];
	// There doesn't seem to be any library function for this
	const time_t minutes = t / 60;
	const time_t days = minutes / 60 / 24;
	if(t <= 0) {
		strncpy(buf, "expired", 100);
	} else if(minutes == 0) {
		snprintf(buf, 100, "00:00:%02ld", t);
	} else if(days == 0) {
		snprintf(buf, 100, "%02ld:%02ld", minutes / 60, minutes % 60);
	} else {
		snprintf(buf, 100, "%ld %02ld:%02ld", days, (minutes / 60) % 24, minutes % 60);
	}
	return buf;
}

static void print_precise_timestamp(std::ostream & out)
{
	facet.put(
		std::ostreambuf_iterator<char>(out), 
		out, 
		' ', 
		boost::posix_time::microsec_clock::local_time());
}

std::ostream &logger::operator()(log_domain const &domain, bool show_names, bool do_indent) const
{
	if (severity_ > domain.domain_->second)
		return null_ostream;
	else {
		std::ostream& stream = output();
		if(do_indent) {
			for(int i = 0; i != indent; ++i)
				stream << "  ";
			}
		if (timestamp) {
			if(precise_timestamp) {
				print_precise_timestamp(stream);
			} else {
				stream << get_timestamp(time(NULL));
			}
		}
		if (show_names) {
			stream << name_ << ' ' << domain.domain_->first << ": ";
		}
		return stream;
	}
}

void scope_logger::do_log_entry(log_domain const &domain, const std::string& str)
{
	output_ = &debug(domain, false, true);
	str_ = str;
	ticks_ = SDL_GetTicks();
	(*output_) << "{ BEGIN: " << str_ << "\n";
	++indent;
}

void scope_logger::do_log_exit()
{
	const int ticks = SDL_GetTicks() - ticks_;
	--indent;
	do_indent();
	if (timestamp) (*output_) << get_timestamp(time(NULL));
	(*output_) << "} END: " << str_ << " (took " << ticks << "ms)\n";
}

void scope_logger::do_indent() const
{
	for(int i = 0; i != indent; ++i)
		(*output_) << "  ";
}

std::stringstream wml_error;

} // end namespace lg

