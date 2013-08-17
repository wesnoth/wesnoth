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

#include "tools/code_generator/sql2cpp/sql2cpp_options.hpp"

#include <iostream>

namespace
{
	namespace po = boost::program_options;
}

sql2cpp_options::sql2cpp_options(int argc, char* argv[]) : 
		header_("  SQL schema to C++ classes (sql2cpp).\n  Development version by Pierre Talbot. Copyright (C) 2013.\n"), 
		version_("sql2cpp - Development version")
{
	build_options_desc();

	// Parse the command line.
	po::store(po::command_line_parser(argc, argv).options(options_desc_).run(), vm_);
	po::notify(vm_);

	// Print info.
	if(vm_.count("version"))
	{
		std::cout << version_ << std::endl;
	}
	if(vm_.count("help"))
	{
		std::cout << header_ << options_desc_ << std::endl;
	}
}

void sql2cpp_options::build_options_desc()
{
	// Help messages.
	std::string output_dir_help_msg("The output directory in which generated files will be created.");
	std::string schema_file_help_msg("The SQL schema file you want to generate C++ classes from. Be careful, only a small subset of the SQL is handled.");
	std::string header_help_msg("The header file we need to add in the top of each files.");

	// Generic options.
	po::options_description generic("General options");
	generic.add_options()
		("help", "produce help message")
		("version,v", "output the version number")
	;

	// Options related to the config file.
	po::options_description generation_options("Generation options");
	generation_options.add_options()
		("output_dir,d", po::value<std::string>(&output_dir_), output_dir_help_msg.c_str())
		("schema,s", po::value<std::string>(&schema_file_), schema_file_help_msg.c_str())
		("header,h", po::value<std::string>(&header_file_), header_help_msg.c_str())
	;

	options_desc_.add(generic).add(generation_options);
}

std::string sql2cpp_options::add_trailing_slash(const std::string& dir) const
{
	if(dir.size() > 0 && *dir.rbegin() != '/')
		return dir + '/';
	return dir;
}

bool sql2cpp_options::is_info() const
{
	return (vm_.count("version") || vm_.count("help")) && !vm_.count("file");
}

std::string sql2cpp_options::output_directory() const
{
	if(output_dir_.empty())
		throw po::validation_error(po::validation_error::at_least_one_value_required, "output_dir");
	return add_trailing_slash(output_dir_);
}

std::string sql2cpp_options::schema_file() const
{
	if(schema_file_.empty())
		throw po::validation_error(po::validation_error::at_least_one_value_required, "schema_file");
	return schema_file_;
}

std::string sql2cpp_options::header_file() const
{
	if(header_file_.empty())
		throw po::validation_error(po::validation_error::at_least_one_value_required, "header_file");
	return header_file_;
}
