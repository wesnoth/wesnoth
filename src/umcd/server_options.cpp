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

#include "umcd/server_options.hpp"
#include <boost/program_options/errors.hpp>
#include "filesystem.hpp"
#include "serialization/schema_validator.hpp"

namespace
{
	namespace po = boost::program_options;
}

server_options::server_options(int argc, char* argv[]) : 
		header_("  Wesnoth User Made Content Daemon (UMCD).\n  Development version by Pierre Talbot. Copyright (C) 2013.\n"), 
		version_("Wesnoth User Made Content Daemon (UMCD) - Development version")
{
	build_options_desc();

	// Positional options (we don't need the option "cfg-file" to specify the config file).
	po::positional_options_description p;
	p.add("file,f", -1);

	// Parse the command line.
	po::store(po::command_line_parser(argc, argv).options(options_desc_).positional(p).run(), vm_);
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

void server_options::build_options_desc()
{
	// Help messages.
	std::string cfg_help_msg("The config file in which we read the server configuration.");
	std::string daemon_msg("Launch the server as a daemon task.");

	// Generic options.
	po::options_description generic("General options");
	generic.add_options()
		("help", "produce help message")
		("version,v", "output the version number")
	;

	// Options related to the config file.
	po::options_description file_options("Config file options");
	file_options.add_options()
		("file,f", po::value<std::string>(&config_file_name_), cfg_help_msg.c_str())
	;

	// Options related to the command line.
	po::options_description cmdline("Server configuration"); 
	cmdline.add_options()
		("daemon,d", daemon_msg.c_str())
	;

	options_desc_.add(generic).add(file_options).add(cmdline);
	config_file_options_.add(cmdline); 
}

bool server_options::is_info() const
{
	return (vm_.count("version") || vm_.count("help")) && !vm_.count("file");
}

bool server_options::is_daemon() const
{
	return vm_.count("daemon");
}

boost::optional<std::string> server_options::wesnoth_dir(const config& cfg) const
{
	boost::optional<std::string> wesdir;
	if(cfg.has_child("server_core") && cfg.child("server_core").has_attribute("wesnoth_dir"))
	{
		wesdir = cfg.child("server_core")["wesnoth_dir"];
	}
	return wesdir;
}

void server_options::validate(const config& cfg) const
{
	boost::optional<std::string> wesdir = wesnoth_dir(cfg);
	if(wesdir)
	{
		std::string validation_filename = *wesdir + get_umcd_config_file_schema();
		config dummy;
		schema_validation::schema_validator validator(validation_filename);
		::read(dummy, cfg.to_string(), &validator);
	}
	else
	{
		throw po::validation_error(po::validation_error::at_least_one_value_required, "wesnoth_dir");
	}
}

config server_options::read_config() const
{
	config cfg;
	if(vm_.count("file"))
	{
		std::ifstream cfgfile(config_file_name_.c_str());
		if(!cfgfile)
			throw po::reading_file(config_file_name_.c_str());
		::read(cfg, cfgfile);
	}
	return cfg;
}
