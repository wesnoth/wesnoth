/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef FILE_CHOOSER_H_INCLUDED
#define FILE_CHOOSER_H_INCLUDED

class config;
class display;

#include "construct_dialog.hpp"

namespace gui
{
	class file_menu;
}

namespace dialogs
{

class file_dialog : public gui::dialog {
public:
	file_dialog(display &disp, const std::string& file_path, const std::string& title, const std::string& default_file_name, bool show_directory_buttons);

	virtual gui::dialog::dimension_measurements layout(int xloc=-1, int yloc=-1);

	/// Return the chosen file.
	std::string get_choice() const { return chosen_file_; }

	void select_file(const std::string& file);

    void set_autocomplete(bool value) { autocomplete_ = value; }

protected:
	void action(gui::dialog_process_info &dp_info);
	void set_save_text(const std::string& filename);
	std::string unformat_filename(const std::string& filename) const;
	std::string format_filename(const std::string& filename) const;
	std::string format_dirname(const std::string& dirname) const;

private:
	bool show_directory_buttons_;
	gui::file_menu *files_list_;
	int last_selection_;
	std::string last_textbox_text_;
	std::string chosen_file_;
	std::string default_file_name_;
    bool autocomplete_;
};

/// Show a dialog where the user can navigate through files and select a
/// file. The filename is used as a starting point in the navigation and
/// contains the chosen file when the function returns.  Return the
/// index of the button pressed, or -1 if the dialog was canceled
/// through keypress.
int show_file_chooser_dialog(display &displ, std::string &filename,
                             std::string const &title, bool show_directory_buttons = true,
							 const std::string& file_to_search = "",
							 int xloc = -1, int yloc = -1);

/// Show a filechooser dialog in a "save" mode, that is, without relying
/// on autocomplete to allow saving under any filename
int show_file_chooser_dialog_save(display &displ, std::string &filename,
                             std::string const &title,
                             const std::string& default_file_name = "",
                             bool show_directory_buttons = true,
							 const std::string& file_to_search = "",
							 int xloc = -1, int yloc = -1);
} // end of dialogs namespace

#endif
