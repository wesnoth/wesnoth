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

#include <string>
#include <vector>

enum FILE_NAME_MODE { ENTIRE_FILE_PATH, FILE_NAME_ONLY };

void get_files_in_dir(const std::string& dir,
                      std::vector<std::string>* files,
                      std::vector<std::string>* dirs=NULL,
                      FILE_NAME_MODE mode=FILE_NAME_ONLY);

std::string get_prefs_file();
std::string get_saves_dir();
std::string get_user_data_dir();

bool is_directory(const std::string& fname);

#endif
