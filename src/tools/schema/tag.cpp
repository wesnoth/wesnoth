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
 * Implementation of tag.hpp.
 */

#include "./tools/schema/tag.hpp"

namespace schema_generator{
void class_key::print(std::ostream& os,int level) const {
	std::string s;
	for (int j=0;j<level;j++){
		s.append(" ");
	}
	os << s << "[key]\n"
			<< s << "name=\""<< name_ <<"\"\n"
			<< s << "type=\""<< type_ <<"\"\n";
	if (is_mandatory()){
		os << s << "mandatory=\"true\"\n";
	}else{
		os << s <<"default="<< default_ <<"\n";
	}
	os << s << "[/key]\n";
}

void class_tag::print(std::ostream& os){
	printl(os,4,4);
}


void class_tag::printl(std::ostream &os,int level, int step){
	std::sort (keys_.begin(), keys_.end());
	std::sort (tags_.begin(), tags_.end());
	std::sort (links_.begin(), links_.end());
	std::string s;

	for (int j=0;j<level;j++){
		s.append(" ");
	}
	os << s << "[tag]\n"
			<< s <<"name=\""<< name_ <<"\"\n"
			<< s <<"min=\""<< min_ <<"\"\n"
			<< s <<"max=\""<< max_ <<"\"\n";
	if (! super_.empty() ){
		os<< s <<"super=\""<< super_ <<"\"\n";
	}
	for (std::vector<class_tag>::iterator i = tags_.begin();
	i != tags_.end(); ++i ){
		i->printl(os,level+step,step);
	}
	for (std::vector<std::string>::iterator i = links_.begin();
	i != links_.end(); ++i ){
		os << s << "" << "[link]\n"
				<< s << "" << "name=\"" <<*i << "\"\n"
				<< s << "" << "[/link]\n";
	}
	for (std::vector<class_key>::iterator i = keys_.begin();
	i != keys_.end(); ++i ){
		i->print(os,level+step);
	}
	os<< s << "[/tag]\n";
}

const std::string  class_tag::add_tag(const std::string &path,
									  const class_tag &tag){
	if ( path.empty() || path == "/" ){
		std::vector<class_tag>::iterator it =
				std::find(tags_.begin(),tags_.end(),tag);
		if (it == tags_.end()){
			tags_.push_back(tag);
		}else{
			it->add_tags(tag.tags_);
			it->set_min(tag.min_);
			it->set_max(tag.max_);
			it->add_keys(tag.keys_);
		}
		return "";
	}
	int pos = path.find('/');
	std::string name = path.substr(0,pos);
	std::string next_path = path.substr(++pos,path.length());
	std::vector<std::string>::iterator it_links= links_.begin();
	for (;it_links != links_.end();++it_links){
		std::string::size_type pos_last = it_links->rfind('/');
		//	if (pos_last > it_links->length()) {
		//continue;
		//}
		std::string name_link = it_links->substr(++pos_last,path.length());
		if (name_link == name) {
			break;
		}
	}
	if (it_links != links_.end()){
		return  *it_links + "/" + next_path;
	}
	std::vector<class_tag>::iterator it_tags = tags_.begin();
	// = std::find(tags_.begin(),tags_.end(),class_tag(name,0,0);
	for (;it_tags!=tags_.end(); ++it_tags){
		if ( it_tags->name_ == name ){
			break;
		}
	}
	if (it_tags == tags_.end()){
		class_tag subtag;
		subtag.set_name(name);
		subtag.add_tag(next_path,tag);
		tags_.push_back(subtag);
		return "";
	}
	return it_tags->add_tag(next_path,tag);
}


}//namespace schema_generator
