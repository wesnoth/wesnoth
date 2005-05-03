/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "../global.hpp"

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "../display.hpp"
#include "../events.hpp"
#include "../filesystem.hpp"
#include "../font.hpp"
#include "../gettext.hpp"
#include "../key.hpp"
#include "../show_dialog.hpp"
#include "../wml_separators.hpp"
#include "file_chooser.hpp"

#include <sstream>
#include <cstdio>

namespace {
	const std::string dir_picture = "misc/folder-icon.png";
}

namespace gui {

file_chooser::file_chooser(display &disp, std::string start_file) 
	: widget(disp.video()), delete_button_(disp.video(), _("Delete File")),
	  path_delim_('/'), current_dir_(get_path(start_file)),
	  chosen_file_(start_file), file_list_(disp.video(), files_in_current_dir_, false),
	  filename_textbox_(disp.video(), 100, start_file, true),
	  current_path_label_(disp.video(), current_dir_), choice_made_(false),
	  last_selection_(-1), disp_(disp)
{
	// If the start file is not a file or directory, use the root.
	if(!file_exists(chosen_file_) && !is_directory(chosen_file_)
	   || !is_directory(current_dir_)) {
		current_dir_ = path_delim_;
		chosen_file_ = current_dir_;
	}
	// FIXME: quick hack
	// on a high-res screen the initial max_items_onscreen is based
	// on .66 of y dimension, eg. 17 menu items, exceeding the
	// starting box which can only take 13 or so: force it to be smaller
	set_measurements(400, 384);
	update_file_lists();
	display_chosen_file();
}

void file_chooser::display_current_files() {
	std::vector<std::string> to_show;
	if (!is_root(current_dir_)) {
		to_show.push_back("..");
	}
	std::copy(dirs_in_current_dir_.begin(), dirs_in_current_dir_.end(),
			  std::back_inserter(to_show));
	std::vector<std::string>::iterator it;
	for (it = to_show.begin(); it != to_show.end(); it++) {
		// Add an image to show that these are directories.
		std::stringstream ss;
		ss << font::IMAGE << dir_picture << COLUMN_SEPARATOR << *it;
		*it = ss.str();
	}
	for (it = files_in_current_dir_.begin(); it != files_in_current_dir_.end(); it++) {
		const std::string display_string = COLUMN_SEPARATOR + *it;
		to_show.push_back(display_string);
	}
	const int menu_font_size = font::SIZE_NORMAL; // Known from menu.cpp.
	for (it = to_show.begin(); it != to_show.end(); it++) {
		// Make sure that all lines fit.
		// Guess the width of the scrollbar to be 30 since it is not accessible from here.
		// -25 to compensate for the picture column.
		while (font::line_width(*it, menu_font_size) > file_list_.width() - 30 - 25) {
			(*it).resize((*it).size() - 1);
		}
	}
	file_list_.set_items(to_show);
	current_path_label_.set_text(current_dir_);
}

void file_chooser::display_chosen_file() {
	if (is_directory(chosen_file_)) {
		filename_textbox_.set_text(strip_last_delim(chosen_file_) + path_delim_);
	}
	else {
		filename_textbox_.set_text(chosen_file_);
	}
}

void file_chooser::process_event() {
	CKey key;
	int mousex, mousey;
	SDL_GetMouseState(&mousex,&mousey);
	
	// The menu does not implement focus functionality, so we fake
	// it. We give the file list focus whenever the filename textbox
	// does not have focus. Inflexible but easy solution.
	SDL_Rect const &loc = location();
	if (!(mousex > loc.x && (unsigned)mousex < loc.x + loc.w && mousey > loc.y 
		&& (unsigned)mousey < loc.y + loc.h - filename_textbox_.height())) {
		// Hmm, as I understand it this should happen automatically when
		// the mouse is in the textbox again. However this is not the
		// case so this is done explicitly here.
		filename_textbox_.set_focus(true);
	}
	else {
		filename_textbox_.set_focus(false);
	}
	if (!filename_textbox_.focus()) {
		file_list_.process();
		const int new_selection = file_list_.selection();
		const bool double_click = file_list_.double_clicked();
		if (double_click && new_selection >= 0) {
			last_selection_ = new_selection;
			entry_chosen(new_selection);
		}
		if (new_selection >= 0 && last_selection_ != new_selection) {
			last_selection_ = new_selection;
			entry_selected(new_selection);
		}
	}
	if (delete_button_.pressed()) {
		const int ret = remove(get_current_file().c_str());
		if (ret == -1) {
			show_dialog(disp_, NULL, "", _("Deletion of the file failed."), OK_ONLY);
		}
		else {
			update_file_lists();
			chosen_file_ = current_dir_;
			display_chosen_file();
		}
	}
}

void file_chooser::entry_selected(const unsigned entry) {
	const int entry_index = entry - (is_root(current_dir_) ? 0 : 1);
	if (entry_index >= 0) {
		// Do not change the selection if the parent directory entry is selected.
		std::string selected;
		if ((unsigned)entry_index < dirs_in_current_dir_.size()) {
			const int dir_index = entry_index;
			selected = dirs_in_current_dir_[dir_index];
		}
		else {
			const int file_index = entry_index - dirs_in_current_dir_.size();
			if(file_index >= 0 && size_t(file_index) < files_in_current_dir_.size()) {
				selected = files_in_current_dir_[file_index];
			} else {
				return;
			}
		}
		chosen_file_ = add_path(current_dir_, selected);
		display_chosen_file();
	}
}

void file_chooser::entry_chosen(const unsigned entry) {
	const int entry_index = entry - (is_root(current_dir_) ? 0 : 1);
	if (entry_index == -1) {
		// Parent dir wanted.
		if (!is_root(current_dir_)) {
			current_dir_ = get_path_up(current_dir_);
			update_file_lists();
			chosen_file_ = current_dir_;
		}
		else {
			return;
		}
	}
	else {
		if ((unsigned)entry_index < dirs_in_current_dir_.size()) {
			// Descend into a directory.
			const int dir_index = entry_index;
			const std::string selected_dir = dirs_in_current_dir_[dir_index];
			current_dir_ = add_path(current_dir_, selected_dir);
			chosen_file_ = current_dir_;
			update_file_lists();
		}
		else {
			// Choose a file.
			const int file_index = entry_index - dirs_in_current_dir_.size();
			const std::string selected_file = files_in_current_dir_[file_index];
			chosen_file_ = add_path(current_dir_, selected_file);
			choice_made_ = true;
		}
	}
	display_chosen_file();
}

std::string file_chooser::get_current_file() const {
	const std::string textbox_contents = filename_textbox_.text();
	// If the contents of the textbox is not equal to the chosen file it
	// means that the textbox has been edited since the last selection,
	// and the textbox contents is wanted.
	if (textbox_contents != chosen_file_) {
		return textbox_contents;
	}
	return chosen_file_;
}
	

bool file_chooser::choice_made() const {
	return choice_made_;
}

std::string file_chooser::get_choice() const {
	return get_current_file();
}

void file_chooser::update_location(SDL_Rect const &rect) {
	const int file_list_y = rect.y + current_path_label_.height() + 10;
	const int filename_textbox_y = rect.y + rect.h - filename_textbox_.height();
	const int file_list_height = filename_textbox_y  - file_list_y - 10 - 32;

	const int delete_button_x = rect.x + rect.w - delete_button_.width();
	const int delete_button_y = file_list_y + file_list_height + 5;
	delete_button_.set_location(delete_button_x, delete_button_y);

	current_path_label_.set_width(rect.w);
	file_list_.set_width(rect.w);
	filename_textbox_.set_width(rect.w);

	current_path_label_.set_location(rect.x, rect.y);
	file_list_.set_location(rect.x, file_list_y);
	filename_textbox_.set_location(rect.x, filename_textbox_y);

	file_list_.set_max_height(file_list_height);
}

std::string file_chooser::get_path(const std::string file_or_dir) const {
	std::string res_path = file_or_dir;
	if (!is_directory(file_or_dir)) {
		size_t index = file_or_dir.find_last_of(path_delim_);
		if (index != std::string::npos) {
			res_path = file_or_dir.substr(0, index);
		}
	}
	return res_path;
}

std::string file_chooser::get_path_up(const std::string path, const unsigned levels) const {
	std::string curr_path = get_path(path);
	for (unsigned i = 0; i < levels; i++) {
		if (is_root(curr_path)) {
			break;
		}
		curr_path = strip_last_delim(curr_path);
		size_t index = curr_path.find_last_of(path_delim_);
		if (index != std::string::npos) {
			curr_path = curr_path.substr(0, index);
		}
		else {
			break;
		}
	}
	if (curr_path.size() == 0) {
		// The root was reached, represent this as one delimiter only.
		curr_path = path_delim_;
	}
	return curr_path;
}

std::string file_chooser::strip_last_delim(const std::string path) const {
	std::string res_string = path;
	if (path[path.size() - 1] == path_delim_) {
		res_string = path.substr(0, path.size() - 1);
	}
	return res_string;
}

bool file_chooser::is_root(const std::string path) const {
	return path.size() == 0 || path.size() == 1 && path[0] == path_delim_;
}

std::string file_chooser::add_path(const std::string path, const std::string to_add) {
	std::string joined_path = strip_last_delim(path);
	if (to_add.size() > 0) {
		if (to_add[0] == path_delim_) {
			joined_path += to_add;
		}
		else {
			joined_path += "/" + to_add;
		}
	}
	return joined_path;
}

void file_chooser::update_file_lists() {
	files_in_current_dir_.clear();
	dirs_in_current_dir_.clear();
	get_files_in_dir(current_dir_, &files_in_current_dir_,
	                 &dirs_in_current_dir_, FILE_NAME_ONLY);
	display_current_files();
}

void file_chooser::handle_event(const SDL_Event& event) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_RETURN) {
			if (filename_textbox_.focus()) {
				chosen_file_ = filename_textbox_.text();
				choice_made_ = true;
			}
			else {
				const int selected = file_list_.selection();
				if (selected >= 0) {
					entry_chosen(selected);
				}
			}
		}
	}
}

}
