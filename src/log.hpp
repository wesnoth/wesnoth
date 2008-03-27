/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
                 2004 - 2008 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file log.hpp
//!

#ifndef LOG_HPP_INCLUDED
#define LOG_HPP_INCLUDED

#include <iosfwd>
#include <string>

namespace lg {

class logger;

class log_domain {
	int domain_;
public:
	log_domain(char const *name);
	friend class logger;
};

bool set_log_domain_severity(std::string const &name, int severity);
std::string list_logdomains();

class logger {
	char const *name_;
	int severity_;
public:
	logger(char const *name, int severity): name_(name), severity_(severity) {}
	std::ostream &operator()(log_domain const &domain, bool show_names = true) const;
	bool dont_log(log_domain const &domain) const;
};

void timestamps(bool);
std::string get_timestamp(const time_t& t, const std::string& format="%Y%m%d %T ");

extern logger err, warn, info, debug;
extern log_domain general, ai, config, display, engine, network, mp_server,
	filesystem, audio, notifs, replay, help;

class scope_logger
{
	int ticks_;
	std::string str_;
	std::ostream &output_;
public:
	scope_logger(log_domain const &domain, std::string const &str);
	~scope_logger();
	void do_indent() const;
};

/**
 * Use this logger to send errors due to deprecated WML.
 * The preferred format is:
 * xxx is deprecated, support will be removed in version X. or
 * xxx is deprecated, support has been removed in version X.
 *
 * After every wml-event the errors are shown to the user,
 * so they can inform the campaign maintainer.
 */
extern std::stringstream wml_error;

} // namespace lg

#define log_scope(a) lg::scope_logger scope_logging_object__(lg::general, a);
#define log_scope2(a,b) lg::scope_logger scope_logging_object__(lg::a, b);

#define LOG_STREAM(a, b) if (lg::a.dont_log(lg::b)) ; else lg::a(lg::b)

#endif
