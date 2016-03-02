/*
   Copyright (C) 2003 - 2016 by Jörg Hinrichs, refactored from various
   places formerly created by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SAVE_INDEX_H_INCLUDED
#define SAVE_INDEX_H_INCLUDED

#include "config.hpp"
#include "serialization/compression.hpp"

class config_writer;
class game_display;


namespace savegame {

/** Filename and modification date for a file list */
class save_info {
private:
	friend class create_save_info;
private:
	save_info(const std::string& name, const time_t& modified) :
		name_(name), modified_(modified)
	{}

public:
	const std::string& name()     const  { return name_; }
	const time_t&      modified() const  { return modified_; }
public:
	std::string format_time_summary() const;
	std::string format_time_local() const;
	const config& summary() const;
private:
	std::string name_;
	time_t modified_;
};

/**
 * A structure for comparing to save_info objects based on their modified time.
 * If the times are equal, will order based on the name.
 */
struct save_info_less_time {
	bool operator()(const save_info& a, const save_info& b) const;
};

std::vector<save_info> get_saves_list(const std::string* dir = NULL, const std::string* filter = NULL);

/** Read the complete config information out of a savefile. */
void read_save_file(const std::string& name, config& cfg, std::string* error_log);

/** Remove autosaves that are no longer needed (according to the autosave policy in the preferences). */
void remove_old_auto_saves(const int autosavemax, const int infinite_auto_saves);

/** Delete a savegame. */
void delete_game(const std::string& name);


class create_save_info {
public:
	create_save_info(const std::string* d = NULL) ;
	save_info operator()(const std::string& filename) const ;
	const std::string dir;
};


class save_index_class
{
public:
	void rebuild(const std::string& name) ;
	void rebuild(const std::string& name, const time_t& modified) ;
	void remove(const std::string& name) ;
	void set_modified(const std::string& name, const time_t& modified) ;
	config& get(const std::string& name) ;
public:
	void write_save_index() ;

public:
	save_index_class();
private:
	config& data(const std::string& name) ;
	config& data() ;
private:
	bool loaded_;
	config data_;
	std::map< std::string, time_t > modified_;
};
extern save_index_class save_index_manager;
} //end of namespace savegame

void replace_underbar2space(std::string &name);
void replace_space2underbar(std::string &name);

#endif
