/*
	Copyright (C) 2016 - 2024
	by Iris Morelle <shadowm2006@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "server/campaignd/fs_commit.hpp"

#include "log.hpp"
#include "serialization/parser.hpp"

#include <cerrno>
#include <cstdio>
#include <cstring>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#ifndef _WIN32

#include <unistd.h>

#else

#include "formatter.hpp"
#include "serialization/unicode_cast.hpp"

#include <boost/system/error_code.hpp>
#include <boost/filesystem.hpp>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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
namespace biostreams = boost::iostreams;

// These types correspond to what's used by filesystem::ostream_file() in
// filesystem.cpp.

using sink_type = biostreams::file_descriptor_sink;
using stream_type = biostreams::stream<sink_type>;
using platform_file_handle_type = sink_type::handle_type;

const platform_file_handle_type INVALID_FILE_HANDLE =
#ifndef _WIN32
	0
#else
	INVALID_HANDLE_VALUE
#endif
	;

inline void atomic_fail(const std::string& step_description)
{
	const std::string errno_desc = std::strerror(errno);
	ERR_FS << "Atomic commit failed (" << step_description << "): " << errno_desc;
	throw filesystem::io_exception(std::string("Atomic commit failed (") + step_description + ")");
}

/**
 * Returns the real file descriptor/handle associated with the stream.
 *
 * This only makes sense for valid streams returned by ostream_file(). Anything
 * else will yield an invalid value (e.g. 0 for POSIX, INVALID_HANDLE_VALUE for
 * Windows).
 */
platform_file_handle_type get_stream_file_descriptor(std::ostream& os)
{
	stream_type* const real = dynamic_cast<stream_type*>(&os);
	return real ? (*real)->handle() : INVALID_FILE_HANDLE;
}

#ifdef _WIN32

/**
 * Opens the specified file with FILE_SHARE_DELETE access.
 *
 * This is a drop-in replacement for filesystem::ostream_file. The special
 * access is required on Windows to rename or delete the file while we hold
 * handles to it.
 */
filesystem::scoped_ostream ostream_file_with_delete(const std::string& fname)
{
	LOG_FS << "streaming " << fname << " for writing with delete access.";

	namespace bfs = boost::filesystem;
	const auto& w_name = unicode_cast<std::wstring>(fname);

	try {
		HANDLE file = CreateFileW(w_name.c_str(),
								  GENERIC_WRITE | DELETE,
								  FILE_SHARE_WRITE | FILE_SHARE_DELETE,
								  nullptr,
								  CREATE_ALWAYS,
								  FILE_ATTRIBUTE_NORMAL,
								  nullptr);

		if(file == INVALID_HANDLE_VALUE) {
			throw BOOST_IOSTREAMS_FAILURE(formatter() << "CreateFile() failed: " << GetLastError());
		}

		// Transfer ownership to the sink post-haste
		sink_type fd{file, biostreams::close_handle};
		return std::make_unique<stream_type>(fd, 4096, 0);
	} catch(const BOOST_IOSTREAMS_FAILURE& e) {
		// Create directories if needed and try again
		boost::system::error_code ec_unused;
		if(bfs::create_directories(bfs::path{fname}.parent_path(), ec_unused)) {
			return ostream_file_with_delete(fname);
		}
		// Creating directories was impossible, give up
		throw filesystem::io_exception(e.what());
	}
}

/**
 * Renames an open file, potentially overwriting another (closed) existing file.
 *
 * @param new_name             New path for the open file.
 * @param open_handle          Handle for the open file.
 *
 * @return @a true on success, @a false on failure. Passing an invalid handle
 *         will always result in failure.
 */
bool rename_open_file(const std::string& new_name, HANDLE open_handle)
{
	if(open_handle == INVALID_HANDLE_VALUE) {
		ERR_FS << "replace_open_file(): Bad handle";
		return false;
	}

	const auto& w_name = unicode_cast<std::wstring>(new_name);
	const std::size_t buf_size = w_name.length()*sizeof(wchar_t) + sizeof(FILE_RENAME_INFO);

	// Avert your eyes, children

	std::unique_ptr<BYTE[]> fileinfo_buf{new BYTE[buf_size]};
	FILE_RENAME_INFO& fri = *reinterpret_cast<FILE_RENAME_INFO*>(fileinfo_buf.get());

	SecureZeroMemory(fileinfo_buf.get(), buf_size);
	fri.ReplaceIfExists = TRUE;
	fri.RootDirectory = nullptr;
	fri.FileNameLength = static_cast<DWORD>(w_name.length());
	::wmemcpy(fri.FileName, w_name.c_str(), w_name.length());

	// Okay, back to our regular programming

	if(!SetFileInformationByHandle(open_handle,
								   FileRenameInfo,
								   fileinfo_buf.get(),
								   static_cast<DWORD>(buf_size)))
	{
		ERR_FS << "replace_open_file(): SetFileInformationByHandle() " << GetLastError();
		return false;
	}

	return true;
}

#endif // !defined(_WIN32)

} // unnamed namespace

atomic_commit::atomic_commit(const std::string& filename)
	: temp_name_(filename + ".new")
	, dest_name_(filename)
#ifndef _WIN32
	, out_(filesystem::ostream_file(temp_name_))
	, outfd_(filesystem::get_stream_file_descriptor(*out_))
#else
	, out_(filesystem::ostream_file_with_delete(temp_name_))
	, handle_(filesystem::get_stream_file_descriptor(*out_))
#endif
{
	LOG_FS << "Atomic write guard created for " << dest_name_ << " using " << temp_name_;
}

atomic_commit::~atomic_commit()
{
	if(!temp_name_.empty()) {
		ERR_FS << "Temporary file for atomic write leaked: " << temp_name_;
	}
}

void atomic_commit::commit()
{
	if(temp_name_.empty()) {
		ERR_FS << "Attempted to commit " << dest_name_ << " more than once!";
		return;
	}

#ifdef _WIN32
	if(!rename_open_file(dest_name_, handle_)) {
		atomic_fail("rename");
	}
#else
	if(fsync(outfd_) != 0) {
		atomic_fail("fsync");
	}

	if(std::rename(temp_name_.c_str(), dest_name_.c_str()) != 0) {
		atomic_fail("rename");
	}
#endif

	LOG_FS << "Atomic commit succeeded: " << temp_name_ << " -> " << dest_name_;

	temp_name_.clear();
}

} // namespace filesystem
