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

//an exception object used when an IO error occurs
struct io_exception : public std::exception {
	io_exception() {}
	io_exception(const std::string& msg) : message(msg) {}
	virtual ~io_exception() throw() {}

	virtual const char* what() const throw() { return message.c_str(); }
private:
	std::string message;
};

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

std::string get_dir(const std::string &dir);
//the location of various important files
std::string get_prefs_file();
std::string get_save_index_file();
std::string get_saves_dir();
std::string get_cache_dir();
std::string get_intl_dir();
std::string get_user_data_dir();

std::string get_cwd();

void make_directory(const std::string& dirname);

//basic disk I/O
bool filesystem_init();
std::string read_file(const std::string& fname);
std::istream *stream_file(std::string const &fname);
//throws io_exception if an error occurs
void write_file(const std::string& fname, const std::string& data);
std::string read_stdin();

std::string read_map(const std::string& name);

//function which returns true iff the given file is a directory
bool is_directory(const std::string& fname);

//function which returns true iff file with name already exists
bool file_exists(const std::string& name);

//function to get the creation time of a file
time_t file_create_time(const std::string& fname);

struct file_tree_checksum
{
	file_tree_checksum();
	explicit file_tree_checksum(const class config& cfg);
	void write(class config& cfg) const;
	size_t nfiles, sum_size;
	time_t modified;
};

bool operator==(const file_tree_checksum& lhs, const file_tree_checksum& rhs);
bool operator!=(const file_tree_checksum& lhs, const file_tree_checksum& rhs);


//function to get the time at which the most recently modified file
//in a directory tree was modified at
file_tree_checksum get_file_tree_checksum(const std::string& path);

//function to get the time at which the data/ tree was last modified at
const file_tree_checksum& data_tree_checksum();

//returns the size of a file, or -1 if the file doesn't exist
int file_size(const std::string& fname);

//returns the base filename of a file, with directory name stripped. Equivalent
//to a portable basename() function
std::string file_name(const std::string& file);

//returns the directory name of a file, with filename stripped. Equivalent to a
//portable dirname()
std::string directory_name(const std::string& file);

///the paths manager is responsible for recording the various paths that
///binary files may be located at. It should be passed a config object
///which holds binary path information. This is in the format
///[binary_path]
///path=<path>
///[/binary_path]
///Binaries will be searched for in [wesnoth-path]/data/<path>/images/
struct binary_paths_manager
{
	binary_paths_manager();
	binary_paths_manager(const class config& cfg);
	~binary_paths_manager();

	void set_paths(const class config& cfg);

private:
	binary_paths_manager(const binary_paths_manager& o);
	binary_paths_manager& operator=(const binary_paths_manager& o);

	void cleanup();

	std::vector<std::string> paths_;
};


//function which, given a type of binary, e.g. 'images', 'sounds', etc,
//will return a vector with all possible paths to that type of binary
const std::vector<std::string>& get_binary_paths(const std::string& type);

//function which, given a type of binary, and the name of the binary file,
//will return a complete path to the actual file, or an empty string if
//the file isn't present
std::string get_binary_file_location(const std::string& type, const std::string& filename);

#endif
