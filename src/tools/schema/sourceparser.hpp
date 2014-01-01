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
 * This file contains sourceparser object, collecting annotations
 * and building a tag tree.
 * Also here are declared fuctions that return regex templates.
 * And a fuction to create a file with list of templates.
 */

#ifndef TOOLS_SCHEMA_SOURCEPARSER_HPP_INCLUDED
#define TOOLS_SCHEMA_SOURCEPARSER_HPP_INCLUDED

#include "tools/schema/error_container.hpp"
#include "tools/schema/tag.hpp"

#include <iostream>
#include <fstream>
#include <map>
#include <queue>
#include <string>
#include <vector>

namespace schema_validation{
/** A few regex templates. Please notice, that adding any regex template,
 * schould be called in test_regexes()
 */

/** Template to check line of beeing valid*/
const std::string & get_valid();
/** Template to check line is beginnnig of Wiki block*/
const std::string & get_wiki();
/** Template to check begining of parent block*/
const std::string & get_parent_begin() ;
/** Template to check closing of parent block*/
const std::string & get_parent_end() ;
/** Template to check if line contains opening of tag block*/
const std::string & get_tag_begin() ;
/** Template to check end of tag block*/

const std::string & get_tag_end() ;

/** Template to check allow{link} block*/
const std::string & get_allow_link();
/** Template to check allow{global} block*/
const std::string & get_allow_global();

/** Template to check begining of table{config} storing key values*/
const std::string & get_table_key_begin() ;
/** Template to check if table is closed*/
const std::string & get_table_end() ;

/** Template to get key value*/
const std::string & get_key_value();
/** Writes to the file regex templates list.*/
void test_regex(std::ostream & f );

class class_source_parser{
public:
	class_source_parser():
			input_ (""),
			output_(""),
			f_(),
			line_(0),
			current_(),
			root_(class_tag("root",1,1)),
			parent_name_(""),
			orphan_tags_(),
			errors_(),
			types_(),
			forbidden_()
	{
	}

	~class_source_parser(){
	}

	void set_input(const std::string &s){
		input_ = s;
	}
	void set_output(const std::string &s){
		output_ = s;
	}
	/**
	 * Parses file line-by-line, checking every line to open WIKI block
	 * Please, notice that main input work is made in check_*** methods.
	 * Methods parse_*** are used to organize alhoritm and set which
	 * regex template can be used in this context.
	 * The only exception is parse_keys, where table of keys is read
	 * and add to top of stack tags.
	 */
	bool parse_source();
	/**
	 * Saves tag tree to schema file.
	 */
	bool save_schema();

	/**
	 * Expands all tags.
	 * While expanding tag copies list of keys and links from super-tag
	 * And adds links to super-tag children to links list.
	 * Useful when debugging the schema_markup
	 */
	void expand(){
		root_.expand_all(root_);
	}


	const std::vector<class_tag> & see_orphans() const{
		return orphan_tags_;
	}
	/** Grants access to error container*/
	const class_error_container & see_errors() const{
		return errors_;
	}
private:
	/** name of input file to be parsed*/
	std::string input_;
	/** name of output file to print schema*/
	std::string output_;
	std::fstream f_;
	//current line number
	/** number of current read line. Is used when generating errors*/
	int line_;
	//vector-based stack. The element on top is current opened tag.
	/** Stack of opened tags.*/
	std::vector<class_tag> current_;
	/** Root of the schema tree*/
	class_tag root_;
	/** Name of current parent*/
	std::string parent_name_;
	/** List of tags without parents.*/
	std::vector<class_tag> orphan_tags_;
	/** used to store errors*/
	class_error_container  errors_;
	/** Allowed types*/
	std::map<std::string,std::string> types_;
	/** Types to remove*/
	std::vector<std::string> forbidden_;
	/**
	 * Parses WIKI block line-by-line, checking every line
	 * to open annotation block
	 */
	bool parse_block();
	/**
	 * Parses lines inside tag block.
	 * Calls checkers that are allowed in tag block
	 */
	bool parse_tag();
	/**
	 * Read key table and add keys to tag on the top of the stack.
	 */
	bool parse_keys();

	/** check the input line with a template
	 * check if the line is valid (still in block)
	 */
	bool check_valid(const std::string& s);
	/**
	 * Gets a line from file and returns it.
	 * Is used to manage exceptions while IO.
	 */
	bool getline(std::string & s);
	/**
	 * Сhecks stack of opened tags. If any tags is opened - closes it
	 *and adds to sublist of next tag in stack.
	 * Add last tag in stack to parent
	 * @param i number of tags in stack to close.
	 */
	void close_opened_tags(int i);
	/** Generates errors for each opened tag.
	 * @param i number of tags in stack to complain.
	 */
	void add_open_tag_error(int i);


	/**
	 * Read tag form the line and add it to stack
	 */
	bool check_wiki(const std::string& s);

	/** Checks line for tag annotation. Reads tag and puts in into stack.*/
	bool check_tag_begin(const std::string& s);
	/**
	 * Puts closed tag to child list of previous tag.
	 * Also closes all opened child tags, if they are, and generates warnings.
	 */
	bool check_tag_end(const std::string& s);

	/** Opens parrent block*/
	bool check_parent_begin(const std::string& s);
	/** Closes parent block*/
	bool check_parent_end(const std::string& s);
	/** Checks beginning of keys*/
	bool check_keys_begin(const std::string&s);
	/** Checks end of keys*/
	bool check_keys_end(const std::string&s);
	/** Checks links*/
	bool check_allow_link(const std::string & s);
	/** Checks allowed global tags*/
	bool check_allow_global(const std::string &s);
	/** Checks allowed types*/
	bool check_allow_type(const std::string &s);
	/** Checks removed types*/
	bool check_remove_type(const std::string &s);
	/** Checks removed keys*/
	bool check_remove_key(const std::string &s);
};
} // namespace schema_validation

#endif // TOOLS_SCHEMA_SOURCEPARSER_HPP_INCLUDED
