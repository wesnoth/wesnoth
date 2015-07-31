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

#include "serialization/schema_validator.hpp"

std::string version = "0.2.0";

#include <iostream>

#include "filesystem.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "config.hpp"
#include "config_cache.hpp"

using namespace schema_validation;
int main(int argc, char *argv[]){
	std::string default_schema ("data/gui/schema.cfg");
	std::string input ;
	for (int arg = 1; arg != argc; ++arg) {
		const std::string val(argv[arg]);
		if (val.empty()) {
			continue;
		}
		else if ((val == "--schema" || val == "-s") && arg+1 != argc) {
			default_schema = argv[++arg];
		}
		else if ((val == "--input" || val == "-i") && arg+1 != argc) {
			input = argv[++arg];
		}
		else if (val == "--help" || val == "-h") {
			std::cout << "usage: " << argv[0]
					<< " [-hV] [-i <input_file>] [-s <schema_file>]\n"
					<< " -h, --help\t\t\t"
					<< "Shows this usage message.\n"
					<< " -s, --schema <schema_file>\t"
					<<"Select the file with schema information.\n"
					<< " -i, --input <input_file>\t"
					<<"Select the config file.\n"
					<< " -V, --version\t\t\t"
					<< "Version of tool\n";
			return 0;
		} else if (val == "--version" || val == "-V") {
			std::cout << "Battle for Wesnoth schema validator tool, version "
					<< version << "\n";
			return 0;
		}
	}
	schema_validator validator (default_schema);
	if (input.empty()) input = "./data/gui/default.cfg";
	std::cout << "Processing "<< input <<"\n";
	config cfg;
	try {
		preproc_map preproc(
				game_config::config_cache::instance().get_preproc_map());
		filesystem::scoped_istream stream = preprocess_file(input,
												&preproc);
		read(cfg, *stream, &validator);
	} catch(config::error & t) {
		std::cout << t.message;
		return 1;
	}
	return 0;
}
