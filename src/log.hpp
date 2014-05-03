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
 * Standard logging facilities (interface).
 */

#ifndef LOG_HPP_INCLUDED
#define LOG_HPP_INCLUDED

#ifndef __func__
 #ifdef __FUNCTION__
  #define __func__ __FUNCTION__
 #endif
#endif

#include <iostream> // needed else all files including log.hpp need to do it.
#include <sstream> // as above. iostream (actually, iosfwd) declares stringstream as an incomplete type, but does not define it
#include <string>
#include <utility>

namespace lg {

/**
 * Helper class to redirect the output of the logger in a certain scope.
 *
 * The main usage of the redirection is for the unit tests to validate the
 * ourput on the logger with the expected output.
 */
class tredirect_output_setter
{
public:

	/**
	 * Constructor.
	 *
	 * @param stream              The stream to direct the output to.
	 */
	explicit tredirect_output_setter(std::ostream& stream);

	~tredirect_output_setter();

private:

	/**
	 * The previously set redirection.
	 *
	 * This value is stored here to be restored in this destructor.
	 */
	std::ostream* old_stream_;
};

class logger;

typedef std::pair<const std::string, int> logd;

class log_domain {
	logd *domain_;
public:
	log_domain(char const *name);
	friend class logger;
};

bool set_log_domain_severity(std::string const &name, int severity);
bool set_log_domain_severity(std::string const &name, const logger &lg);
std::string list_logdomains(const std::string& filter);

class logger {
	char const *name_;
	int severity_;
public:
	logger(char const *name, int severity): name_(name), severity_(severity) {}
	std::ostream &operator()(log_domain const &domain,
		bool show_names = true, bool do_indent = false) const;

	bool dont_log(log_domain const &domain) const
	{
		return severity_ > domain.domain_->second;
	}

	int get_severity() const
	{
		return severity_;
	}
};

void timestamps(bool);
void precise_timestamps(bool);
std::string get_timestamp(const time_t& t, const std::string& format="%Y%m%d %H:%M:%S ");
std::string get_timespan(const time_t& t);

extern logger err, warn, info, debug;
extern log_domain general;

class scope_logger
{
	int ticks_;
	std::ostream *output_;
	std::string str_;
public:
	scope_logger(log_domain const &domain, const char* str) :
		ticks_(0),
		output_(NULL),
		str_()
	{
		if (!debug.dont_log(domain)) do_log_entry(domain, str);
	}
	scope_logger(log_domain const &domain, const std::string& str) :
		ticks_(0),
		output_(NULL),
		str_()
	{
		if (!debug.dont_log(domain)) do_log_entry(domain, str);
	}
	~scope_logger()
	{
		if (output_) do_log_exit();
	}
	void do_indent() const;
private:
	void do_log_entry(log_domain const &domain, const std::string& str);
	void do_log_exit();
};

/**
 * Use this logger to send errors due to deprecated WML.
 * The preferred format is:
 * xxx is deprecated; support will be removed in version X. or
 * xxx is deprecated; support has been removed in version X.
 *
 * After every wml-event the errors are shown to the user,
 * so they can inform the campaign maintainer.
 */
extern std::stringstream wml_error;

} // namespace lg

#define log_scope(a) lg::scope_logger scope_logging_object__(lg::general, a);
#define log_scope2(a,b) lg::scope_logger scope_logging_object__(a, b);

#define LOG_STREAM(a, b) if (lg::a.dont_log(b)) ; else lg::a(b)

// When using log_scope/log_scope2 it is nice to have all output indented.
#define LOG_STREAM_INDENT(a,b) if (lg::a.dont_log(b)) ; else lg::a(b, true, true)

#endif
