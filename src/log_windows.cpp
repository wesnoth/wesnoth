/*
   Copyright (C) 2015 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "log_windows.hpp"

#include "filesystem.hpp"
#include "libc_error.hpp"
#include "log.hpp"
#include "serialization/unicode.hpp"

#include <cstdio>
#include <ctime>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501 // XP and later

#include <windows.h>

#if 1
static lg::log_domain log_setup("logsetup");
#define ERR_LS LOG_STREAM(err,   log_setup)
#define WRN_LS LOG_STREAM(warn,  log_setup)
#define LOG_LS LOG_STREAM(info,  log_setup)
#define DBG_LS LOG_STREAM(debug, log_setup)

#else

std::stringstream foo;
#define ERR_LS foo
#define WRN_LS foo
#define LOG_LS foo
#define DBG_LS foo

#endif

namespace lg
{

namespace
{

// Prefix and extension for log files. This is used both to generate the unique
// log file name during startup and to find old files to delete.
const std::string log_file_prefix = "wesnoth-";
const std::string log_file_suffix = ".log";

// Maximum number of older log files to keep intact. Other files are deleted.
// Note that this count does not include the current log file!
const unsigned max_logs = 8;

/** Helper function for rotate_logs. */
bool is_not_log_file(const std::string& fn)
{
	return !(boost::algorithm::istarts_with(fn, log_file_prefix) &&
			 boost::algorithm::iends_with(fn, log_file_suffix));
}

/**
 * Deletes old log files from the log directory.
 */
void rotate_logs(const std::string& log_dir)
{
	std::vector<std::string> files;
	filesystem::get_files_in_dir(log_dir, &files);

	files.erase(std::remove_if(files.begin(), files.end(), is_not_log_file), files.end());

	if(files.size() <= max_logs) {
		return;
	}

	// Sorting the file list and deleting all but the last max_logs items
	// should hopefully be faster than stat'ing every single file for its
	// time attributes (which aren't very reliable to begin with.

	std::sort(files.begin(), files.end());

	for(size_t j = 0; j < files.size() - max_logs; ++j) {
		const std::string path = log_dir + '/' + files[j];
		LOG_LS << "rotate_logs(): delete " << path << '\n';
		if(!filesystem::delete_file(path)) {
			WRN_LS << "rotate_logs(): failed to delete " << path << "!\n";
		}
	}
}

/**
 * Generates a "unique" log file name.
 *
 * This is really not guaranteed to be unique, but it's close enough, since
 * the odds of having multiple Wesnoth instances spawn with the same PID within
 * a second span are close to zero.
 *
 * The file name includes a timestamp in order to satisfy the requirements of
 * the rotate_logs logic.
 */
std::string unique_log_filename()
{
	std::ostringstream o;

	o << log_file_prefix;

	const time_t cur = time(NULL);
	const tm* const lt = localtime(&cur);

	if(lt) {
		char ts_buf[128] = { 0 };
		strftime(ts_buf, 128, "%Y%m%d-%H%M%S-", lt);
		o << ts_buf;
	}

	o << GetCurrentProcessId() << log_file_suffix;

	return o.str();
}

/**
 * Returns the path to a system-defined temporary files dir.
 */
std::string temp_dir()
{
	wchar_t tmpdir[MAX_PATH + 1];

	if(GetTempPath(MAX_PATH + 1, tmpdir) == 0) {
		return ".";
	}

	return unicode_cast<std::string>(std::wstring(tmpdir));
}

/**
 * Display an alert box to warn about log initialization errors, and exit.
 */
void log_init_panic(const std::string& msg)
{
	ERR_LS << "Log initialization panic call: " << msg << '\n';

	const std::string full_msg = msg + "\n\n" + "This may indicate an issue with your Wesnoth launch configuration. If the problem persists, contact the development team for technical support, including the full contents of this message (copy with CTRL+C).";

	// It may not be useful to write to stderr at this point, so warn the user
	// in a failsafe fashion via Windows UI API.
	MessageBox(NULL,
			   unicode_cast<std::wstring>(full_msg).c_str(),
			   L"Battle for Wesnoth",
			   MB_ICONEXCLAMATION | MB_OK);

	// It may seem excessive to quit over something like this, but it's a good
	// indicator of possible configuration issues with the user data dir that
	// may cause much weirder symptoms later (see http://r.wesnoth.org/t42970
	// for an example).
	exit(1);
}

/**
 * Display an alert box to warn about log initialization errors, and exit.
 */
void log_init_panic(const libc_error& e,
					const std::string& new_log_path,
					const std::string& old_log_path = std::string())
{
	std::ostringstream msg;

	if(old_log_path.empty()) {
		msg << "Early log initialization failed.";
	} else {
		msg << "Log relocation failed.";
	}

	msg << "\n\n"
		<< "Runtime error: " << e.desc() << " (" << e.num() << ")\n"
		<< "New log file path: " << new_log_path << '\n'
		<< "Old log file path: ";

	if(old_log_path.empty()) {
		msg << "Log file path: " << new_log_path << '\n';
	} else {
		msg << "New log file path: " << new_log_path << '\n'
			<< "Old log file path: ";
	}

	log_init_panic(msg.str());
}

