/*
	Copyright (C) 2012-2013 by Pierre Talbot <ptalbot@mopong.net>
	Part of the Battle for Wesnoth Project http://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#ifndef UMCD_OPTIONS_HPP
#define UMCD_OPTIONS_HPP

#include <string>
#include <fstream>

#include "config.hpp"
#include <boost/program_options.hpp>
#include <boost/optional.hpp>

class server_options
{
private:
	void build_options_desc();

public:
	/**
		@brief Accept argument as describe with the "--help" option. 
		@post Build a server_options object.
		@throw If the command line arguments are not recognized by the current pattern.
	*/
	server_options(int argc, char* argv[]);

	/**
	@return True if the command line arguments has only requested version/help information. False if the server should be started.
	*/
	bool is_info() const;
	
	/**
	@return True if the server must be launched as a daemon. False if it must be launched as a frontend task.
	*/
	bool is_daemon() const;

	/**
	@return The config file validated by the file returned by the function get_umcd_config_file_schema().
	*/
	config read_config() const;

	/**
	@return The Wesnoth directory if available in cfg.
	*/
	boost::optional<std::string> wesnoth_dir(const config& cfg) const;

	/**
	@throw if the config file is incomplete or wrong.
	@pre game_config::path initialized.
	*/
	void validate(const config& cfg) const;

private:
	std::string add_trailing_slash(const std::string& dir) const;

	std::string header_;
	std::string version_;
	boost::program_options::options_description options_desc_;
	boost::program_options::options_description config_file_options_;
	boost::program_options::variables_map vm_;
	std::string config_file_name_;
};

#endif //UMCD_OPTIONS_HPP