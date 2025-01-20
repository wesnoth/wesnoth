/*
	Copyright (C) 2004 - 2024
	by Guillaume Melquiond <guillaume.melquiond@gmail.com>
	Copyright (C) 2003 by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
 * \#define ERR_DP LOG_STREAM(err, log_display)
 * \#define LOG_DP LOG_STREAM(info, log_display)
 *
 * Then stream logging info to ERR_DP, or LOG_DP, as if it were an ostream like std::cerr.
 * (In general it will actually be std::cerr at runtime when logging is enabled.)
 *
 * LOG_DP << "Found a window resize event: ...";
 *
 * Please do not use iomanip features like std::hex directly on the logger. Because of the
 * design of the logger, this will result in all of the loggers (in fact std::cerr) being
 * imbued with std::hex. Please use a formatter instead.
 *
 * \#include "formatter.hpp"
 *
 * LOG_DP << (formatter() << "The random seed is: '" << std::hex << seed << "'\n").str();
 *
 * It might be nice if somehow the logger class / macros could support using iomanip
 * things directly, but right now it doesn't, and it seems that it would complicate the
 * design greatly enough that it doesn't seem worth it.
 */

#pragma once

#ifndef __func__
 #ifdef __FUNCTION__
  #define __func__ __FUNCTION__
 #endif
#endif

#include <iosfwd> // needed else all files including log.hpp need to do it.
#include "utils/optional_fwd.hpp"
#include <string>
#include <utility>
#include <chrono>
#include <ctime>
#include <cstdint>

#include "formatter.hpp"

namespace lg {

// Prefix and extension for log files.
// This is used to find old files to delete.
const std::string log_file_prefix = "wesnoth-";
const std::string log_file_suffix = ".log";
// stdout file for Windows; needs to end in log_file_suffix so log rotation works for both
const std::string out_log_file_suffix = ".out" + log_file_suffix;

// Maximum number of older log files to keep intact. Other files are deleted.
// Note that this count does not include the current log file!
// double for Windows due to the separate .log and .out.log files
const unsigned max_logs = 8
#ifdef _WIN32
*2
#endif
;

enum class severity
{
    LG_NONE=-1,
	LG_ERROR=0,
	LG_WARN=1,
	LG_INFO=2,
	LG_DEBUG=3
};
std::ostringstream& operator<<(std::ostringstream& oss, lg::severity severity);

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

typedef std::pair<const std::string, severity> logd;

class log_domain {
	logd *domain_;
public:
	explicit log_domain(char const *name, severity severity = severity::LG_WARN);
	friend class logger;
};

bool set_log_domain_severity(const std::string& name, severity severity);
bool set_log_domain_severity(const std::string& name, const logger &lg);
bool get_log_domain_severity(const std::string& name, severity &severity);
std::string list_log_domains(const std::string& filter);

void set_strict_severity(severity severity);
void set_strict_severity(const logger &lg);
bool broke_strict();

/** toggle log sanitization */
void set_log_sanitize(bool sanitize);

/**
 * Do the initial redirection to a log file if the logs directory is writable.
 * Also performs log rotation to delete old logs.
 * NOTE: This runs before command line arguments are processed.
 *       Therefore the log file is initially written under the default userdata directory
 */
void set_log_to_file();
/**
 * Move the log file to another directory.
 * Used if a custom userdata directory is given as a command line option to move it to the new location.
 */
void move_log_file();
/**
 * Checks that a dummy file can be written to and deleted from the logs directory.
 */
void check_log_dir_writable();
/**
 * Returns the result set by check_log_dir_writable().
 * Will not be set if called before log redirection is done.
 *
 * @return true if the log directory is writable, false otherwise.
 */
utils::optional<bool> log_dir_writable();

/**
 * Use the defined prefix and suffix to determine if a filename is a log file.
 *
 * @return true if it's a log file, false otherwise
 */
bool is_not_log_file(const std::string& filename);
/**
 * Check how many log files exist and delete the oldest when there's too many.
 */
void rotate_logs(const std::string& log_dir);
/**
 * Generate a unique file name using the current timestamp and a randomly generated number.
 *
 * @return A unique file name to use for the current log file.
 */
std::string unique_log_filename();

// A little "magic" to surround the logging operation in a mutex.
// This works by capturing the output first to a stringstream formatter, then
// locking a mutex and dumping it to the stream all in one go.
// By doing this we can avoid rare deadlocks if a function whose output is streamed
// calls logging of its own.
// We overload operator| only because it has lower precedence than operator<<
// Any other lower-precedence operator would have worked just as well.
class log_in_progress {
	std::ostream& stream_;
	int indent_ = 0;
	bool timestamp_ = false;
	std::string prefix_;
	bool auto_newline_ = true;
public:
	log_in_progress(std::ostream& stream);
	void operator|(const formatter& message);
	void set_indent(int level);
	void enable_timestamp();
	void set_prefix(const std::string& prefix);
	void set_auto_newline(bool enabled);
};

class logger {
	char const *name_;
    severity severity_;
public:
	logger(char const *name, severity severity): name_(name), severity_(severity) {}
	log_in_progress operator()(const log_domain& domain,
		bool show_names = true, bool do_indent = false, bool show_timestamps = true, bool break_strict = true, bool auto_newline = true) const;