/**
 * Singleton class that deals with the intricacies of log file redirection.
 */
class log_file_manager : private boost::noncopyable
{
public:
	log_file_manager();
	~log_file_manager();

	/**
	 * Returns the path to the current log file.
	 */
	std::string log_file_path() const;

	/**
	 * Moves the log file to a new directory.
	 *
	 * This causes the associated streams to closed momentarily in order to be
	 * able to move the log file, because Windows does not allow move/rename
	 * operations on currently-open files.
	 *
	 * @param log_dir        Log directory path.
	 *
	 * @throw libc_error     If the log file cannot be opened or relocated.
	 */
	void move_log_file(const std::string& log_dir);

private:
	std::string fn_;
	std::string cur_path_;

	enum STREAM_ID {
		STREAM_STDOUT = 1,
		STREAM_STDERR = 2
	};

	/**
	 * Opens the log file for the current session in the specified directory.
	 *
	 * @param file_path      Log file path.
	 * @param truncate       Whether to truncate an existing log file or append
	 *                       to it instead.
	 *
	 * @throw libc_error     If the log file cannot be opened.
	 */
	void open_log_file(const std::string& file_path,
					   bool truncate);

	/**
	 * Takes care of any tasks required for redirecting a log stream.
	 *
	 * @param file_path      Log file path.
	 * @param stream         Stream identifier.
	 * @param truncate       Whether to truncate an existing log file or append
	 *                       to it instead.
	 *
	 * @throw libc_error     If the log file cannot be opened.
	 *
	 * @note This does not set cur_path_ to the new path.
	 */
	void do_redirect_single_stream(const std::string& file_path,
								   STREAM_ID stream,
								   bool truncate);
};

log_file_manager::log_file_manager()
	: fn_(unique_log_filename())
	, cur_path_()
{
	DBG_LS << "Early init message\n";

	//
	// We use the Windows temp dir on startup,
	//
	const std::string new_path = temp_dir() + "/" + fn_;

	try {
		open_log_file(new_path, true);
	} catch(const libc_error& e) {
		log_init_panic(e, new_path, cur_path_);
	}

	LOG_LS << "Opened log file at " << new_path << '\n';
}

log_file_manager::~log_file_manager()
{
	if(cur_path_.empty()) {
		// No log file, nothing to do.
		return;
	}

	DBG_LS << "Closing log file...\n";
	fclose(stdout);
	fclose(stderr);
}

std::string log_file_manager::log_file_path() const
{
	return cur_path_;
}

void log_file_manager::move_log_file(const std::string& log_dir)
{
	const std::string new_path = log_dir + "/" + fn_;

	try {
		if(!cur_path_.empty()) {
			const std::string old_path = cur_path_;

			// Need to close files before moving or renaming. This will replace
			// cur_path_ with NUL, hence the backup above.
			open_log_file("NUL", false);

			if(rename(old_path.c_str(), new_path.c_str()) != 0) {
				throw libc_error();
			}
		}

		// Reopen.
		open_log_file(new_path, false);
	} catch(const libc_error& e) {
		log_init_panic(e, new_path, cur_path_);
	}

	LOG_LS << "Moved log file to " << new_path << '\n';
}

void log_file_manager::open_log_file(const std::string& file_path, bool truncate)
{
	do_redirect_single_stream(file_path, STREAM_STDERR, truncate);
	do_redirect_single_stream(file_path, STREAM_STDOUT, false);

	cur_path_ = file_path;
}

void log_file_manager::do_redirect_single_stream(const std::string& file_path,
												 log_file_manager::STREAM_ID stream,
												 bool truncate)
{
	DBG_LS << stream << ' ' << cur_path_ << " -> " << file_path << " [side A]\n";

	FILE* crts = stream == STREAM_STDERR ? stderr : stdout;
	std::ostream& cxxs = stream == STREAM_STDERR ? std::cerr : std::cout;

	fflush(crts);
	cxxs.flush();

	if(!freopen(file_path.c_str(), (truncate ? "w" : "a"), crts))
	{
		throw libc_error();
	}

	//setbuf(crts, NULL);

	DBG_LS << stream << ' ' << cur_path_ << " -> " << file_path << " [side B]\n";
}

boost::scoped_ptr<log_file_manager> lfm;

} // end anonymous namespace

std::string log_file_path()
{
	if(lfm) {
		return lfm->log_file_path();
	}

	return "";
}

void early_log_file_setup()
{
	if(lfm) {
		return;
	}

	lfm.reset(new log_file_manager());
}

void finish_log_file_setup()
{
	// Make sure the LFM is actually set up just in case.
	early_log_file_setup();

	static bool setup_complete = false;

	if(setup_complete) {
		ERR_LS << "finish_log_file_setup() called more than once!\n";
		return;
	}

	const std::string log_dir = filesystem::get_user_data_dir() + "/logs";
	if(!filesystem::file_exists(log_dir) && !filesystem::make_directory(log_dir)) {
		log_init_panic(std::string("Could not create logs directory at ") +
					   log_dir + ".");
	} else {
		rotate_logs(log_dir);
	}

	lfm->move_log_file(log_dir);

	setup_complete = true;
}

} // end namespace lg
