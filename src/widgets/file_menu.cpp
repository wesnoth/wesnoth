/*
   Copyright (C) 2003 - 2015 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "filesystem.hpp"
#include "marked-up_text.hpp"
#include "wml_separators.hpp"
#include "widgets/file_menu.hpp"


namespace {
	std::vector<std::string> empty_string_vector;


	std::string uniform_path(const std::string& path)
	{
#ifdef _WIN32
		std::string res = path;
		std::replace(res.begin(), res.end(), '/', '\\');
		return res;
#else
		return path;
#endif
	}

}

namespace gui {

static const std::string dir_picture("misc/folder-icon.png");
static const std::string path_up("..");
#ifdef _WIN32
const char file_menu::path_delim('\\');
#else
const char file_menu::path_delim('/');
#endif

file_menu::file_menu(CVideo &disp, std::string start_file)
	: menu(disp, empty_string_vector, false),
	  current_dir_(uniform_path(get_path(start_file))),
	  chosen_file_(start_file), last_selection_(-1),
	  type_a_head_(-1)
{
	// If the start file is not a file or directory, use the root.
	if((!filesystem::file_exists(chosen_file_) && !filesystem::is_directory(chosen_file_))
		|| !filesystem::is_directory(current_dir_)) {
		current_dir_ = path_delim;
		chosen_file_ = current_dir_;
	}
	// FIXME: quick hack
	// on a high-res screen the initial max_items_onscreen is based
	// on .66 of y dimension, eg. 17 menu items, exceeding the
	// starting box which can only take 13 or so: force it to be smaller
//	set_measurements(400, 384);
	update_file_lists();
}

void file_menu::display_current_files() {
	std::vector<std::string> to_show;
	if (!is_root(current_dir_)) {
		to_show.push_back(path_up);
	}
	std::vector<std::string>::iterator it;
	for (it = dirs_in_current_dir_.begin(); it != dirs_in_current_dir_.end(); ++it) {
		// Add an image to show that these are directories.
		std::stringstream ss;
		ss << font::IMAGE << dir_picture << COLUMN_SEPARATOR << font::NULL_MARKUP << *it;
		to_show.push_back(ss.str());
	}
	for (it = files_in_current_dir_.begin(); it != files_in_current_dir_.end(); ++it) {
		const std::string display_string = COLUMN_SEPARATOR + std::string(1, font::NULL_MARKUP) + *it;
		to_show.push_back(display_string);
	}
	const int menu_font_size = font::SIZE_NORMAL; // Known from menu.cpp.
	for (it = to_show.begin(); it != to_show.end(); ++it) {
		// Make sure that all lines fit.
		// Guess the width of the scrollbar to be 30 since it is not accessible from here.
		// -25 to compensate for the picture column.
		while(static_cast<unsigned int>(
				font::line_width(*it, menu_font_size)) > width() - 30 - 25) {

			(*it).resize((*it).size() - 1);
		}
	}
	set_items(to_show);
}

int file_menu::delete_chosen_file() {
	const int ret = filesystem::delete_file(chosen_file_);
	if (ret == -1) {
	//	gui2::show_transient_message(disp_.video(), "", _("Deletion of the file failed."));
	}
	else {
		last_selection_ = -1;
		update_file_lists();
		chosen_file_ = current_dir_;
	}
	return ret;
}

bool file_menu::make_directory(const std::string& subdir_name) {
	bool ret = filesystem::make_directory(add_path(current_dir_, subdir_name));
	if (ret == false) {
	//	gui2::show_transient_message(disp_.video(), "", _("Creation of the directory failed."));
	}
	else {
		last_selection_ = -1;
		update_file_lists();
		chosen_file_ = current_dir_;
	}
	return ret;
}

void file_menu::handle_event(const SDL_Event& event) {
	menu::handle_event(event);

	if(selection() != last_selection_
			&& !type_a_head()) {
		type_a_head_ = -1;
		entry_selected(selection());
		last_selection_ = selection();
	}
}

void file_menu::entry_selected(const unsigned entry) {
	const int entry_index = entry - (is_root(current_dir_) ? 0 : 1);
	if (entry_index >= 0) {
		std::string selected;
		if(static_cast<unsigned>(entry_index) < dirs_in_current_dir_.size()) {
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
	} else {
		chosen_file_ = path_up;
	}
}

bool file_menu::is_directory(const std::string& fname) const {
	if(fname == path_up)
		return true;
	return filesystem::is_directory(fname);
}

void file_menu::change_directory(const std::string& path) {
	if(path == path_up)
	{
		// Parent dir wanted.
		if (!is_root(current_dir_)) {
			current_dir_ = get_path_up(current_dir_);
			last_selection_ = -1;
			update_file_lists();
			chosen_file_ = current_dir_;
		}
		else {
			return;
		}

	} else {
		current_dir_ = uniform_path(path);
		chosen_file_ = current_dir_;
		last_selection_ = -1;
		update_file_lists();
	}
}

std::string file_menu::get_choice() const {
	return chosen_file_;
}


std::string file_menu::get_path(const std::string& file_or_dir) const {
	std::string res_path = file_or_dir;
	if (!filesystem::is_directory(file_or_dir)) {
		size_t index = file_or_dir.find_last_of(path_delim);
		if (index != std::string::npos) {
			res_path = file_or_dir.substr(0, index);
		}
	}
	return res_path;
}

std::string file_menu::get_path_up(const std::string& path, const unsigned levels) const {
	std::string curr_path = get_path(path);
	for (unsigned i = 0; i < levels; i++) {
		if (is_root(curr_path)) {
			break;
		}
		curr_path = strip_last_delim(curr_path);
		size_t index = curr_path.find_last_of(path_delim);
		if (index != std::string::npos) {
			curr_path = curr_path.substr(0, index);
		}
		else {
#ifdef __AMIGAOS4__
			index = curr_path.find_last_of(':');
			if (index != std::string::npos) index++;
#endif
			break;
		}
	}
	if (curr_path.empty()) {
		// The root was reached, represent this as one delimiter only.
		curr_path = path_delim;
	}
#ifdef _WIN32
	if (curr_path.size() == 2 && curr_path[1] == ':') curr_path += path_delim;
#endif
	return curr_path;
}

std::string file_menu::strip_last_delim(const std::string& path) const {
	std::string res_string = path;
	if (path[path.size() - 1] == path_delim) {
		res_string = path.substr(0, path.size() - 1);
	}
	return res_string;
}

bool file_menu::is_root(const std::string& path) const {
#ifdef __AMIGAOS4__
	return path.empty() || path[path.size()-1] == ':';
#else
	return path.empty() || (path.size() == 1 && path[0] == path_delim);
#endif
}

std::string file_menu::add_path(const std::string& path, const std::string& to_add) const
{
	std::string joined_path = strip_last_delim(path);
	if (!to_add.empty()) {
		if (to_add == path_up) {
			return get_path_up(path);
		}
#ifdef __AMIGAOS4__
		else if (joined_path.empty() || joined_path[joined_path.size()-1] == ':') {
			if (to_add[0] == path_delim)
				joined_path += to_add.substr(1);
			else
				joined_path += to_add;
		}
#endif
#ifdef _WIN32
		else if (to_add.size() > 1 && to_add[1] == ':') {
			joined_path = to_add;
		}
#else
		else if (to_add[0] == path_delim) {
			joined_path = to_add;
		}
#endif
		else {
			joined_path += "/" + to_add;
		}
	}
	return joined_path;
}

struct match_begin {
	match_begin(const std::string& begin) : begin_(begin)
	{}

	bool operator()(const std::string& o) const
	{
		return o.compare(0, begin_.size(), begin_) == 0;
	}

	private:
	const std::string& begin_;
};

bool file_menu::type_a_head() const
{
	return selection() == type_a_head_;
}

void file_menu::reset_type_a_head()
{
	if (type_a_head_ >= 0)
	{
		entry_selected(type_a_head_);
		last_selection_ = type_a_head_;
	}
	type_a_head_ = -1;
}

void file_menu::select_file(const std::string& begin_of_filename)
{
	size_t additional_index = is_root(current_dir_) ? 0 : 1;
	std::vector<std::string>::iterator it;
	it = std::find_if(dirs_in_current_dir_.begin(), dirs_in_current_dir_.end(), match_begin(begin_of_filename));
	if (it != dirs_in_current_dir_.end())
	{
		type_a_head_ = additional_index + it - dirs_in_current_dir_.begin();
		move_selection(type_a_head_);
		return;
	}
	additional_index += dirs_in_current_dir_.size();

	it = std::find_if(files_in_current_dir_.begin(), files_in_current_dir_.end(), match_begin(begin_of_filename));
	if (it != files_in_current_dir_.end())
	{
		type_a_head_ = it - files_in_current_dir_.begin() + additional_index;
		move_selection(type_a_head_);
		return;
    }
}

void file_menu::update_file_lists() {
	files_in_current_dir_.clear();
	dirs_in_current_dir_.clear();
	filesystem::get_files_in_dir(current_dir_, &files_in_current_dir_,
	                 &dirs_in_current_dir_, filesystem::FILE_NAME_ONLY);
	display_current_files();
}

}
