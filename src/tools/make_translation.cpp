/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "config.hpp"

using namespace std;

void process_config(const std::string& element_name, const config& cfg,
                    std::map<std::string,std::string>& out)
{
	typedef std::pair<string,string> pair;

	for(config::child_map::const_iterator i = cfg.all_children.begin(); i != cfg.all_children().end(); ++i) {
		for(vector<config*>::const_iterator j = i->second.begin();
		    j != i->second.end(); ++j) {
			process_config(i->first,**j,out);
		}
	}

	const map<string,string>& table = cfg.values;
	const map<string,string>::const_iterator id = table.find("id");

	if(element_name == "message") {
		const map<string,string>::const_iterator msg = table.find("message");

		if(id == table.end()) {
			static const std::string dummy;
			const std::string& text = msg != table.end() ? msg->second:dummy;
			std::cerr << "message found with no id: " << text << "\n";
			return;
		}

		if(msg == table.end()) {
			std::cerr << "message found with no text\n";
			return;
		}

		out.insert(std::pair<string,string>(id->second,msg->second));
	} else if(element_name == "part") {
		const map<string,string>::const_iterator msg = table.find("story");
		if(id == table.end() || msg == table.end()) {
			return;
		}

		out.insert(std::pair<string,string>(id->second,msg->second));
	} else if(element_name == "multiplayer" || element_name == "scenario") {
		map<string,string>::const_iterator msg = table.find("name");
		if(id == table.end() || msg == table.end()) {
			return;
		}

		out.insert(std::pair<string,string>(id->second,msg->second));

		if(element_name == "scenario") {
			msg = table.find("objectives");
			if(msg != table.end()) {
				out.insert(std::pair<string,string>(id->second + "_objectives",
				                                    msg->second));
			}
		}
	} else if(element_name == "language") {
		const map<string,string>::const_iterator name = table.find("language");
		if(name != table.end() && name->second == "English") {
			for(map<string,string>::const_iterator i = table.begin();
			    i != table.end(); ++i) {
				if(i->first != "language") {
					out.insert(*i);
				}
			}
		}
	} else if(element_name == "unit") {
		const map<string,string>::const_iterator name_it = table.find("name");
		if(name_it == table.end()) {
			return;
		}

		std::string name = name_it->second;
		name.erase(std::remove(name.begin(),name.end(),' '),name.end());

		out.insert(std::pair<string,string>(name,name_it->second));

		const map<string,string>::const_iterator description_it =
		                                  table.find("unit_description");
		if(description_it != table.end()) {
			out.insert(std::pair<string,string>(name + "_description",
			                                    description_it->second));
		}

		const map<string,string>::const_iterator ability_it =
		                                  table.find("ability");
		if(ability_it != table.end()) {
			out.insert(pair("ability_" + ability_it->second,
			                ability_it->second));
		}
	} else if(element_name == "attack") {
		const map<string,string>::const_iterator name_it=table.find("name");
		const map<string,string>::const_iterator type_it=table.find("type");
		const map<string,string>::const_iterator spec_it=table.find("special");
		if(name_it == table.end() || type_it == table.end()) {
			return;
		}

		out.insert(pair("weapon_name_" + name_it->second,name_it->second));
		out.insert(pair("weapon_type_" + type_it->second,type_it->second));

		if(spec_it == table.end()) {
			return;
		}

		out.insert(pair("weapon_special_" + spec_it->second,spec_it->second));
	}
}

int main()
{
	config cfg(preprocess_file("data/game.cfg"));

	map<string,string> table;
	process_config("",cfg,table);
	std::cout << "[language]\n\tlanguage=\"Language Name Goes Here\"\n" <<
	             "id=en  #language code - English=en, French=fr, etc\n";
	for(map<string,string>::const_iterator i = table.begin();
	    i != table.end(); ++i) {
		std::cout << "\t" << i->first << "=\"" << i->second << "\"\n";
	}
	std::cout << "[/language]\n";

	return 0;
}
