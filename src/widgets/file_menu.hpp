/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef FILE_MENU_H_INCLUDED
#define FILE_MENU_H_INCLUDED

#include "../construct_dialog.hpp"

namespace gui {


/// A widget where the user may navigate through directories and choose
/// a file.
class file_menu : public menu {
public:
	//Static members
	static const char path_delim;

	/// Initialize the file menu. start_file is the file that will be
	/// selected initially. The current directory will be
	/// the one the file is in.
	file_menu(CVideo &disp, std::string start_file="");

	/// Return the chosen file.
	std::string get_choice() const;

	int delete_chosen_file();

	std::string get_directory() const { return current_dir_; }

	/// Return path with to_add added, using a path delimiter between them.
	std::string add_path(const std::string& path, const std::string& to_add) const;

	/// Return the string with the last path delimiter removed, if one
	/// was there.
	std::string strip_last_delim(const std::string& path) const;

	bool is_directory(const std::string& fname) const;

	void change_directory(const std::string& path);
	bool make_directory(const std::string& subdir_name);

	/**
	 * Selects file (type-a-head search)
	 **/
	void select_file(const std::string& begin_of_filename);

	bool type_a_head() const;
	void reset_type_a_head();

protected:
	void handle_event(const SDL_Event& event);

private:
	/// If file_or_dir is a file, return the directory the file is in,
	/// if it is a directory, return the directory name. If no path
	/// delimiters could be found, return the unchanged argument.
	std::string get_path(const std::string& file_or_dir) const;

	/// Return the path that is the specified number of levels up from
	/// the path. If the movement could not proceed due to being at the
	/// root or having an invalid argument, return the path that the
	/// movement ended on.
	std::string get_path_up(const std::string& path,
							const unsigned levels=1) const;


	/// Return true if the path is the root of the filesystem.
	bool is_root(const std::string& path) const;

	/// Show the files in the current directory.
	void display_current_files();

	/// Updated the locally maintained lists of files and directories in
	/// the current directory.
	void update_file_lists();

	/// Set the textbox to reflect the selected file.
	void entry_selected(const unsigned entry);

	std::string current_dir_;
	std::string chosen_file_;
	std::vector<std::string> files_in_current_dir_, dirs_in_current_dir_;
	int last_selection_;
	int type_a_head_;
};

} //end namespace gui

#endif // FILE_MENU_H_INCLUDED
