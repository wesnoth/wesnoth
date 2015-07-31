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
 * This file contains implementation of sourceparser.cpp.
 */

#include "tools/schema/sourceparser.hpp"

#include <boost/regex.hpp>

#include <stack>

namespace schema_validation{
/** Little parts of regex templates used to parse Wml annoations.
 *For details, look http://wiki.wesnoth.org/WML_Annotation_Format , please
 */
/** line is valid*/
const std::string valid = "^\\s*\\*\\s*";
/** begining of wiki block*/
const std::string wiki_begin ="^\\s*/\\*(?:WIKI|SCHEMA)";
/** whitespace is possible*/
const std::string space ="\\s*";
/** sigh "="*/
const std::string equals ="=";
/** non-mandatory sign "*/
const std::string quote_symbol ="\"?";
/** begining of the block.*/
const std::string block_begin="@begin" ;
/** end of block*/
const std::string block_end ="@end";
/** allow directive*/
const std::string allow="@allow";
/** remove directive*/
const std::string remove="@remove";
/** sign "{" - curly bracket*/
const std::string property_open="\\{";
/** sign "}" - another curly bracket*/
const std::string property_close= "\\}";
/** type of possible name identificator*/
const std::string name_type= "[a-z][a-zA-Z0-9_-]*" ;
/** type of possible parent indentificator*/
const std::string parent_type= "/|(?:[a-z][a-zA-Z0-9_-]*/)+";
/** type of possible link indentificator*/
const std::string link_type="(?:[a-z][a-zA-Z0-9_-]*/)*(?:[a-z][a-zA-Z0-9_-]*)";
/** template to number regex*/
const std::string number="\\d*";
/** sign "-" - hyphen-minus used to set sign of signed integer*/
const std::string sign="-?";

/**
 * end of line + possible various character before.
 * Used to close template. ".*" is used cause I dont want to get error
 * every misprint whitespace
 * after annotation element
 */
const std::string eol=".*$";

/** Private function to surround an argument with brackets.
 * This allows substitutions :-)
 */
static std::string sub (const std::string & s){
	return "(" + s + ")";
}

/** Private function to surround argument with not mandatory quotes.
 * Is used when creating properties
 */
static std::string quote(const std::string & s){
	return quote_symbol + s + quote_symbol ;
}

/**
 * Creates a property template
 * @param name Name of property
 * @param value Type of property value
 * If value is empty creates simple property like {table}
 * Else creates a named property like {name="[name_type_template]"}
 */
static std::string property(const std::string & name,
							const std::string & value = ""){
	if (value.empty()){
		return property_open + name + property_close;
	}
	return property_open + name + equals + quote(value) + property_close;
}


const std::string & get_valid() {
	return valid;
}

const std::string & get_wiki() {
	static std::string wiki = wiki_begin + eol;
	return wiki;
}

const std::string & get_parent_begin() {
	static std::string parent_begin = valid + block_begin
									  + property("parent")
									  + property("name",sub(parent_type))
									  + eol;
	return parent_begin;
}

const std::string & get_parent_end() {
	static std::string parent_end = valid + block_end + property("parent")
									+ property("name",sub(parent_type))+ eol;
	return parent_end;
}

const std::string & get_tag_begin() {
	static std::string tag_begin = valid + block_begin + property("tag")
								   + property("name",sub(name_type))
								   + property("min",sub(number))
								   + property("max",sub(sign + number))
								   + sub(property("super",sub(link_type)))
								   +"?" + eol;
	/* sub(property("super"),sub(link_type))+"?"
	 * property super is not mandatory
	 */
	return tag_begin;
}


const std::string & get_tag_end() {
	static std::string tag_end = valid + block_end + property("tag")
								 + property("name",sub(name_type)) + eol;
	return tag_end;
}


const std::string & get_allow_link(){
	static std::string allow_link = valid + allow + property("link")
									+property("name",sub(link_type)) + eol;
	return allow_link;
}

const std::string & get_allow_global(){
	static std::string global_link = valid + allow + property("global")
									 +property("name",sub(name_type)) + eol;
	return global_link;
}

static const std::string & get_allow_type(){
	static std::string allow_type = valid + allow + property("type")
									+property("name",sub(name_type))
									+property("value",sub("\\^.+\\$"))
									+ eol;
	return allow_type;
}
static const std::string & get_remove_type(){
	static std::string remove_type = valid + remove + property("type")
									+property("name",sub(name_type))
									+ eol;
	return remove_type;
}
static const std::string & get_remove_key(){
	static std::string remove_key = valid + remove + property("key")
									+property("name",sub(name_type))
									+ eol;
	return remove_key;
}

const std::string & get_table_key_begin() {
	static std::string keys_begin = valid + block_begin + property("table")
									+ property("config")+ eol;
	return keys_begin;
}

const std::string & get_table_end() {
	static std::string table_end = valid + block_end + property("table")
								   + eol;
	return table_end;
}


const std::string & get_key_value(){
	static std::string key_value = valid + sub("[a-zA-z][a-zA-Z\\d_+-]*")
								   + "\\s*&\\s*"+sub(name_type)
								   +"\\s*&\\s?"+ sub(quote("[a-zA-Z._0-9+-]*"))
								   +"\\s&"+ eol;
	return key_value;
}

void test_regex( std::ostream & f ){
	f << get_valid() << "\n"
			<< get_wiki() << "\n"
			<< get_parent_begin() << "\n"
			<< get_parent_end() << "\n"
			<< get_tag_begin() << "\n"
			<< get_tag_end() << "\n"
			<< get_allow_link() << "\n"
			<< get_allow_global() << "\n"
			<< get_table_key_begin() << "\n"
			<< get_table_end() << "\n"
			<< get_key_value() << "\n"
			<< get_allow_type() << "\n"
			<< std::endl;
}

bool class_source_parser::save_schema(){

	std::fstream out;
	if (output_.empty()){
		return false;
	}
	out.open(output_.c_str(),std::ios::out|std::ios::trunc);
	if (out.fail()){
		errors_.add_simple_error("Cannot open file "+output_+
								 "\n Output would not be stored\n");
		return false;
	}
	// remove all forbidden keys
	for (std::vector<std::string>::const_iterator i= forbidden_.begin ();
	i != forbidden_.end (); ++i){
		root_.remove_keys_by_type (*i);
		types_.erase (*i);
	}
	out << "[wml_schema]\n";
	for (std::map<std::string,std::string>::iterator i=types_.begin();
	i!= types_.end();++i){
		out << "    [type]\n"
			<< "        name=" << i->first << "\n"
			<< "        value=\""<< i->second << "\"\n"
			<<"    [/type]\n";
	}
	root_.print(out);
	out << "[/wml_schema]\n";
	out.close();
	return true; // @TODO add error support
}


bool class_source_parser::getline(std::string &s){
	if (f_.fail()){
		errors_.add_read_error(input_,line_);
		return false;
	}
	std::getline(f_,s);
	line_ ++;
	if (! f_.eof()){
		if (f_.fail()){
			errors_.add_read_error(input_,line_);
			return false;
		}
	}
	return true;
}
// call without arg when you want to closethem all
void class_source_parser::add_open_tag_error(int i = INT_MAX){
	std::vector<class_tag>::iterator it;
	for (it = current_.begin(); it != current_.end() && i > 0; ++it){
		errors_.add_opened_entity_error(input_,line_,it->get_name());
		--i;
	}
}
// call without arg when you want to closethem all
void class_source_parser::close_opened_tags(int i = INT_MAX){
	if (current_.empty()){
		return;
	}
	std::stack<std::string> error_cache ;
	while (current_.size() > 1){
		if (i==0){
			break;
		}
		class_tag tag (current_.back());
		current_.pop_back();
		current_.back().add_tag(tag);
		error_cache.push(tag.get_name());
		i--;
	}
	if (i!=0){
		//adding to parent
		if (parent_name_.empty()) {
			orphan_tags_.push_back(current_.back());
			errors_.add_orphan_error(input_,line_,current_.back().get_name());
		}else{
			error_cache.push(current_.back().get_name());
			root_.add_tag(parent_name_,current_.back(),root_);
		}
		current_.pop_back();
	}
	std::string name_to_remove_from_cache = parent_name_;
	for (std::vector<class_tag>::const_iterator ii = current_.begin();
	ii!= current_.end();++ii){
		name_to_remove_from_cache +=  ii->get_name() + "/";
	}
	while (! error_cache.empty()){
		name_to_remove_from_cache +=  error_cache.top();
		errors_.remove_link_errors(name_to_remove_from_cache);
		error_cache.pop();
	name_to_remove_from_cache += "/";
	}

}


bool class_source_parser::parse_source(){
	if (input_.empty()){
		errors_.add_simple_error("File was not defined\n");
		// Use  to hack sorting errors.
		// Item with  will be at the end of error list.
		return false;
	}

	f_.open(input_.c_str(),std::ios::in);
	if (f_.fail()){
		errors_.add_simple_error("File "+input_ + " cannot be opened\n");
		return false;
	}
	line_ = 0;
	bool result = true;
	while (!f_.eof()){
		std::string line;
		if (! getline(line) ) {
			f_.close();
			f_.clear();

			close_opened_tags();
			parent_name_.clear();
			return false;
		} // is used to avoid exceptions.

		if (check_wiki(line)) {
			result = parse_block();
			if (! result) {
				break;
			}
		}
	}

	f_.close();

	// Clear all flags ( eg the eof flag ) after closing the file.
	// This will let us reuse the same fstream variable for different files.
	f_.clear();

	close_opened_tags();
	parent_name_.clear();
	return result;
}


bool class_source_parser::parse_block(){
	while (!f_.eof()){
		std::string line;
		if (! getline(line) ) { return false; }
		if ( check_valid(line)) {
			if (check_allow_type(line)) continue;
			if (check_remove_type(line)) continue;
			if (check_parent_begin(line)) continue;

			if (check_tag_begin(line)){
				parse_tag();
				continue;
			}
			check_parent_end(line);
		}
		else{
			// wiki-block is closed. checking stack of opened tags
			if (!current_.empty()){
				add_open_tag_error();
				close_opened_tags();
				// continue working
			}
			// block is closed and all tags are closed.
			return true;

		}
	}// end while
	return false;
}

bool class_source_parser::parse_tag(){
	while (!f_.eof()){
		std::string line;
		if (! getline(line) ) { return false; }
		if (check_valid(line)) {
			if (check_tag_begin(line)){
				parse_tag();
			}
			if (check_tag_end(line)){
				return true;
			}
			if (check_keys_begin(line)){
				parse_keys();
			}
			if (check_allow_link(line)){
				continue;
			}
			if (check_allow_global(line)){
				continue;
			}
			if (check_remove_key(line)){
			}
		}else{
			if (!current_.empty()){
				// adding error messages for each unclosed entity
				add_open_tag_error();
				close_opened_tags();
			}
			return true;
		}
	}
	return true;
}


bool class_source_parser::parse_keys(){
	std::string line;
	do{
		if (! getline(line) ) { return false; }
		if (! check_valid(line)) {
			errors_.add_opened_entity_error(input_,line_,"Table config");
			add_open_tag_error();
			close_opened_tags();
			return false;
		}
		static const boost::regex value (get_key_value() );
		boost::smatch sub;
		bool res = boost::regex_match(line,sub,value);
		if (res){
			std::string type = sub[2];
			class_key key (sub[1],type,sub[3]);
			current_.back().add_key(key);
			if (types_.find(type) == types_.end()){
				errors_.add_type_error(input_,line_,type);
			}
		}
	}while (! check_keys_end(line));
	return true;
}


bool class_source_parser::check_valid(const std::string &s){
	// s must be like " *" or "*"
	static const boost::regex valid (get_valid());
	return boost::regex_search(s,valid);
}

bool class_source_parser::check_wiki(const std::string& s){
	boost::regex wiki (get_wiki());
	return boost::regex_match(s,wiki);
}

bool class_source_parser::check_tag_begin(const std::string &s){
	// read tag;
	static boost::regex tag (get_tag_begin());
	boost::smatch sub;
	bool res = boost::regex_match(s,sub,tag);
	if (res){
		std::string link = sub[5];
		class_tag new_tag;
		new_tag.set_name(sub[1]);
		new_tag.set_min(sub[2]);
		new_tag.set_max(sub[3]);
		new_tag.set_super(link);
		current_.push_back(new_tag);
		if (! link.empty() &&
				! static_cast<const class_tag>(root_).find_tag(link,root_)){
			errors_.add_link_error(input_,line_,link);
		}
		return true;
	}
	return false;
}

bool class_source_parser::check_tag_end(const std::string &s){
	static const boost::regex endtag (get_tag_end());
	boost::smatch sub;
	bool res = boost::regex_match(s,sub,endtag);
	if (res){
		std::string name = sub[1];
		if (current_.empty()){
			errors_.add_unopened_entity_error(input_,line_,name);
			return false;
		}
		std::vector<class_tag>::iterator ii = current_.end();
		int count_opened = 0;
		do{
			--ii;
			if (ii->get_name() == name){
				add_open_tag_error(count_opened);
				close_opened_tags(++count_opened);
				return true;
			}else{
				count_opened ++;
			}
		}while (ii != current_.begin()) ;
	}
	return false;
}

bool class_source_parser::check_allow_link(const std::string &s){
	static const boost::regex allow_link (get_allow_link());
	boost::smatch sub;
	bool res = boost::regex_match(s,sub,allow_link);
	if (res){
		if (!current_.empty()){
			std::string link = sub[1];
			current_.back().add_link(link);
			if (static_cast<const class_tag>(root_).find_tag(link,root_) == NULL){
				errors_.add_link_error(input_,line_,link);
			}
		}
	}
	return res;
}

bool class_source_parser::check_allow_global(const std::string &s){
	static const boost::regex allow_global (get_allow_global());
	boost::smatch sub;
	bool res = boost::regex_match(s,sub,allow_global);
	if (res){
		if (!current_.empty()){
			current_.back().add_link("global/"+sub[1]);
		}
	}
	return res;
}

bool class_source_parser::check_parent_begin(const std::string &s){
	static const boost::regex parent (get_parent_begin());
	boost::smatch sub;
	bool res = boost::regex_match(s,sub,parent);
	if (res){
		std::string name = sub[1];
		if (!parent_name_.empty()) {
			errors_.add_second_parent_error(input_,line_,parent_name_,name);
		}
		parent_name_ = name;
	}
	return res;
}

bool class_source_parser::check_parent_end(const std::string &s){
	static const boost::regex parent (get_parent_end());
	boost::smatch sub;
	bool res = boost::regex_match(s,sub,parent);
	if (res){
		std::string name = sub[1];
		if (parent_name_ == name) {
			parent_name_.clear();
		}else{
			errors_.add_unopened_entity_error(input_,line_,name);
		}
	}
	return true;
}

bool class_source_parser::check_keys_begin(const std::string &s){
	static const boost::regex keys (get_table_key_begin());
	return boost::regex_match(s,keys);

}

bool class_source_parser::check_keys_end(const std::string &s){
	static const boost::regex endkeys (get_table_end());
	bool res = boost::regex_match(s,endkeys);
	return res;
}

bool class_source_parser::check_allow_type(const std::string &s){
	static const boost::regex allow_type (get_allow_type());
	boost::smatch sub;
	bool res = boost::regex_match(s,sub,allow_type);
	if (res){
		std::string name = sub[1];
		std::string value = sub[2];
		try{
			boost::regex tmp (value);
		}catch(std::exception ){
			errors_.wrong_type_error(input_,line_,name,value);
			return true;
		}
		if(types_.find(name)!=types_.end()){
			errors_.overriding_type_error(input_,line_,name);
		}
		types_[name]=value;
		errors_.remove_type_errors(name);
	}
	return res;
}
bool class_source_parser::check_remove_type(const std::string &s){
	static const boost::regex remove_type (get_remove_type());
	boost::smatch sub;
	bool res = boost::regex_match(s,sub,remove_type);
	if (res){
		std::string name = sub[1];
		types_[name]="";
		forbidden_.push_back (name);
		errors_.remove_type_errors (name);
	}
	return res;
}
bool class_source_parser::check_remove_key(const std::string &s){
	static const boost::regex remove_key (get_remove_key());
	boost::smatch sub;
	bool res = boost::regex_match(s,sub,remove_key);
	if (res){
		if (! current_.empty ()){
			current_.back ().remove_key_by_name(sub[1]);
		}
	}
	return res;
}
} // namespace schema_generator