	bool dont_log(const log_domain& domain) const
	{
		return severity_ > domain.domain_->second;
	}

	severity get_severity() const
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
std::string sanitize_log(const std::string& logstr);
std::string get_log_file_path();

logger &err(), &warn(), &info(), &debug();
log_domain& general();

class scope_logger
{
	std::chrono::steady_clock::time_point start_;
	const log_domain& domain_;
	std::string str_;
public:
	scope_logger(const log_domain& domain, const char* str)
		: start_()
		, domain_(domain)
		, str_()
	{
		if (!debug().dont_log(domain)) do_log_entry(str);
	}
	scope_logger(const log_domain& domain, const std::string& str)
		: start_()
		, domain_(domain)
		, str_()
	{
		if (!debug().dont_log(domain)) do_log_entry(str);
	}
	~scope_logger()
	{
		if (!str_.empty()) do_log_exit();
	}
private:
	void do_log_entry(const std::string& str) noexcept;
	void do_log_exit() noexcept;
};

/**
 * Use this to show WML errors in the ingame chat.
 * After every WML event the errors are shown to the user so they can inform the campaign maintainer.
 */
std::stringstream& log_to_chat();

} // namespace lg

#define log_scope(description) lg::scope_logger scope_logging_object__(lg::general(), description);
#define log_scope2(domain,description) lg::scope_logger scope_logging_object__(domain, description);

#define LOG_STREAM(level, domain) if (lg::level().dont_log(domain)) ; else lg::level()(domain) | formatter()

// Don't prefix the logdomain to messages on this stream
#define LOG_STREAM_NAMELESS(level, domain) if (lg::level().dont_log(domain)) ; else lg::level()(domain, false) | formatter()

// Like LOG_STREAM_NAMELESS except doesn't add newlines automatically
#define LOG_STREAM_NAMELESS_STREAMING(level, domain) if (lg::level().dont_log(domain)) ; else lg::level()(domain, false, false, true, true, false) | formatter()

// When using log_scope/log_scope2 it is nice to have all output indented.
#define LOG_STREAM_INDENT(level,domain) if (lg::level().dont_log(domain)) ; else lg::level()(domain, true, true) | formatter()

// If you have an explicit logger object and want to ignore the logging level, use this.
// Meant for cases where you explicitly call dont_log to avoid an expensive operation if the logging is disabled.
#define FORCE_LOG_TO(logger, domain) logger(domain) | formatter()

// always log (since it's at the error level) to the general log stream
// outputting the log domain and timestamp is disabled
// meant as a replacement to using cerr/cout, but that goes through the same logging infrastructure as everything else
#define PLAIN_LOG lg::err()(lg::general(), false, false, false, false, true) | formatter()
#define STREAMING_LOG lg::err()(lg::general(), false, false, false, false, false) | formatter()
