/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
                 2004 - 2015 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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

#include "log.hpp"

#include <boost/date_time.hpp>

#include <map>
#include <sstream>
#include <ctime>

#include "utils/io.hpp"

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
static std::ostream *output_stream = nullptr;

static std::ostream& output()
{
	if(output_stream) {
		return *output_stream;
	}
	return std::cerr;
}

namespace lg {

redirect_output_setter::redirect_output_setter(std::ostream& stream)
	: old_stream_(output_stream)
{
	output_stream = &stream;
}

redirect_output_setter::~redirect_output_setter()
{
	output_stream = old_stream_;
}

typedef std::map<std::string, int> domain_map;
static domain_map *domains;
static int strict_level_ = -1;
void timestamps(bool t) { timestamp = t; }
void precise_timestamps(bool pt) { precise_timestamp = pt; }

logger& err()
{
	static logger lg("error", 0);
	return lg;
}

logger& warn()
{
	static logger lg("warning", 1);
	return lg;
}

logger& info()
{
	static logger lg("info", 2);
	return lg;
}

logger& debug()
{
	static logger lg("debug", 3);
	return lg;
}

static log_domain dom("general");

log_domain& general()
{
	return dom;
}

log_domain::log_domain(char const *name)
	: domain_(nullptr)
{
	// Indirection to prevent initialization depending on link order.
	if (!domains) domains = new domain_map;
	domain_ = &*domains->insert(logd(name, 1)).first;
}

bool set_log_domain_severity(const std::string& name, int severity)
{
	std::string::size_type s = name.size();
	if (name == "all") {
		for(logd &l : *domains) {
			l.second = severity;
		}
	} else if (s > 2 && name.compare(s - 2, 2, "/*") == 0) {
		for(logd &l : *domains) {
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
bool set_log_domain_severity(const std::string& name, const logger &lg) {
	return set_log_domain_severity(name, lg.get_severity());
}

bool get_log_domain_severity(const std::string& name, int &severity)
{
	domain_map::iterator it = domains->find(name);
	if (it == domains->end())
		return false;
	severity = it->second;
	return true;
}

std::string list_logdomains(const std::string& filter)
{
	std::ostringstream res;
	for(logd &l : *domains) {
		if(l.first.find(filter) != std::string::npos)
			res << l.first << "\n";
	}
	return res.str();
}

void set_strict_severity(int severity) {
	strict_level_ = severity;
}

void set_strict_severity(const logger &lg) {
	set_strict_severity(lg.get_severity());
}

static bool strict_threw_ = false;

bool broke_strict() {
	return strict_threw_;
}

std::string get_timestamp(const time_t& t, const std::string& format) {
	std::ostringstream ss;

	ss << utils::put_time(std::localtime(&t), format.c_str());

	return ss.str();
}
std::string get_timespan(const time_t& t) {
	std::ostringstream sout;
	// There doesn't seem to be any library function for this
	const time_t minutes = t / 60;
	const time_t days = minutes / 60 / 24;
	if(t <= 0) {
		sout << "expired";
	} else if(minutes == 0) {
		sout << t << " seconds";
	} else if(days == 0) {
		sout << minutes / 60 << " hours, " << minutes % 60 << " minutes";
	} else {
		sout << days << " days, " << (minutes / 60) % 24 << " hours, " << minutes % 60 << " minutes";
	}
	return sout.str();
}

static void print_precise_timestamp(std::ostream & out) throw()
{
	try{
		facet.put(
			std::ostreambuf_iterator<char>(out),
			out,
			' ',
			boost::posix_time::microsec_clock::local_time());
	} catch(...){}
}

std::ostream &logger::operator()(log_domain const &domain, bool show_names, bool do_indent) const
{
	if (severity_ > domain.domain_->second) {
		return null_ostream;
	} else {
		if (!strict_threw_ && (severity_ <= strict_level_)) {
			std::stringstream ss;
			ss << "Error (strict mode, strict_level = " << strict_level_ << "): wesnoth reported on channel " << name_ << " " << domain.domain_->first;
			std::cerr << ss.str() << std::endl;
			strict_threw_ = true;
		}
		std::ostream& stream = output();
		if(do_indent) {
			for(int i = 0; i != indent; ++i)
				stream << "  ";
			}
		if (timestamp) {
			if(precise_timestamp) {
				print_precise_timestamp(stream);
			} else {
				stream << get_timestamp(time(nullptr));
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
	output_ = &debug()(domain, false, true);
	str_ = str;
	ticks_ = boost::posix_time::microsec_clock::local_time();
	(*output_) << "{ BEGIN: " << str_ << "\n";
	++indent;
}

void scope_logger::do_log_exit()
{
	const long ticks = (boost::posix_time::microsec_clock::local_time() - ticks_).total_milliseconds();
	--indent;
	do_indent();
	if (timestamp) (*output_) << get_timestamp(time(nullptr));
	(*output_) << "} END: " << str_ << " (took " << ticks << "ms)\n";
}

void scope_logger::do_indent() const
{
	for(int i = 0; i != indent; ++i)
		(*output_) << "  ";
}

std::stringstream& wml_error()
{
	static std::stringstream lg;
	return lg;
}

} // end namespace lg

