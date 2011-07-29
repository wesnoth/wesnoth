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

#include "tools/schema/tag.hpp"

namespace schema_generator{	
/*WIKI
 * @begin{parent}{name="wml_schema/tag/"}
 * @begin{tag}{name="key"}{min=0}{max=-1}
 * @begin{table}{config}
 *     name & string & &              The name of key. $
 *     type & string & &              The type of key value. $
 *     default & string & &        The default value of the key. $
 *     mandatory & string & &   Shows if key is mandatory $
 * @end{table}
 * @end{tag}{name="key"}
 * @end{parent}{name="wml_schema/tag/"}
 */

void class_key::print(std::ostream& os,int level) const {
	std::string s;
	for (int j=0;j<level;j++){
		s.append(" ");
	}
	os << s << "[key]\n"
			<< s << "    name=\""<< name_ <<"\"\n"
			<< s << "    type=\""<< type_ <<"\"\n";
	if (is_mandatory()){
		os << s << "    mandatory=\"true\"\n";
	}else{
		os << s <<"    default="<< default_ <<"\n";
	}
	os << s << "[/key]\n";
}

void class_tag::print(std::ostream& os){
	printl(os,4,4);
}

void class_tag::add_link(const std::string &link){
	std::string::size_type pos_last = link.rfind('/');
	//if (pos_last == std::string::npos) return;
	std::string name_link = link.substr(++pos_last,link.length());
	links_.insert(std::pair<std::string,std::string>(name_link,link));
}

const class_key * class_tag::find_key(const std::string &name) const{
	key_map::const_iterator it_keys = keys_.find(name);
	if ( it_keys!= keys_.end() ){
		return &(it_keys->second);
	}
	return NULL;
}

const std::string * class_tag::find_link(const std::string &name) const{
	link_map::const_iterator it_links = links_.find(name);
	if ( it_links!= links_.end() ){
		return &(it_links->second);
	}
	return NULL;
}

const class_tag * class_tag::find_tag(const std::string &fullpath,
									   const class_tag &root) const{
	 if (fullpath.empty()) return NULL;
	 std::string::size_type pos = fullpath.find('/');
	 std::string name;
	 std::string next_path;
	 if (pos != std::string::npos) {
		 name = fullpath.substr(0,pos);
		 next_path = fullpath.substr(++pos,fullpath.length());
	 }else{
		 name = fullpath;
	 }
	 tag_map::const_iterator it_tags = tags_.find(name);
	 if (it_tags != tags_.end()){
		 if (next_path.empty()){
			 return &(it_tags->second);
				 }else{
			 return it_tags->second.find_tag(next_path,root);
		 }
	 }
	 link_map::const_iterator it_links = links_.find(name);
	 if (it_links != links_.end()){
		 return root.find_tag(it_links->second + "/" +next_path,root);
	 }
	 return NULL;

 }

void class_tag::expand_all(class_tag &root){
	for (tag_map::iterator i = tags_.begin(); i!= tags_.end(); ++i){
		i->second.expand(root);
		i->second.expand_all(root);
	}
}
/*WIKI
 * @begin{parent}{name="wml_schema/"}
 * @begin{tag}{name="tag"}{min=0}{max=1}
 * @begin{table}{config}
 *     name & string & &          The name of tag. $
 *     min & int & &           The min number of occurences. $
 *     max & int & &           The max number of occurences. $
 *     super & string & "" &   The super-tag of this tag $
 * @end{table}
 * @begin{tag}{name="link"}{min=0}{max=-1}
 * @begin{table}{config}
 *     name & string & &          The name of link. $
 * @end{table}
 * @end{tag}{name="link"}
 * @begin{tag}{name="tag"}{min=0}{max=-1}{super="wml_schema/tag"}
 * @end{tag}{name="tag"}
 * @end{tag}{name="tag"}
 * @begin{tag}{name="type"}{min=0}{max=-1}
 * @begin{table}{config}
 *     name & string & &          The name of type. $
 *     value & string & &         The value of the type, regex. $
 * @end{table}
 * @end{tag}{name="type"}
 * @end{parent}{name="wml_schema/"}
 */
void class_tag::printl(std::ostream &os,int level, int step){
	std::string s;
	for (int j=0;j<level;j++){
		s.append(" ");
	}
	os << s << "[tag]\n"
			<< s <<"    name=\""<< name_ <<"\"\n"
			<< s <<"    min=\""<< min_ <<"\"\n"
			<< s <<"    max=\""<< max_ <<"\"\n";
	if (! super_.empty() ){
		os<< s <<"    super=\""<< super_ <<"\"\n";
	}
	for (tag_map::iterator i = tags_.begin();
	i != tags_.end(); ++i ){
		i->second.printl(os,level+step,step);
	}
	for (link_map::iterator i = links_.begin();
	i != links_.end(); ++i ){
		os << s << "" << "[link]\n"
				<< s << "" << "    name=\"" <<i->second << "\"\n"
				<< s << "" << "[/link]\n";
	}
	for (key_map::iterator i = keys_.begin();
	i != keys_.end(); ++i ){
		i->second.print(os,level+step);
	}
	os<< s << "[/tag]\n";
}

