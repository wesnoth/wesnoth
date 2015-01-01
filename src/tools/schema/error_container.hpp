/*
	Copyright (C) 2011 - 2015 by Sytyi Nick <nsytyi@gmail.com>
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

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace schema_validation{
/**
 * Container of errors, which are generated while schema generator tool
 * is parsing source files.
 * These errors are collected here, to be print on screen after parsing.
 */
class class_error_container{
public:
	class_error_container()
		: list_(),types_(),links_()
	{
	}

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
	 * Generate and put GCC-style error message about wrong type value
	 * @param file  Filename
	 * @param line  Number with error
	 * @param name  Name of type
	 * @param value Name of value
	 */
	void wrong_type_error(const std::string & file,int line,
						  const std::string & name,const std::string & value);
	/**
	 * Generate and put GCC-style error message about unknown type to type cache
	 * @param file Filename
	 * @param line Number with error
	 * @param type Name of type
	 */
	void add_type_error(const std::string & file,int line,
						  const std::string & type);
	/**
	 * Clears type cache
	 * @param type Name of type
	 */
	void remove_type_errors(const std::string & type);
	/**
	 * Generate and put GCC-style error message about overriding type
	 * Overriding means that that type was defined somewhere else,
	 * and it's old value is lost.
	 * @param file Filename
	 * @param line Number of line with error
	 * @param type Name of type
	 */
	void overriding_type_error(const std::string & file,int line,
							   const std::string & type);

	/**
	 * Generate and put GCC-style error message about failed link to link cache
	 * @param file Filename
	 * @param line Number of line with error
	 * @param link Name of link
	 */
	void add_link_error(const std::string & file,int line,
						  const std::string & link);
	/**
	 * Clears link cache
	 * @param link Name of link
	 */
	void remove_link_errors(const std::string & link);

	/**
	 * Prints errors to output stream
	 * @param s Output
	 */
	void print_errors(std::ostream & s) const;

	/** Checks, if container is empty.*/
	bool is_empty() const {
		return list_.empty() && types_.empty() && links_.empty();
	}
private:
	/** Container to store error messages.*/
	std::vector<std::string> list_;
	/**
	 * Container to cache type errors.
	 * Types are checked while keys are read.
	 * There is a possibility to have unknown type error just because file with
	 * it desription haven't been parsed yet.
	 * So every type warning adds to cache.
	 * When sourseparser finds new type, it clears that warning list
	 */
	struct error_cache_element{
		std::string file;
		int line;
		std::string name;
		error_cache_element(const std::string &f,int l,const std::string &n):
		file(f),line(l),name(n){}
	};

	typedef std::map<std::string,
	std::vector<error_cache_element> > error_cache_map;
	error_cache_map types_;
	error_cache_map links_;
};


}//namespace schema_generator
#endif // TOOLS_ERROR_CONTAINER_HPP_INCLUDED
