/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include <iostream>
#include <map>
#include <string>

#include "config.hpp"

int main(int argc, char** argv)
{
	if(argc != 3) {
		std::cerr << "Usage: " << argv[0]
		          << " translation default-translation\n";
		return 0;
	}

	const std::string& data1 = read_file(argv[1]);
	const std::string& data2 = read_file(argv[2]);

	if(data1.empty()) {
		std::cerr << "Could not read '" << argv[1] << "'\n";
		return 0;
	}

	if(data2.empty()) {
		std::cerr << "Could not read '" << argv[2] << "'\n";
		return 0;
	}

	config cfg;

	try {
		cfg.read(data1);
	} catch(config::error& e) {
		std::cerr << "error parsing '" << argv[1] << "': " << e.message << "\n";
		return 0;
	}

	std::vector<config*> translations = cfg.children["language"];
	if(translations.empty()) {
		std::cerr << "no translation data found in '" << argv[1] << "'\n";
		return 0;
	}

	if(translations.size() > 1) {
		std::cerr << "warning: found multiple translations in '" << argv[1]
		          << "'\n";
	}

	const std::map<std::string,std::string> strings = translations[0]->values;

	try {
		cfg.read(data2);
	} catch(config::error& e) {
		std::cerr << "error parsing '" << argv[2] << "': " << e.message << "\n";
		return 0;
	}

	translations = cfg.children["language"];
	if(translations.empty()) {
		std::cerr << "no translation data found in '" << argv[2] << "'\n";
		return 0;
	}

	if(translations.size() > 1) {
		std::cerr << "warning: found multiple translations in '" << argv[2]
		          << "'\n";
	}

	const std::map<std::string,std::string> default_strings =
	                                             translations[0]->values;

	std::cout << "[language]\n\n"
	          << "#strings that were not found in the translation, \n"
	          << "#and which should be translated now:\n";

	for(std::map<std::string,std::string>::const_iterator i =
	    default_strings.begin(); i != default_strings.end(); ++i) {
		if(strings.find(i->first) == strings.end()) {
			std::cout << i->first << "=\"" << i->second << "\"\n";
		}
	}

	std::cout << "\n#---------------------------------"
	          << "\n#strings that have already been translated\n";

	for(std::map<std::string,std::string>::const_iterator i = strings.begin();
	    i != strings.end(); ++i) {
		std::cout << i->first << "=\"" << i->second << "\"\n";
	}

	std::cout << "[/language]\n";
	return 0;
}