 class_tag * class_tag::find_tag(const std::string &fullpath,
									   class_tag &root) {
	 if (fullpath.empty()) return NULL;
	 std::string::size_type pos = fullpath.find('/');
	 std::string name;
	 std::string next_path;
	 if (pos != std::string::npos) {
		 name = fullpath.substr(0,pos);
		 next_path = fullpath.substr(++pos,fullpath.length());
	 }else{
		 name = fullpath;
	 }

	 tag_map::iterator it_tags = tags_.find(name);
	 if (it_tags != tags_.end()){
		 if (next_path.empty()){
			 return &(it_tags->second);
				 }else{
			 return it_tags->second.find_tag(next_path,root);
		 }
	 }
	 link_map::iterator it_links = links_.find(name);
	 if (it_links != links_.end()){
		 return root.find_tag(it_links->second +"/" +next_path,root);
	 }

	 return NULL;

 }

void class_tag::add_tag(const std::string &path, const class_tag &tag,
						class_tag &root){
	if ( path.empty() || path == "/" ){
		tag_map::iterator it = tags_.find(tag.name_);
		if (it == tags_.end()){
			tags_.insert(tag_map_value(tag.name_,tag));
		}else{
			it->second.set_min(tag.min_);
			it->second.set_max(tag.max_);
			it->second.add_tags(tag.tags_);
			it->second.add_keys(tag.keys_);
			it->second.add_links(tag.links_);
		}
		return ;
	}
	std::string::size_type pos = path.find('/');
	std::string name = path.substr(0,pos);
	std::string next_path = path.substr(++pos,path.length());

	link_map::const_iterator it_links= links_.find(name);
	if (it_links != links_.end()){
		root.add_tag(it_links->second + "/" + next_path,tag,root);
	}
	tag_map::iterator it_tags = tags_.find(name);
	if (it_tags == tags_.end()){
		class_tag subtag;
		subtag.set_name(name);
		subtag.add_tag(next_path,tag,root);
		tags_.insert(tag_map_value(name,subtag));
		return;
	}
	it_tags->second.add_tag(next_path,tag,root);
}

void class_tag::append_super(const class_tag &tag,const std::string & path){
	add_keys(tag.keys_);
	add_links(tag.links_);
	for (tag_map::const_iterator i = tag.tags_.begin();i!=tag.tags_.end();++i){
		add_link(path + "/" + i->first);
	}
}

void class_tag::expand(class_tag &root){
	if (! super_.empty()){
		class_tag * super_tag = root.find_tag(super_,root);
		if (super_tag){
			if (super_tag != this){
				super_tag->expand(root);
				append_super(*super_tag,super_);
				super_.clear();
			}else{
				std::cerr << "the same" << super_tag->name_ <<"\n";
			}
		}
	}
}
}//namespace schema_generator
