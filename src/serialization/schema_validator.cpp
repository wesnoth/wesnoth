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

#include "serialization/schema_validator.hpp"


#include "filesystem.hpp"
#include "foreach.hpp"
#include "log.hpp"
#include "serialization/preprocessor.hpp"
#include "wml_exception.hpp"

namespace schema_validation{

static lg::log_domain log_validation("validation");

#define ERR_VL LOG_STREAM(err, log_validation)
#define WRN_VL LOG_STREAM(warn, log_validation)
#define LOG_VL LOG_STREAM(info, log_validation)

std::string at(const std::string & file, int line){
	std::ostringstream ss;
	ss << line << " " << file;
	return ::lineno_string(ss.str());
}

void extra_tag_error(const std::string & file, int line,
					 const std::string & name){
	WRN_VL << at(file,line) << ": extra tag "<< name << "\n";
}
void wrong_tag_error(const std::string & file, int line,
					 const std::string & name){
	std::ostringstream ss;
	ss 	 <<at(file,line) << ": wrong tag "<< name << "\n";
	WRN_VL << ss.str();
	//throw twml_exception("Validation error",ss.str ());
}
void missing_tag_error(const std::string & file, int line,
					 const std::string & name){
	WRN_VL <<at(file,line) << ": missing tag "<< name << "\n";
}
void extra_key_error(const std::string & file, int line,
					 const std::string & tag,const std::string & key
					 ){
	WRN_VL << at(file,line) << ": In tag "<< tag
			<< " which begins here, " << "key "<< key << " wasn't allowed\n";
}
void missing_key_error(const std::string & file, int line,
					 const std::string & tag,const std::string & key
					 ){
	WRN_VL << at(file,line) << ": In tag "<< tag
			<< " which begins here, " << " missing key "<< key << "\n";
}
void wrong_value_error(const std::string & file, int line,
					 const std::string & tag,const std::string & key,
					 const std::string & value){
	WRN_VL << at(file,line) << ": In tag "<< tag
			<< " which begins here, " << "key "<< key << " have wrong value "
			<< value << "\n";
}

schema_validator::schema_validator():config_read_(false),stack_(){
	ERR_VL << "No schema file\n";
	throw abstract_validator::error("No schema file\n");
}

schema_validator::~schema_validator(){}

schema_validator::schema_validator(const std::string & config_file_name)
	:stack_(){
	config_read_ = read_config_file(config_file_name);
	if (! config_read_) {
		ERR_VL << "Schema file "<< config_file_name << " was not read.\n";
		throw abstract_validator::error("Schema file "+ config_file_name
										+ " was not read.\n");
	}else{
		stack_.push(&root_);
		counter_.push(cnt_map());
		cache_.push(message_map());
		root_.expand_all(root_);
		LOG_VL << "Schema file "<< config_file_name << " was read.\n"
				<< "Validator initialized\n";
	}
}

bool schema_validator::read_config_file(const std::string &filename){
	config cfg;
	try {
		preproc_map preproc(
				game_config::config_cache::instance().get_preproc_map());
		scoped_istream stream = preprocess_file(filename, &preproc);
		read(cfg, *stream);
	} catch(config::error&) {
		return false;
	}
	foreach (const config &g, cfg.child_range("wml_schema")) {
		foreach (const config &schema, g.child_range("tag")) {
			if (schema["name"].str() == "root"){
				//@NOTE Don't know, maybe merging of roots needed.
				root_ = class_tag (schema);
			}
		}
		foreach (const config &type, g.child_range("type")) {
			try{
				types_[type["name"].str()] = boost::regex( type["value"].str());
			}
			catch (std::exception){
			// Need to check all type values in schema-generator
			}
		}
	}

	return true;
}
/*
 * Please, @Note that there is some magic in pushing and poping to/from stacks.
 * assume they all are on their place due to parser algorithm
 * and validation logic
 */
void schema_validator::open_tag(const std::string & name,int start_line,
							   const std::string &file){
	if (! stack_.empty()){
		const class_tag * tag = NULL;
		if (stack_.top()){
			tag = stack_.top()->find_tag(name,root_);
			if (! tag){
				wrong_tag_error(file,start_line,name);
			}else{
				counter & cnt = counter_.top()[name];
				++ cnt.cnt;
			}
		}
		stack_.push(tag);
	}else{
		stack_.push(NULL);
	}
	counter_.push(cnt_map());
	cache_.push(message_map());
}

void schema_validator::close_tag(){
	stack_.pop();
	counter_.pop();
	//cache_ is cleared in another place.
}

bool schema_validator::validate(const config & cfg, const std::string & name,
								int start_line,
								const std::string &file){
	bool retval = false;
	//close previous errors and print them to output.
	message_map::iterator cache_it = cache_.top().begin();
	for (;cache_it != cache_.top().end();++cache_it){
		for (message_list::iterator i = cache_it->second.begin();
		i != cache_it->second.end(); ++i){
			print(*i);
		}
	}
	cache_.pop();
	// clear cache
	cache_it = cache_.top().find(&cfg);
	if (cache_it != cache_.top().end()){
		cache_it->second.clear();
	}
	// Please note that validating unknown tag keys the result will be false
	if (!stack_.empty() && stack_.top() && config_read_){
		retval = true;
		// checking existing keys
		foreach (const config::attribute & attr, cfg.attribute_range()){
			const class_key * key =stack_.top()->find_key(attr.first);
			if (key){
				std::map<std::string,boost::regex>::iterator itt =
						types_.find(key->get_type());
				if (itt!= types_.end()){
					boost::smatch sub;
					bool res = boost::regex_match(attr.second.str(),
												  sub,itt->second);
					if (!res ) {
						cache_.top()[&cfg].push_back(
								message_info(WRONG_VALUE,file,start_line,
										   stack_.top()->get_name(),
										   key->get_name(),
										   attr.second.str()));
					}
				}
			}
			else{
				cache_.top()[&cfg].push_back(
						message_info(EXTRA_KEY,file,start_line,name,attr.first));
				retval = false;
			}
		}
		// Checking all elements counters.
		class_tag::all_const_tag_iterators p = stack_.top()->tags();
		for (class_tag::const_tag_iterator tag = p.first;
			 tag != p.second ; ++tag){
			int cnt = counter_.top()[tag->first].cnt;
			if (tag->second.get_min() > cnt){
				cache_.top()[&cfg].push_back(
						message_info(MISSING_TAG,file,start_line,tag->first ));
				continue;
			}
			if (tag->second.get_max() < cnt){
				cache_.top()[&cfg].push_back(
						message_info(EXTRA_TAG,file,start_line,tag->first ));
			}
		}
		// Checking if all mandatory keys are present
		class_tag::all_const_key_iterators k = stack_.top()->keys();
		for (class_tag::const_key_iterator key = k.first;
			 key != k.second ; ++key){
			if (key->second.is_mandatory()){
				if (cfg.get(key->first) == NULL){
					cache_.top()[&cfg].push_back(
							message_info(MISSING_KEY,file,start_line,
									   stack_.top()->get_name(),key->first ));
				}
			}
		}
	}
	return retval;
}

void schema_validator::print(message_info & el){
	switch (el.type){
	case WRONG_TAG:
		wrong_tag_error(el.file,el.line,el.tag);
		break;
	case EXTRA_TAG:
		extra_tag_error(el.file,el.line,el.tag);
		break;
	case MISSING_TAG:
		missing_tag_error(el.file,el.line,el.tag);
		break;
	case EXTRA_KEY:
		extra_key_error(el.file,el.line,el.tag,el.key);
		break;
	case WRONG_VALUE:
		wrong_value_error(el.file,el.line,el.tag,el.key,el.value);
		break;
	case MISSING_KEY:
		missing_key_error(el.file,el.line,el.tag,el.key);
	}
}
}//namespace schema_validation{
