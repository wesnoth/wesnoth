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
 * Standard logging facilities (implementation).
 * See also the command line switches --logdomains and --log-@<level@>="domain".
 */

#include "log.hpp"
#include "filesystem.hpp"
#include "mt_rng.hpp"
#include "serialization/chrono.hpp"
#include "serialization/string_utils.hpp"
#include "utils/general.hpp"

#include <boost/algorithm/string.hpp>

#include <map>
#include <ctime>
#include <mutex>
#include <iostream>
#include <iomanip>

#ifdef _WIN32
#include <io.h>
#endif

static lg::log_domain log_setup("logsetup");
#define ERR_LS LOG_STREAM(err,   log_setup)
#define WRN_LS LOG_STREAM(warn,  log_setup)
#define LOG_LS LOG_STREAM(info,  log_setup)
#define DBG_LS LOG_STREAM(debug, log_setup)

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
static std::mutex log_mutex;

static bool log_sanitization = true;

/** whether the current logs directory is writable */
static utils::optional<bool> is_log_dir_writable_ = utils::nullopt;
/** alternative stream to write data to */
static std::ostream *output_stream_ = nullptr;

/**
 * @return std::cerr if the redirect_output_setter isn't being used, output_stream_ if it is
 */
static std::ostream& output()
{
	if(output_stream_) {
		return *output_stream_;
	}
	return std::cerr;
}

/** path to the current log file; does not include the extension */
static std::string output_file_path_ = "";
/** path to the current logs directory; may change after being initially set if a custom userdata directory is given on the command line */
static std::string logs_dir_ = "";

