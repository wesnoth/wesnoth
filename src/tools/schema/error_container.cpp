/*
   Copyright (C) 2011 - 2014 by Sytyi Nick <nsytyi@gmail.com>
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
 * This file contains implementation of error_container.hpp.
 */

#include "tools/schema/error_container.hpp"

namespace schema_validation{

void class_error_container::add_simple_error(const std::string & message){
	list_.push_back(message);
}

void class_error_container::add_read_error(const std::string & file,int line){
	std::ostringstream s;
	s << file << ":" << line <<": Read error at line "<< line <<".\n";
	list_.push_back(s.str());
}

void class_error_container::add_opened_entity_error(
		const std::string & file,int line,const std::string & name){
	std::ostringstream s;
	s << file << ":" << line <<": Entity "<< name
			<<" is opened but not closed.\n";
	list_.push_back(s.str());
}

void class_error_container::add_unopened_entity_error(
		const std::string & file,int line,const std::string & name){
	std::ostringstream s;
	s << file << ":" << line <<": Entity "<< name
			<<" is being closed  but was not opened.\n";
	list_.push_back(s.str());
}

void class_error_container::add_second_parent_error(
		const std::string & file,int line,const std::string & first,
		const std::string & second){
	std::ostringstream s;
	s << file << ":" << line <<": Parent "<< first <<" is closed due to parent"
			<< second << "is opened here. \n";
	list_.push_back(s.str());
}

void class_error_container::add_orphan_error(const std::string & file,int line,
											 const std::string & name){
	std::ostringstream s;
	s << file << ":" << line <<": Tag "<< name <<" has no parent \n";
	list_.push_back(s.str());
}
void class_error_container::wrong_type_error(const std::string & file,int line,
											 const std::string & name,
											 const std::string & value){
	std::ostringstream s;
	s << file << ":" << line <<": Type "<< name <<" has wrong value:"<<
			value <<". Cannot create a regex\n";
	list_.push_back(s.str());
}

void class_error_container::add_type_error(const std::string &file,int line,
										   const std::string &type){
	types_[type].push_back(error_cache_element(file,line,type));
}

void class_error_container::remove_type_errors(const std::string &type){
	types_.erase(type);
}
void class_error_container::overriding_type_error(const std::string &file,
												  int line,
												  const std::string &type){
	std::ostringstream s;
	s << file << ":" << line <<": Type "<< type <<" is overriding here \n";
	list_.push_back(s.str());
}
void class_error_container::add_link_error(const std::string &file,int line,
										   const std::string &link){
	links_[link].push_back(error_cache_element(file,line,link));
}

void class_error_container::remove_link_errors(const std::string &link){
	links_.erase(link);
}


void class_error_container::print_errors(std::ostream & s)  const{
	for (std::vector<std::string>::const_iterator i = list_.begin();
	i!= list_.end(); ++i){
		s << *(i);
	}
	error_cache_map::const_iterator i = types_.begin() ;
	for ( ; i != types_.end(); ++i){
		for (std::vector<error_cache_element>::const_iterator ii=
			 i->second.begin();	ii != i->second.end();++ii){
			s << ii->file << ":" << ii->line <<": Unknown type: "
					<< ii->name <<"\n";
		}
	}

	for (i = links_.begin() ; i != links_.end(); ++i){
		for (std::vector<error_cache_element>::const_iterator ii =
			 i->second.begin();	ii != i->second.end();++ii){
			s << ii->file << ":" << ii->line <<": Failed link: "
					<< ii->name <<"\n";
		}
	}
}

}
