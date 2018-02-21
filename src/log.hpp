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
 * Standard logging facilities (interface).
 *
 * To use one of the standard log channels, put something like the following at the start
 * of your .cpp file:
 *
 * static lg::log_domain log_display("display");
 * #define ERR_DP LOG_STREAM(err, log_display)
 * #define LOG_DP LOG_STREAM(info, log_display)
 *
 * Then stream logging info to ERR_DP, or LOG_DP, as if it were an ostream like std::cerr.
 * (In general it will actually be std::cerr at runtime when logging is enabled.)
 *
 * LOG_DP << "Found a window resize event: ...\n";
 *
 * Please do not use iomanip features like std::hex directly on the logger. Because of the
 * design of the logger, this will result in all of the loggers (in fact std::cerr) being
 * imbued with std::hex. Please use a formatter instead.
 *
 * #include "formatter.hpp"
 *
 * LOG_DP << (formatter() << "The random seed is: '" << std::hex << seed << "'\n").str();
 *
 * It might be nice if somehow the logger class / macros could support using iomanip
 * things directly, but right now it doesn't, and it seems that it would complicate the
 * design greatly enough that it doesn't seem worth it.
 */

#pragma once

#include "global.hpp"

#ifndef __func__
 #ifdef __FUNCTION__
  #define __func__ __FUNCTION__
 #endif
#endif

#include <iostream> // needed else all files including log.hpp need to do it.
#include <sstream> // as above. iostream (actually, iosfwd) declares stringstream as an incomplete type, but does not define it
#include <string>
#include <utility>

#include <boost/date_time/posix_time/posix_time_types.hpp>

using boost::posix_time::ptime;

namespace lg {

/**
 * Helper class to redirect the output of the logger in a certain scope.
 *
 * The main usage of the redirection is for the unit tests to validate the
 * output on the logger with the expected output.
 */
class redirect_output_setter
{
public:

	/**
	 * Constructor.
	 *
	 * @param stream              The stream to direct the output to.
	 */
	explicit redirect_output_setter(std::ostream& stream);

	~redirect_output_setter();

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
	log_domain(char const *name, int severity = 1);
	friend class logger;
};

bool set_log_domain_severity(const std::string& name, int severity);
bool set_log_domain_severity(const std::string& name, const logger &lg);
bool get_log_domain_severity(const std::string& name, int &severity);
std::string list_logdomains(const std::string& filter);

void set_strict_severity(int severity);
void set_strict_severity(const logger &lg);
bool broke_strict();

class logger {
	char const *name_;
	int severity_;
public:
	logger(char const *name, int severity): name_(name), severity_(severity) {}
	std::ostream &operator()(const log_domain& domain,
		bool show_names = true, bool do_indent = false) const;

	bool dont_log(const log_domain& domain) const
	{
		return severity_ > domain.domain_->second;
	}

	int get_severity() const
	{
		return severity_;
	}

	std::string get_name() const
	{
		return name_;
	}
};

void timestamps(bool);
void precise_timestamps(bool);
std::string get_timestamp(const time_t& t, const std::string& format="%Y%m%d %H:%M:%S ");
std::string get_timespan(const time_t& t);

logger &err(), &warn(), &info(), &debug();
log_domain& general();

class scope_logger
{
	ptime ticks_;
	std::ostream *output_;
	std::string str_;
public:
	scope_logger(const log_domain& domain, const char* str) :
		output_(nullptr)
	{
		if (!debug().dont_log(domain)) do_log_entry(domain, str);
	}
	scope_logger(const log_domain& domain, const std::string& str) :
		output_(nullptr)
	{
		if (!debug().dont_log(domain)) do_log_entry(domain, str);
	}
	~scope_logger()
	{
		if (output_) do_log_exit();
	}
	void do_indent() const;
private:
	void do_log_entry(const log_domain& domain, const std::string& str) NOEXCEPT;
	void do_log_exit() NOEXCEPT;
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
std::stringstream& wml_error();

} // namespace lg

#define log_scope(description) lg::scope_logger scope_logging_object__(lg::general(), description);
#define log_scope2(domain,description) lg::scope_logger scope_logging_object__(domain, description);

#define LOG_STREAM(level, domain) if (lg::level().dont_log(domain)) ; else lg::level()(domain)

// When using log_scope/log_scope2 it is nice to have all output indented.
#define LOG_STREAM_INDENT(level,domain) if (lg::level().dont_log(domain)) ; else lg::level()(domain, true, true)
