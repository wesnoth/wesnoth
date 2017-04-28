/*
   Copyright (C) 2011 - 2017 by Sytyi Nick <nsytyi@gmail.com>
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
#include "gettext.hpp"
#include "log.hpp"
#include "serialization/preprocessor.hpp"
#include "wml_exception.hpp"

namespace schema_validation{

static lg::log_domain log_validation("validation");

#define ERR_VL LOG_STREAM(err, log_validation)
#define WRN_VL LOG_STREAM(warn, log_validation)
#define LOG_VL LOG_STREAM(info, log_validation)

static std::string at(const std::string & file, int line){
	std::ostringstream ss;
	ss << line << " " << file;
	return "at " + ::lineno_string(ss.str());
}

static void print_output(const std::string & message,bool flag_exception = false ){
#ifndef VALIDATION_ERRORS_LOG
	if(flag_exception){
			throw wml_exception("Validation error occured",message);
		}else{
	ERR_VL << message;
}
#else
// dirty hack to avoid "unused" error in case of compiling with definition on
	flag_exception = true;
	if (flag_exception){ ERR_VL << message;}
#endif
}

static void extra_tag_error(const std::string & file, int line,
							const std::string & name,int n,
							const std::string & parent, bool flag_exception){
	std::ostringstream ss;
	ss << "Extra tag [" << name << "]; there may only be "
	   << n << " [" << name << "] in [" << parent << "]\n"
	   << at(file, line) << "\n";
	print_output (ss.str (),flag_exception);
}

static void wrong_tag_error(const std::string & file, int line,
							const std::string & name,const std::string & parent,
							bool flag_exception){
	std::ostringstream ss;
	ss << "Tag [" << name << "] may not be used in ["
	   << parent << "]\n"
	   << at(file, line) << "\n";
	print_output (ss.str (),flag_exception);
}

static void missing_tag_error(const std::string & file, int line,
							  const std::string & name,int n,
							  const std::string & parent, bool flag_exception){
	std::ostringstream ss;
	ss << "Missing tag [" << name << "]; there must be "
	   << n << " [" <<  name  << "]s in [" << parent << "]\n"
	   << at(file, line) << "\n";
	print_output (ss.str (),flag_exception);
}

static void extra_key_error(const std::string & file, int line,
					 const std::string & tag,const std::string & key,
					 bool flag_exception){
	std::ostringstream ss;
	ss << "Invalid key '" << key << "=' in tag [" << tag
	   << "]\n"
	   << at(file, line) << "\n";
	print_output (ss.str (),flag_exception);
}

static void missing_key_error(const std::string & file, int line,
					 const std::string & tag,const std::string & key,
					 bool flag_exception){
	std::ostringstream ss;
	ss << "Missing key '" << key << "=' in tag [" << tag
	   << "]\n"
	   << at(file, line) << "\n";
	print_output (ss.str (),flag_exception);
}

static void wrong_value_error(const std::string & file, int line,
					 const std::string & tag,const std::string & key,
					 const std::string & value,bool flag_exception){
	std::ostringstream ss;
	ss << "Invalid value '" << value << "' in key '" << key
	   << "=' in tag [" << tag << "]\n"
	   << at(file, line) << "\n";
	print_output (ss.str (),flag_exception);
}



schema_validator::~schema_validator(){}

schema_validator::schema_validator(const std::string & config_file_name)
	: config_read_(false)
	, create_exceptions_(strict_validation_enabled)
	, root_()
	, stack_()
	, counter_()
	, cache_()
	, types_()
{
	if ( !read_config_file(config_file_name) ) {
		ERR_VL << "Schema file "<< config_file_name << " was not read." << std::endl;
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
		filesystem::scoped_istream stream = preprocess_file(filename, &preproc);
		read(cfg, *stream);
	} catch(config::error& e) {
		ERR_VL << "Failed to read file "<< filename << ":\n" << e.what() << "\n";
		return false;
	}
	for(const config &g : cfg.child_range("wml_schema")) {
		for(const config &schema : g.child_range("tag")) {
			if (schema["name"].str() == "root"){
				//@NOTE Don't know, maybe merging of roots needed.
				root_ = class_tag (schema);
			}
		}
		for(const config &type : g.child_range("type")) {
			try{
				types_[type["name"].str()] = boost::regex( type["value"].str());
			}
			catch (std::exception){
			// Need to check all type values in schema-generator
			}
		}
	}

	config_read_ = true;
	return true;
}
/*
 * Please, @Note that there is some magic in pushing and poping to/from stacks.
 * assume they all are on their place due to parser algorithm
 * and validation logic
 */
