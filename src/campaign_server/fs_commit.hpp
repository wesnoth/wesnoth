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

/**
 * @file
 * Atomic filesystem commit functions.
 */

#pragma once

#include "filesystem.hpp"

namespace filesystem
{

/**
 * Wrapper class that guarantees that file commit atomicity.
 *
 * It is possible for a signal or exception to cause a file write to be aborted
 * in the middle of the process, leaving potentially inconsistent contents
 * behind which may be read by the same or another application later and result
 * in errors or further data loss.
 *
 * This wrapper prevents this by providing callers with an interface to request
 * a write stream that will be actually associated to a temporary file. Once
 * the caller is done with the file, it should call the commit() method to
 * complete the process so that the temporary replaces the original in an
 * atomic step.
 *
 * The rationale for using an explicit commit() method instead of the class
 * destructor is that the destructor will also be invoked during exception
 * handling, which could still cause the destination file to end up in an
 * inconsistent state.
 *
 * Note that if the destructor is invoked before commit() is, the temporary
 * file will be left behind. This is deliberate so as to provide a way for
 * the user to look at the resulting file and optionally try to reconcile it
 * against the original.
 */
class atomic_commit
{
public:
	/**
	 * Constructor.
	 *
	 * @throws filesystem::io_exception if the operation fails in some way.
	 */
	atomic_commit(const std::string& filename);

	atomic_commit(const atomic_commit&) = delete;
	atomic_commit& operator=(const atomic_commit&) = delete;

	~atomic_commit();

	/**
	 * Returns the write stream associated with the file.
	 *
	 * Before commit() is invoked, this refers to the temporary file; afterwards,
	 * to the destination.
	 */
	scoped_ostream& ostream()
	{
		return out_;
	}

	/**
	 * Commits the new file contents to disk atomically.
	 *
	 * @throws filesystem::io_exception if the operation fails in some way.
	 */
	void commit();

private:
	std::string temp_name_;
	std::string dest_name_;
	scoped_ostream out_;
#ifndef _WIN32
	int outfd_;
#endif
};

}
