/*
   Copyright (C) 2003 by David White <davidnwhite@comcast.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef FILE_CHOOSER_H_INCLUDED
#define FILE_CHOOSER_H_INCLUDED

#include "label.hpp"
#include "menu.hpp"
#include "textbox.hpp"
#include "widget.hpp"

class display;

namespace gui {

/// A widget where the user may navigate through directories and choose
/// a file.
class file_chooser : public gui::widget {
public:
	/// Initialize the file chooser. start_file is the file that will be
	/// selected when the dialog starts. The current directory will be
	/// the one the file is in.
	file_chooser(display &disp, std::string start_file="");

	/// Return true if the user is done making her choice.
	bool choice_made() const;

	/// Return the choosen file.
	std::string get_choice() const;
protected:
	virtual void update_location(SDL_Rect const &rect);
	virtual void handle_event(const SDL_Event& event);
	virtual void process_event();

private:
	/// If file_or_dir is a file, return the directory the file is in,
	/// if it is a directory, return the directory name. If no path
	/// delimiters could be found, return the unchanged argument.
	std::string get_path(const std::string file_or_dir) const;

	/// Return the path that is the specified number of levels up from
	/// the path. If the movement could not proceed due to being at the
	/// root or having an invalid argument, return the path that the
	/// movement ended on.
	std::string get_path_up(const std::string path,
							const unsigned levels=1) const;

	/// Return path with to_add added, using a path delimiter between them.
	std::string add_path(const std::string path, const std::string to_add);

	/// Return the string with the last path delimiter removed, if one
	/// was there.
	std::string strip_last_delim(const std::string path) const;

	/// Return true if the path is the root of the filesystem.
	bool is_root(const std::string path) const;

	/// Show the files in the current directory.
	void display_current_files();

	/// Display the currently selected file in the file text box.
	void display_chosen_file();

	/// Updated the locally maintained lists of files and directories in
	/// the current directory.
	void update_file_lists();

	/// Set the textbox to reflect the selected file.
	void entry_selected(const unsigned entry);

	/// Enter the directory or choose the file.
	void entry_chosen(const unsigned entry);

	/// Return the filename of either the currently selected file in the
	/// file list, or the filename in the textbox, depending on which is
	/// most recently updated.
	std::string get_current_file() const;

	button delete_button_;
	const char path_delim_;
	std::string current_dir_;
	std::string chosen_file_;
	std::vector<std::string> files_in_current_dir_, dirs_in_current_dir_;
	menu file_list_;
	textbox filename_textbox_;
	label current_path_label_;
	bool choice_made_;
	int last_selection_;
	display& disp_;
};

}

#endif // FILE_CHOOSER_H_INCLUDED