namespace lg {

std::ostringstream& operator<<(std::ostringstream& oss, const lg::severity severity)
{
    oss << static_cast<int>(severity);
    return oss;
}

bool is_not_log_file(const std::string& fn)
{
	return !(boost::algorithm::istarts_with(fn, lg::log_file_prefix) &&
			 boost::algorithm::iends_with(fn, lg::log_file_suffix));
}

void rotate_logs(const std::string& log_dir)
{
	// if logging to file is disabled, don't rotate the logs
	if(output_file_path_.empty()) {
		return;
	}

	std::vector<std::string> files;
	filesystem::get_files_in_dir(log_dir, &files);

	utils::erase_if(files, is_not_log_file);

	if(files.size() <= lg::max_logs) {
		return;
	}

	// Sorting the file list and deleting all but the last max_logs items
	// should hopefully be faster than stat'ing every single file for its
	// time attributes (which aren't very reliable to begin with).

	std::sort(files.begin(), files.end());

	for(std::size_t j = 0; j < files.size() - lg::max_logs; ++j) {
		const std::string path = log_dir + '/' + files[j];
		LOG_LS << "rotate_logs(): delete " << path;
		if(!filesystem::delete_file(path)) {
			ERR_LS << "rotate_logs(): failed to delete " << path << "!";
		}
	}
}

std::string unique_log_filename()
{
	std::ostringstream o;
	const std::time_t cur = std::time(nullptr);
	randomness::mt_rng rng;

	o << lg::log_file_prefix
	  << std::put_time(std::localtime(&cur), "%Y%m%d-%H%M%S-")
	  << rng.get_next_random();

	return o.str();
}

void check_log_dir_writable()
{
	std::string dummy_log = filesystem::get_logs_dir()+"/dummy.log";

	// log directory doesn't exist and can't be created
	if(!filesystem::file_exists(filesystem::get_logs_dir()) && !filesystem::make_directory(filesystem::get_logs_dir())) {
		is_log_dir_writable_ = false;
		return;
	}

	// can't create and write new log files
	try {
		filesystem::write_file(dummy_log, " ");
	} catch(const filesystem::io_exception&) {
		is_log_dir_writable_ = false;
		return;
	}

	// confirm that file exists and was written to
	if(filesystem::file_size(dummy_log) != 1) {
		is_log_dir_writable_ = false;
	}

	// can't delete files - prevents log rotation
	if(filesystem::file_exists(dummy_log) && !filesystem::delete_file(dummy_log)) {
		is_log_dir_writable_ = false;
		return;
	}

	is_log_dir_writable_ = true;
}

void move_log_file()
{
	if(logs_dir_ == filesystem::get_logs_dir() || logs_dir_ == "") {
		return;
	}

	check_log_dir_writable();

	if(is_log_dir_writable_.value_or(false)) {
#ifdef _WIN32
		std::string old_path = output_file_path_;
		output_file_path_ = filesystem::get_logs_dir()+"/"+unique_log_filename();

		// flush and close existing log files, since Windows doesn't allow moving open files
		std::fflush(stderr);
		std::cerr.flush();
		if(!std::freopen("NUL", "a", stderr)) {
			std::cerr << "Failed to close stderr log file: '" << old_path << "'";
			// stderr is where basically all output goes through, so if that fails then don't attempt anything else
			// moving just the stdout log would be pointless
			return;
		}
		std::fflush(stdout);
		std::cout.flush();
		if(!std::freopen("NUL", "a", stdout)) {
			std::cerr << "Failed to close stdout log file: '" << old_path << "'";
		}

		// move the .log and .out.log files
		// stdout and stderr are set to NUL currently so nowhere to send info on failure
		if(rename((old_path+lg::log_file_suffix).c_str(), (output_file_path_+lg::log_file_suffix).c_str()) == -1) {
			return;
		}
		rename((old_path+lg::out_log_file_suffix).c_str(), (output_file_path_+lg::out_log_file_suffix).c_str());

		// reopen to log files at new location
		// stdout and stderr are still NUL if freopen fails, so again nowhere to send info on failure
		std::fflush(stderr);
		std::cerr.flush();
		std::freopen((output_file_path_+lg::log_file_suffix).c_str(), "a", stderr);

		std::fflush(stdout);
		std::cout.flush();
		std::freopen((output_file_path_+lg::out_log_file_suffix).c_str(), "a", stdout);
#else
		std::string old_path = get_log_file_path();
		output_file_path_ = filesystem::get_logs_dir()+"/"+unique_log_filename();

		// non-Windows can just move the file
		if(rename(old_path.c_str(), get_log_file_path().c_str()) == -1) {
			std::cerr << "Failed to rename log file from '" << old_path << "' to '" << output_file_path_ << "'";
		}
#endif
	}
}

void set_log_to_file()
{
	check_log_dir_writable();
	// if the log directory is not writable, then don't try to do anything.
	// if the log directory is writable, then setup logging and rotate the logs.
	// if the optional isn't set, then logging to file has been disabled, so don't try to do anything
	if(is_log_dir_writable_.value_or(false)) {
		// get the log file stream and assign cerr+cout to it
		logs_dir_ = filesystem::get_logs_dir();
		output_file_path_ = filesystem::get_logs_dir()+"/"+unique_log_filename();

		// IMPORTANT: apparently redirecting stderr/stdout will also redirect std::cerr/std::cout, but the reverse is not true
		//            redirecting std::cerr/std::cout will *not* redirect stderr/stdout

		// redirect stderr to file
		std::fflush(stderr);
		std::cerr.flush();
		if(!std::freopen((output_file_path_+lg::log_file_suffix).c_str(), "w", stderr)) {
			std::cerr << "Failed to redirect stderr to a file!";
		}

		// redirect stdout to file
		// separate handling for Windows since dup2() just... doesn't work for GUI apps there apparently
		// redirect to a separate file on Windows as well, since otherwise two streams independently writing to the same file can cause weirdness
#ifdef _WIN32
		std::fflush(stdout);
		std::cout.flush();
		if(!std::freopen((output_file_path_+lg::out_log_file_suffix).c_str(), "w", stdout)) {
			std::cerr << "Failed to redirect stdout to a file!";
		}
#else
		if(dup2(STDERR_FILENO, STDOUT_FILENO) == -1) {
			std::cerr << "Failed to redirect stdout to a file!";
		}
#endif

		// make stdout unbuffered - otherwise some output might be lost
		// in practice shouldn't make much difference either way, given how little output goes through stdout/std::cout
		if(setvbuf(stdout, nullptr, _IONBF, 2) == -1) {
			std::cerr << "Failed to set stdout to be unbuffered";
		}

		rotate_logs(filesystem::get_logs_dir());
	}
}

utils::optional<bool> log_dir_writable()
{
	return is_log_dir_writable_;
}

std::string get_log_file_path()
{
	return output_file_path_.empty() ? "" : output_file_path_+lg::log_file_suffix;
}

redirect_output_setter::redirect_output_setter(std::ostream& stream)
	: old_stream_(output_stream_)
{
	output_stream_ = &stream;
}

redirect_output_setter::~redirect_output_setter()
{
	output_stream_ = old_stream_;
}

typedef std::map<std::string, severity> domain_map;
static domain_map *domains;
static severity strict_level_ = severity::LG_NONE;
void timestamps(bool t) { timestamp = t; }
void precise_timestamps(bool pt) { precise_timestamp = pt; }

logger& err()
{
	static logger lg("error", severity::LG_ERROR);
	return lg;
}

logger& warn()
{
	static logger lg("warning", severity::LG_WARN);
	return lg;
}

logger& info()
{
	static logger lg("info", severity::LG_INFO);
	return lg;
}

logger& debug()
{
	static logger lg("debug", severity::LG_DEBUG);
	return lg;
}

static log_domain dom("general");

log_domain& general()
{
	return dom;
}

log_domain::log_domain(char const *name, severity severity)
	: domain_(nullptr)
{
	// Indirection to prevent initialization depending on link order.
	if (!domains) domains = new domain_map;
	domain_ = &*domains->insert(logd(name, severity)).first;
	domain_->second = severity;
}

bool set_log_domain_severity(const std::string& name, severity severity)
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

bool get_log_domain_severity(const std::string& name, severity &severity)
{
	domain_map::iterator it = domains->find(name);
	if (it == domains->end())
		return false;
	severity = it->second;
	return true;
}

std::string list_log_domains(const std::string& filter)
{
	std::ostringstream res;
	for(logd &l : *domains) {
		if(l.first.find(filter) != std::string::npos)
			res << l.first << "\n";
	}
	return res.str();
}

void set_strict_severity(severity severity) {
	strict_level_ = severity;
}

void set_strict_severity(const logger &lg) {
	set_strict_severity(lg.get_severity());
}

static bool strict_threw_ = false;

bool broke_strict() {
	return strict_threw_;
}

void set_log_sanitize(bool sanitize) {
	log_sanitization = sanitize;
}

std::string sanitize_log(const std::string& logstr)
{
	if(!log_sanitization) {
		return logstr;
	}

	std::string str = logstr;

#ifdef _WIN32
	const char* user_name = getenv("USERNAME");
#else
	const char* user_name = getenv("USER");
#endif

	if(user_name != nullptr) {
		boost::replace_all(str, std::string("/") + user_name + "/", "/USER/");
		boost::replace_all(str, std::string("\\") + user_name + "\\", "\\USER\\");
	}

	return str;
}

log_in_progress logger::operator() (
	const log_domain& domain,
	bool show_names,
	bool do_indent,
	bool show_timestamps,
	bool break_strict,
	bool auto_newline) const
{
	if (severity_ > domain.domain_->second) {
		return null_ostream;
	} else {
		log_in_progress stream = output();
		if(do_indent) {
			stream.set_indent(indent);
		}
		if (timestamp && show_timestamps) {
			stream.enable_timestamp();
		}
		if (show_names) {
			stream.set_prefix(formatter() << name_ << ' ' << domain.domain_->first << ": ");
		}
		if (!strict_threw_ && severity_ <= strict_level_ && break_strict) {
			stream | formatter() << "Error (strict mode, strict_level = " << strict_level_ << "): wesnoth reported on channel " << name_ << " " << domain.domain_->first << std::endl;
			strict_threw_ = true;
		}
		stream.set_auto_newline(auto_newline);
		return stream;
	}
}

log_in_progress::log_in_progress(std::ostream& stream)
	: stream_(stream)
{}

void log_in_progress::operator|(const formatter& message)
{
	std::scoped_lock lock(log_mutex);
	for(int i = 0; i < indent; ++i)
		stream_ << "  ";
	if(timestamp_) {
		auto now = std::chrono::system_clock::now();
		stream_ << chrono::format_local_timestamp(now); // Truncates precision to seconds
		if(precise_timestamp) {
			auto as_seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
			auto fractional = std::chrono::duration_cast<std::chrono::microseconds>(now - as_seconds);
			stream_ << "." << std::setw(6) << fractional.count();
		}
		stream_ << " ";
	}
	stream_ << prefix_ << sanitize_log(message.str());
	if(auto_newline_) {
		stream_ << std::endl;
	}
}

void log_in_progress::set_indent(int level) {
	indent_ = level;
}

void log_in_progress::enable_timestamp() {
	timestamp_ = true;
}

void log_in_progress::set_prefix(const std::string& prefix) {
	prefix_ = prefix;
}

void log_in_progress::set_auto_newline(bool auto_newline) {
	auto_newline_ = auto_newline;
}

void scope_logger::do_log_entry(const std::string& str) noexcept
{
	str_ = str;
	start_ = std::chrono::steady_clock::now();
	debug()(domain_, false, true) | formatter() << "{ BEGIN: " << str_;
	++indent;
}

void scope_logger::do_log_exit() noexcept
{
	--indent;
	auto output = debug()(domain_, false, true);
	output.set_indent(indent);
	if(timestamp) output.enable_timestamp();
	auto now = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - start_);
	output | formatter() << "} END: " << str_ << " (took " << elapsed.count() << "us)"; // FIXME c++20 stream: operator
}

std::stringstream& log_to_chat()
{
	static std::stringstream lg;
	return lg;
}

} // end namespace lg
