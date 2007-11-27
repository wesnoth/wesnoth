/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
                 2004 - 2007 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file log.cpp
//! Logger for debugging.
//! See also wesnoth-commandline-params --logdomains and --log-<level>="domain".

#include "global.hpp"

#include "SDL.h"

#include "log.hpp"

#include <algorithm>
#include <ctime>
#include <iostream>
#include <sstream>
#include <vector>

namespace {

struct logd {
	char const *name_;
	int severity_;
};

class null_streambuf : public std::streambuf
{
	virtual int overflow(int c) { return std::char_traits< char >::not_eof(c); }
public:
	null_streambuf() {}
};

} // end anonymous namespace

static std::vector< logd > log_domains;
static std::ostream null_ostream(new null_streambuf);
static int indent = 0;
static bool timestamp = false;

namespace lg {

void timestamps(bool t) { timestamp = t; }

logger err("error", 0), warn("warning", 1), info("info", 2), debug("debug", 3);
log_domain general("general"), ai("ai"), config("config"), display("display"),
	engine("engine"), network("network"), mp_server("server"),
	filesystem("filesystem"), audio("audio"), paths("paths"), notifs("notifs"),
	replay("replay");

log_domain::log_domain(char const *name) : domain_(log_domains.size())
{
	logd d = { name, 0 };
	log_domains.push_back(d);
}

bool set_log_domain_severity(std::string const &name, int severity)
{
	std::vector< logd >::iterator
		it = log_domains.begin(),
		it_end = log_domains.end();
	if (name == "all") {
		for(; it != it_end; ++it)
			it->severity_ = severity;
	} else {
		for(; it != it_end; ++it)
			if (name == it->name_) break;
		if (it == it_end)
			return false;
		it->severity_ = severity;
	}
	return true;
}

std::string list_logdomains()
{
	std::vector< logd >::iterator
		it_begin = log_domains.begin(),
		it_end = log_domains.end(),
		it;
	std::string domainlist = "";
	for(it = it_begin; it != it_end; ++it) {
		if (it != it_begin)
			domainlist += ", ";
		domainlist += it->name_;
	}
	return domainlist;
}

bool logger::dont_log(log_domain const &domain) const
{
	logd const &d = log_domains[domain.domain_];
	return severity_ > d.severity_;
}

std::ostream &logger::operator()(log_domain const &domain, bool show_names) const
{
	logd const &d = log_domains[domain.domain_];
	if (severity_ > d.severity_)
		return null_ostream;
	else {
		if (timestamp) {
			time_t t = time(NULL);
			char buf[100];
			strftime(buf, 100, "%b %d %H:%M:%S ", localtime(&t));
			std::cerr << buf;
		}
		if (show_names)
			std::cerr << name_ << ' ' << d.name_ << ": ";
		return std::cerr;
	}
}

scope_logger::scope_logger(log_domain const &domain, const std::string& str)
	: ticks_(SDL_GetTicks()), str_(str), output_(info(domain, false))
{
	do_indent();
	output_ << "BEGIN: " << str_ << "\n";
	++indent;
}

scope_logger::~scope_logger()
{
	const int ticks = SDL_GetTicks() - ticks_;
	--indent;
	do_indent();
	output_ << "END: " << str_ << " (took " << ticks << "ms)\n";
}

void scope_logger::do_indent() const
{
	for(int i = 0; i != indent; ++i)
		output_ << "  ";
}

std::stringstream wml_error;

} // end namespace lg

