/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
                 2004 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
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

class logger {
	char const *name_;
	int severity_;
public:
	logger(char const *name, int severity): name_(name), severity_(severity) {}
	std::ostream &operator()(log_domain const &domain, bool show_names = true) const;
	bool dont_log(log_domain const &domain) const;
};

extern logger err, warn, info;
extern log_domain general, ai, config, display, engine, network;

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

} // namespace lg

#define log_scope(a) lg::scope_logger scope_logging_object__(lg::general, a);
#define log_scope2(a,b) lg::scope_logger scope_logging_object__(lg::a, b);

#define LOG_HELPER(a, b) if (lg::a.dont_log(lg::b)) ; else lg::a(lg::b)

#endif
