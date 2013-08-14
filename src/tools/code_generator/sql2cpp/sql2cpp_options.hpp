/*
	Copyright (C) 2013 by Pierre Talbot <ptalbot@mopong.net>
	Part of the Battle for Wesnoth Project http://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#ifndef SQL2CPP_OPTIONS_HPP
#define SQL2CPP_OPTIONS_HPP

#include <string>
#include <boost/program_options.hpp>

class sql2cpp_options
{
private:
	void build_options_desc();

public:
	/**
		@brief Accept argument as describe with the "--help" option. 
		@post Build a sql2cpp_options object.
		@throw If the command line arguments are not recognized by the current pattern.
	*/
	sql2cpp_options(int argc, char* argv[]);

	/**
	@return True if the command line arguments has only requested version/help information. False if the program should be started.
	*/
	bool is_info() const;

	std::string output_directory() const;
	std::string schema_file() const;
	std::string header_file() const;

private:
	std::string add_trailing_slash(const std::string& dir) const;

	std::string header_;
	std::string version_;
	std::string output_dir_;
	std::string schema_file_;
	std::string header_file_;
	boost::program_options::options_description options_desc_;
	boost::program_options::variables_map vm_;
};

#endif //UMCD_OPTIONS_HPP