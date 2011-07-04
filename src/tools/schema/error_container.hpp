/* $Id$ */
/*
	Copyright (C) 2011 - 2011 by Sytyi Nick <nsytyi@gmail.com>
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
 * This file contains object "error_container", which are used to store error
 * messages while annotation parsing.
 * Also the another part of his job is to create GCC:styled error messages.
 */

#ifndef TOOLS_ERROR_CONTAINER_HPP_INCLUDED
#define TOOLS_ERROR_CONTAINER_HPP_INCLUDED

#include <algorithm>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>

namespace schema_generator{
/**
 * Container of errors, which are generated while schema generator tool
 * is parsing source files.
 * These errors are collected here, to be print on screen after parsing.
 */
class class_error_container{
public:
	class_error_container(){}

	/**
	 *Puts simple error message in container
	 * @param message Message to print.
	 */
	void add_simple_error(const std::string & message);

	/**
	 * Generate and put GCC-style error message about error read file error
	 * @param file Filename
	 * @param line Number of line with error
	 */
	void add_read_error(const std::string & file,int line);

	/**
	 * Generate and put GCC-style error message about annotation block
	 * somebody missed to close
	 * @param file Filename
	 * @param line Number of line with error
	 * @param name Name of open block
	 */
	void add_opened_entity_error(const std::string & file,int line,
								 const std::string & name);

	/**
	 * Generate and put GCC-style error message about closing block
	 * that wasn't opened
	 * @param file Filename
	 * @param line Number of line with error
	 * @param name of block
	 */
	void add_unopened_entity_error(const std::string & file,int line,
								   const std::string & name);

	/**
	 * Generate and put GCC-style error message about opening parent block
	 * before before closing previous block
	 * @param file Filename
	 * @param line Number with error
	 * @param first Name of open parent
	 * @param second Name of parent is opening
	 */
	void add_second_parent_error(
			const std::string & file,int line, const std::string & first,
			const std::string & second);

	/**
	 * Generate and put GCC-style error message about tag without parent
	 * @param file Filename
	 * @param line Number with error
	 * @param name Name of tag
	 */
	void add_orphan_error(const std::string & file,int line,
						  const std::string & name);

	/**
	 * Prints errors to output stream
	 * @param s Output
	 */
	void print_errors(std::ostream & s) const;
	void sort();

	/** Checks, if container is empty.*/
	bool is_empty() const {
		return list_.empty();
	}
private:
	/** Container to store error messages.*/
	std::vector<std::string> list_;
};


}//namespace schema_generator
#endif // TOOLS_ERROR_CONTAINER_HPP_INCLUDED
