/*
	Copyright (C) 2020 - 2024
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

#include "server/campaignd/options.hpp"

#include "commandline_argv.hpp"
#include "formatter.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

namespace po = boost::program_options;

namespace campaignd {

command_line::command_line(int argc, char** argv)
	: command_line(read_argv(argc, argv))
{
}

command_line::command_line(const std::vector<std::string>& args)
	: help(false)
	, version(false)
	, config_file()
	, server_dir()
	, port()
	, show_log_domains(false)
	, log_domain_levels()
	, log_precise_timestamps(false)
	, report_timings(false)
	, argv0_(args.at(0))
	, args_(args.begin() + 1, args.end())
	, help_text_()
{
	po::options_description opts_general{"General options"};
	opts_general.add_options()
		("help,h", "prints this message and exits.")
		("version,v", "displays the server version and exits.")
		;

	po::options_description opts_server{"Server configuration"};
	opts_server.add_options()
		("config,c", po::value<std::string>(), "specifies the path to the server configuration file. By default this is server.cfg in the server directory.")
		("server-dir,s",  po::value<std::string>(), "specifies the path to the server directory. By default this is the process working directory.")
		("port,p", po::value<unsigned short>(), "specifies the port number on which the server will listen for client connections.")
		;

	po::options_description opts_log{"Logging options"};
	opts_log.add_options()
		("logdomains", "lists defined log domains and exits")
		("log-error", po::value<std::string>(), "sets the severity level of the specified log domain(s) to 'error'. <arg> should be given as a comma-separated list of domains.")
		("log-warning", po::value<std::string>(), "sets the severity level of the specified log domain(s) to 'warning'. This is the default for all log domains other than 'campaignd' and 'server'.")
		("log-info", po::value<std::string>(), "sets the severity level of the specified log domain(s) to 'info'. This is the default for the 'campaignd' and 'server' log domains.")
		("log-debug", po::value<std::string>(), "sets the severity level of the specified log domain(s) to 'debug'.")
		("log-none", po::value<std::string>(), "disables logging for the specified log domain(s).")
		("log-precise", "shows the timestamps in log output with more precision.")
		("timings", "outputs timings for serviced requests to stderr.")
		;

	po::options_description opts;
	opts.add(opts_general).add(opts_server).add(opts_log);

	static const int style = po::command_line_style::default_style ^ po::command_line_style::allow_guessing;

	po::variables_map vm;
	po::store(po::command_line_parser(args_).options(opts).style(style).run(), vm);

	static const std::map<std::string, lg::severity> log_levels = {
		{ "error",   lg::err().get_severity() },
		{ "warning", lg::warn().get_severity() },
		{ "info",    lg::info().get_severity() },
		{ "debug",   lg::debug().get_severity() },
		{ "none",    lg::severity::LG_NONE }
	};

	if(vm.count("help")) {
		if(!help) {
			help_text_ = formatter() << "Usage: " << argv0_ << " [<options>]\n" << opts;
		}

		help = true;
	}
	if(vm.count("version")) {
		version = true;
	}

	if(vm.count("config")) {
		config_file = vm["config"].as<std::string>();
	}
	if(vm.count("server-dir")) {
		server_dir = vm["server-dir"].as<std::string>();
	}
	if(vm.count("port")) {
		port = vm["port"].as<unsigned short>();
	}

	if(vm.count("logdomains")) {
		show_log_domains = true;
	}
	for(const auto& lvl : log_levels) {
		const auto& swtch = std::string{"log-"} + lvl.first;
		const auto severity = lvl.second;
		if(vm.count(swtch)) {
			const auto& domains = utils::split(vm[swtch].as<std::string>());
			for(const auto& d : domains) {
				log_domain_levels[d] = severity;
			}
		}
	}
	if(vm.count("log-precise")) {
		log_precise_timestamps = true;
	}
	if(vm.count("timings")) {
		report_timings = true;
	}
}

} // end namespace campaignd
