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
#ifndef FILESYSTEM_HPP_INCLUDED
#define FILESYSTEM_HPP_INCLUDED

#include <time.h>

#include <string>
#include <vector>

enum FILE_NAME_MODE { ENTIRE_FILE_PATH, FILE_NAME_ONLY };

//function which populates files with all the files and dirs
//with all the directories in dir. If files or dirs are NULL
//they will not be used.
//
//mode determines whether the entire path or just the filename
//is retrieved.
void get_files_in_dir(const std::string& dir,
                      std::vector<std::string>* files,
                      std::vector<std::string>* dirs=NULL,
                      FILE_NAME_MODE mode=FILE_NAME_ONLY);

//the location of various important files
std::string get_prefs_file();
std::string get_saves_dir();
std::string get_user_data_dir();

//function which returns true iff the given file is a directory
bool is_directory(const std::string& fname);

//function which returns true iff file with name already exists
bool file_exists(const std::string& name);

time_t file_create_time(const std::string& fname);

#endif
