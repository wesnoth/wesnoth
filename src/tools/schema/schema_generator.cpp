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
 * This file parses the input parameters, prepares a list of files to be parsed
 * and calls parser for each of them.
 *
 */

#include "tools/schema/sourceparser.hpp"

#include "filesystem.hpp"

#include <iostream>
#include <fstream>
#include <string>

#include <string.h>


std::string version = "0.6.0";

using namespace schema_validation;
/**
 * Parses the command line.
 * @retval 0 Everything's OK!
 * @retval 1 Errors found. User decided to exit.
 * @retval 2 No input files found. Please, check your input directory.
 * @retval 3 Output file for schema cannot be created.
 * @retval 4 Output file for regex list cannot be created.
 */
int main(int argc, char *argv[]){
	std::string input_dir = "";
	std::string output_file = "./data/gui/schema.cfg";
	std::string regex_file = "./utils/regex_list.txt";
	bool expand = false;
	for (int arg = 1; arg != argc; ++arg) {
		const std::string val(argv[arg]);
		if (val.empty()) {
			continue;
		}
		else if ((val == "--src-dir" || val == "-s") && arg+1 != argc) {
			input_dir = argv[++arg];
		}
		else if ((val == "--output" || val == "-o") && arg+1 != argc) {
			output_file = argv[++arg];
		}
		else if ((val == "--regex" || val == "-r") ) {
			if (arg+1 != argc){
				regex_file = argv[++arg];
			}
			std::fstream f;
			f.open(regex_file.c_str(),std::ios::out|std::ios::trunc);
			if (f.fail()){
				return 4;
			}
			test_regex(f);
			f.close();
			return 0;
		}
		else if (val == "--help" || val == "-h") {
			std::cout << "usage: " << argv[0]
					<< " [-erhV] [-s <input_dir>] [-o <output_file>]\n"
					<< " -r, --regex <regex_file>\t"
					<< "List of used regexes.\n"
					<< " -e, --expand \t"
					<< "Expands all tags due to their super-tags."
					<< "Useful to debug schema markup.\n"
					<< " -h, --help\t\t\t"
					<< "Shows this usage message.\n"
					<< " -s, --src-dir <input_dir>\t"
					<<"Select the input directory.\n"
					<< " -o, --output <output_file>\t"
					<<"Select the output file.\n"
					<< " -V, --version\t\t\t"
					<< "Version of tool\n";
			return 0;
		} else if (val == "--version" || val == "-V") {
			std::cout << "Battle for Wesnoth schema generator tool, version "
					<< version << "\n";
			return 0;
		} else if (val == "--expand" || val == "-e") {
			expand = true;
		}
	}
	if(input_dir.empty()){
		std::cout << "No input was selected. Processing \"./src\"\n";
		input_dir = "./src";
	}

	if (! filesystem::file_exists(input_dir)){
		return 2;
	}

	std::vector<std::string> files;
	std::vector<std::string> dirs;

	if (filesystem::is_directory(input_dir)){
		filesystem::get_files_in_dir(input_dir, &files, &dirs, filesystem::ENTIRE_FILE_PATH);

		if (files.empty() && dirs.empty()){
			std::cout << "Some problem with input directory "
					<< input_dir << "\n"
					<< "No files found\n";
			return 2;
		}
		/** Getting full list of files recursively.*/
		while (!dirs.empty()){
			std::string temp_dir = dirs.back();
			dirs.pop_back();
			filesystem::get_files_in_dir(temp_dir, &files, &dirs, filesystem::ENTIRE_FILE_PATH);
		}
	}else{
		files.push_back(input_dir);
	}
	class_source_parser parser;
	parser.set_output(output_file);
	std::vector<std::string>::iterator i = files.begin();
	unsigned int counter = 0;

	for (;i != files.end(); ++i){
		bool ok = false;
		if (filesystem::base_name((*i)).find(".cpp")!=std::string::npos){
			ok = true;
		} else
				if (filesystem::base_name((*i)).find(".hpp")!=std::string::npos){
			ok = true;
		} else
			if (filesystem::base_name((*i)).find(".schema")!=std::string::npos){
			ok = true;
		}
		if (ok){
			++counter;
			parser.set_input((*i));
			parser.parse_source();
		}
	}
	std::cout << "Processed " << counter << " files in " << input_dir << "\n";
	// check errors

	if (! parser.see_errors().is_empty()) {
		/**
	 * Let the user decide whether error are great or just misprints.
	 * If any error found, waits for a Yy or Nn to continue or not.
	 */
		std::cout << "There are some errors\n";
		parser.see_errors().print_errors(std::cout);
		// Take user response
		char c;
		std::cout << "Continue with errors? Y/N ";
		while (true) {
			std::cin.get(c);
			const char *r = strchr("yYnN",c);
			if (r == NULL){
				std::cout << "Please, choose your answer " << std::endl;
				continue;
			}
			if (c == 'n' || c=='N'){
				std::cout << "I'll take that as a NO" << std::endl;
				return 1;
			}
			if (c == 'y' || c=='Y'){
				std::cout << "I'll take that as a Yes" << std::endl;
				break;
			}
		}
	}
	if (expand) {
		parser.expand();
	}
	// save schema information
	if ( ! parser.save_schema()){
		return 4;
	}
	std::cout << "Schema written to "<< output_file << "\n";
	return 0;
}
