/* $Id$ */
/*
   Copyright (C) 2011 - 2011 by Sytyi Nick <nsytyi@gmail.com>
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

#include "./tools/schema/error_container.hpp"

namespace schema_generator{

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
	s << file << ":" << line <<": Parent "<< first <<" is closed due  to parent"
			<< second << "is opened here. \n";
	list_.push_back(s.str());
}

void class_error_container::add_orphan_error(const std::string & file,int line,
											 const std::string & name){
	std::ostringstream s;
	s << file << ":" << line <<": Tag "<< name <<" has no parent \n";
	list_.push_back(s.str());
}

void class_error_container::print_errors(std::ostream & s)  const{
	for (unsigned int i = 0; i< list_.size(); i++){
		s << list_.at(i);
	}
}
void class_error_container::sort(){
	std::sort(list_.begin(),list_.end());
}
}