void schema_validator::open_tag(const std::string & name,
								int start_line,
								const std::string &file,
								bool addittion){
	if (! stack_.empty()){
		const class_tag * tag = nullptr;
		if (stack_.top()){
			tag = stack_.top()->find_tag(name,root_);
			if (! tag){
				wrong_tag_error(file,start_line,name,stack_.top()->get_name(),
								create_exceptions_);
			}else{
				if (! addittion){
					counter & cnt = counter_.top()[name];
					++ cnt.cnt;
				}
			}
		}
		stack_.push(tag);
	}else{
		stack_.push(nullptr);
	}
	counter_.push(cnt_map());
	cache_.push(message_map());
}

void schema_validator::close_tag(){
	stack_.pop();
	counter_.pop();
	//cache_ is cleared in another place.
}

void schema_validator::validate(const config & cfg, const std::string & name,
								int start_line,
								const std::string &file){
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
	// Checking all elements counters.
	if (!stack_.empty() && stack_.top() && config_read_){
		class_tag::all_const_tag_iterators p = stack_.top()->tags();
		for (class_tag::const_tag_iterator tag = p.first;
			 tag != p.second ; ++tag){
			int cnt = counter_.top()[tag->first].cnt;
			if (tag->second.get_min() > cnt){
				cache_.top()[&cfg].push_back(
						message_info(MISSING_TAG,file,start_line,
									 tag->second.get_min(),tag->first,"",
									 name));
				continue;
			}
			if (tag->second.get_max() < cnt){
				cache_.top()[&cfg].push_back(
						message_info(EXTRA_TAG,file,start_line,
									 tag->second.get_max(),tag->first,"",
									 name));
			}
		}
		// Checking if all mandatory keys are present
		class_tag::all_const_key_iterators k = stack_.top()->keys();
		for (class_tag::const_key_iterator key = k.first;
			 key != k.second ; ++key){
			if (key->second.is_mandatory()){
				if (cfg.get(key->first) == nullptr){
					cache_.top()[&cfg].push_back(
							message_info(MISSING_KEY,file,start_line,0,
										 name,key->first ));
				}
			}
		}
	}
}


void schema_validator::validate_key(const config & cfg,
				  const std::string & name,
				  const std::string & value,
				  int start_line,
				  const std::string &file){
	if (!stack_.empty() && stack_.top() && config_read_){
		// checking existing keys
		const class_key * key =stack_.top()->find_key(name);
		if (key){
			std::map<std::string,boost::regex>::iterator itt =
					types_.find(key->get_type());
			if (itt != types_.end()){
				boost::smatch sub;
				bool res = boost::regex_match(value,sub,itt->second);
				if (!res ) {
					cache_.top()[&cfg].push_back(
							message_info(WRONG_VALUE,file,start_line,0,
										 stack_.top()->get_name(),
										 name,value));
				}
			}
		}
		else{
			cache_.top()[&cfg].push_back(
					message_info(EXTRA_KEY,file,start_line,0,
								 stack_.top()->get_name(),name));
		}

	}
}

void schema_validator::print(message_info & el){
	switch (el.type){
	case WRONG_TAG:
		wrong_tag_error(el.file,el.line,el.tag,el.value,create_exceptions_);
		break;
	case EXTRA_TAG:
		extra_tag_error(el.file,el.line,el.tag,el.n,el.value,create_exceptions_);
		break;
	case MISSING_TAG:
		missing_tag_error(el.file,el.line,el.tag,el.n,el.value,
						  create_exceptions_);
		break;
	case EXTRA_KEY:
		extra_key_error(el.file,el.line,el.tag,el.key,create_exceptions_);
		break;
	case WRONG_VALUE:
		wrong_value_error(el.file,el.line,el.tag,el.key,el.value,
						  create_exceptions_);
		break;
	case MISSING_KEY:
		missing_key_error(el.file,el.line,el.tag,el.key,create_exceptions_);
	}
}
}//namespace schema_validation{
