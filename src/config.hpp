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
#ifndef CONFIG_HPP_INCLUDED
#define CONFIG_HPP_INCLUDED

#include <map>
#include <string>
#include <vector>

//This module defines the interface to Wesnoth Markup Language (WML).
//WML is a simple hierarchical text-based file format. The format
//is defined in Wiki, under BuildingScenariosWML
//
//All configuration files are stored in this format, and data is
//sent across the network in this format. It is thus used extensively
//throughout the game.

//an object which defines the location an error occurred at when
//parsing WML files
struct line_source
{
	line_source(int ln,const std::string& fname, int line) :
	              linenum(ln), file(fname), fileline(line)
	{}

	int linenum;
	std::string file;
	int fileline;
};

bool operator<(const line_source& a, const line_source& b);

//basic disk I/O
std::string read_file(const std::string& fname);
void write_file(const std::string& fname, const std::string& data);

//function to use the WML preprocessor on a file, and returns the resulting
//preprocessed file data. defines is a map of symbols defined. src is used
//internally and should be set to NULL
std::string preprocess_file(const std::string& fname,
                            const std::map<std::string,std::string>* defines=0,
                            std::vector<line_source>* src=0);

typedef std::map<std::string,std::string> string_map;

//a config object defines a single node in a WML file, with access to
//child nodes.
struct config
{
	//create an empty node.
	config() {}

	//create a node from data
	config(const std::string& data,
	       const std::vector<line_source>* lines=0); //throws config::error
	config(const config& cfg);
	~config();

	config& operator=(const config& cfg);

	//read data in, clobbering existing data.
	void read(const std::string& data,
	          const std::vector<line_source>* lines=0); //throws config::error
	std::string write() const;

	typedef std::vector<config*> child_list;
	typedef std::map<std::string,child_list> child_map;

	//a list of all children of this node.
	child_map children;

	//all the attributes of this node.
	string_map values;

	typedef std::vector<config*>::iterator child_iterator;
	typedef std::vector<config*>::const_iterator const_child_iterator;

	typedef std::pair<child_iterator,child_iterator> child_itors;
	typedef std::pair<const_child_iterator,const_child_iterator>
	                                                  const_child_itors;

	child_itors child_range(const std::string& key);
	const_child_itors child_range(const std::string& key) const;
	
	config* child(const std::string& key);
	const config* child(const std::string& key) const;
	config& add_child(const std::string& key);
	std::string& operator[](const std::string& key);
	const std::string& operator[](const std::string& key) const;

	config* find_child(const std::string& key, const std::string& name,
	                   const std::string& value);
	const config* find_child(const std::string& key, const std::string& name,
	                         const std::string& value) const;

	static std::vector<std::string> split(const std::string& val);
	static std::string& strip(std::string& str);
	static bool has_value(const std::string& values, const std::string& val);

	void clear();

	struct error {
		error(const std::string& msg) : message(msg) {}
		std::string message;
	};
};

struct config_has_value {
	config_has_value(const std::string& name, const std::string& value)
	              : name_(name), value_(value)
	{}

	bool operator()(const config* cfg) const { return (*cfg)[name_] == value_; }

private:
	const std::string name_, value_;
};

#endif
