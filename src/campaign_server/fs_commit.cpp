/*
   Copyright (C) 2016 - 2018 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "campaign_server/fs_commit.hpp"

#include "log.hpp"
#include "serialization/parser.hpp"

#include <cerrno>
#include <cstdio>
#include <cstring>

#ifndef _WIN32
#include <unistd.h>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#endif

static lg::log_domain log_filesystem("filesystem");

#define DBG_FS LOG_STREAM(debug, log_filesystem)
#define LOG_FS LOG_STREAM(info,  log_filesystem)
#define WRN_FS LOG_STREAM(warn,  log_filesystem)
#define ERR_FS LOG_STREAM(err,   log_filesystem)

namespace filesystem
{

namespace
{

inline void atomic_fail(const std::string& step_description)
{
	const std::string errno_desc = std::strerror(errno);
	ERR_FS << "Atomic commit failed (" << step_description << "): "
		   << errno_desc << '\n';
	throw filesystem::io_exception(std::string("Atomic commit failed (") + step_description + ")");
}

#ifndef _WIN32

/**
 * Returns the POSIX file descriptor associated with the stream.
 *
 * This only makes sense for valid streams returned by ostream_file(). Anything
 * else will yield 0.
 */
int get_stream_file_descriptor(std::ostream& os)
{
	// NOTE: This is insider knowledge of filesystem::ostream_file(), but it will
	//       do for 1.12 at least.
	typedef boost::iostreams::stream<boost::iostreams::file_descriptor_sink> fd_stream_type;
	fd_stream_type* const real = dynamic_cast<fd_stream_type*>(&os);
	return real ? (*real)->handle() : 0;
}

#endif // ! _WIN32

} // unnamed namespace

atomic_commit::atomic_commit(const std::string& filename)
	: temp_name_(filename + ".new")
	, dest_name_(filename)
	, out_(filesystem::ostream_file(temp_name_))
#ifndef _WIN32
	, outfd_(filesystem::get_stream_file_descriptor(*out_))
#endif
{
	LOG_FS << "Atomic write guard created for " << dest_name_ << " using " << temp_name_ << '\n';
}

atomic_commit::~atomic_commit()
{
	if(!temp_name_.empty()) {
		ERR_FS << "Temporary file for atomic write leaked: " << temp_name_ << '\n';
	}
}

void atomic_commit::commit()
{
	if(temp_name_.empty()) {
		ERR_FS << "Attempted to commit " << dest_name_ << " more than once!\n";
		return;
	}

#ifdef _WIN32
	// WARNING:
	// Obviously not atomic at all. Perhaps there's an alternate way to achieve
	// the same more securely using the Win32 API, but I don't think anyone
	// cares about running campaignd on this platform, let alone making it
	// resilient against environment errors. This is just here for reference.
	if(filesystem::file_exists(dest_name_) && std::remove(dest_name_.c_str()) != 0) {
		atomic_fail("remove");
	}
#else
	if(fsync(outfd_) != 0) {
		atomic_fail("fsync");
	}
#endif

	if(std::rename(temp_name_.c_str(), dest_name_.c_str()) != 0) {
		atomic_fail("rename");
	}

	LOG_FS << "Atomic commit succeeded: " << temp_name_ << " -> " << dest_name_ << '\n';

	temp_name_.clear();
}

} // namespace filesystem
