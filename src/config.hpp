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

//disable the very annoying VC++ warning 4786
#ifdef _WIN32
#pragma warning(disable:4786)
#endif

#include <cassert>
#include <map>
#include <sstream>
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

struct preproc_define {
	preproc_define() {}
	explicit preproc_define(const std::string& val) : value(val) {}
	preproc_define(const std::string& val, const std::vector<std::string>& args)
		: value(val), arguments(args) {}
	std::string value;
	std::vector<std::string> arguments;
};

typedef std::map<std::string,preproc_define> preproc_map;

//function to use the WML preprocessor on a file, and returns the resulting
//preprocessed file data. defines is a map of symbols defined. src is used
//internally and should be set to NULL
std::string preprocess_file(const std::string& fname,
                            const preproc_map* defines=0,
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

	typedef std::vector<config*>::iterator child_iterator;
	typedef std::vector<config*>::const_iterator const_child_iterator;

	typedef std::pair<child_iterator,child_iterator> child_itors;
	typedef std::pair<const_child_iterator,const_child_iterator>
	                                                  const_child_itors;

	child_itors child_range(const std::string& key);
	const_child_itors child_range(const std::string& key) const;

	const child_list& get_children(const std::string& key) const;
	const child_map& all_children() const;
	
	config* child(const std::string& key);
	const config* child(const std::string& key) const;
	config& add_child(const std::string& key);
	config& add_child(const std::string& key, const config& val);
	std::string& operator[](const std::string& key);
	const std::string& operator[](const std::string& key) const;

	config* find_child(const std::string& key, const std::string& name,
	                   const std::string& value);
	const config* find_child(const std::string& key, const std::string& name,
	                         const std::string& value) const;

	void clear_children(const std::string& key) { children.erase(key); }
	void remove_child(const std::string& key, size_t index) {
		child_list& v = children[key];
		assert(index < v.size());
		v.erase(v.begin()+index);
	}

	static std::vector<std::string> split(const std::string& val, char c=',');
	static std::string& strip(std::string& str);
	static bool has_value(const std::string& values, const std::string& val);

	void clear();

	struct error {
		error(const std::string& msg) : message(msg) {}
		std::string message;
	};

	struct child_pos {
		child_pos(child_map::const_iterator p, size_t i) : pos(p), index(i) {}
		child_map::const_iterator pos;
		size_t index;
	};

	struct all_children_iterator {
		typedef std::vector<child_pos>::const_iterator Itor;
		explicit all_children_iterator(Itor i);

		all_children_iterator operator++();
		all_children_iterator operator++(int);

		std::pair<const std::string*,const config*> operator*() const;

		bool operator==(all_children_iterator i) const;
		bool operator!=(all_children_iterator i) const;

	private:
		Itor i_;
	};

	//in-order iteration over all children
	all_children_iterator ordered_begin() const;
	all_children_iterator ordered_end() const;

	//all the attributes of this node.
	string_map values;

private:
	void write_internal(std::stringstream& str) const;

	//a list of all children of this node.
	child_map children;

	std::vector<child_pos> ordered_children;
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
