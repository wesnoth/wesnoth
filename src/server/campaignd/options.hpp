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

/**
 * @file
 * campaignd command line options parsing.
 */

#pragma once

#include <optional>

#include <boost/program_options/options_description.hpp>
#include <map>
#include "log.hpp"

namespace campaignd {

class command_line
{
public:
	/**
	 * Reads the command line.
	 */
	command_line(int argc, char** argv);

	/**
	 * Retrieves the --help text.
	 *
	 * @note This text is only available when --help is used. Otherwise,
	 *       an empty string is returned instead.
	 */
	const std::string& help_text() const
	{
		return help_text_;
	}

	/** True if --help was passed. */
	bool help;
	/** True if --version was passed. */
	bool version;

	/** Path to the add-ons server configuration file. */
	std::optional<std::string> config_file;
	/** Path to the add-ons server storage dir. */
	std::optional<std::string> server_dir;
	/** Port number on which the server will listen for incoming connections. */
	std::optional<unsigned short> port;

	/** True if --logdomains was passed. */
	bool show_log_domains;
	/** Log domain/severity configuration. */
	std::map<std::string, lg::severity> log_domain_levels;
	/** Whether to use higher precision for log timestamps. */
	bool log_precise_timestamps;
	/** Whether to report timing information for server requests. */
	bool report_timings;

private:
	std::string argv0_;
	std::vector<std::string> args_;
	std::string help_text_;

	command_line(const std::vector<std::string>& args);

	void parse_log_domains(const std::string& domains_string, int severity);
};

} // end namespace campaignd
