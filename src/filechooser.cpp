/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "display.hpp"
#include "gettext.hpp"
#include "file_chooser.hpp"
#include "video.hpp"
#include "widgets/file_menu.hpp"
#include "filesystem.hpp"

#include <vector>
#include <string>

namespace dialogs
{

int show_file_chooser_dialog(display &disp, std::string &filename,
                             std::string const &title, int xloc, int yloc) {

	file_dialog d(disp, filename, title);
	if(d.show(xloc, yloc) >= 0) {
		filename = d.get_choice();
	}
	return d.result();
}

file_dialog::file_dialog(display &disp, const std::string& file_path, const std::string& title)
: gui::dialog(disp, title, file_path, gui::OK_CANCEL), files_list_(NULL), last_selection_(0)
{
	files_list_ = new gui::file_menu(disp.video(), file_path);
	const unsigned file_list_height = (disp.h() / 2);
	const unsigned file_list_width = minimum<unsigned>(files_list_->width(), (disp.w() / 4));
	files_list_->set_measurements(file_list_width, file_list_height);
	files_list_->set_max_height(file_list_height);
	set_menu(files_list_);
	get_message().set_text(format_dirname(files_list_->get_directory()));
	set_textbox(_("File: "), format_filename(file_path), 100);
	add_button( new gui::dialog_button(disp.video(), _("Delete File"),
		gui::button::TYPE_PRESS, gui::DELETE_ITEM), dialog::BUTTON_EXTRA);
	add_button( new gui::dialog_button(disp.video(), _("New Folder"),
		gui::button::TYPE_PRESS, gui::CREATE_ITEM), dialog::BUTTON_EXTRA_LEFT);
}

gui::dialog::dimension_measurements file_dialog::layout(int xloc, int yloc)
{
	gui::dialog::dimension_measurements dim = dialog::layout(xloc, yloc);

	//shift the menu up
	unsigned y_shift = dim.menu_y - minimum<int>(dim.label_y, dim.textbox.y);
	int y_max = dim.menu_y + get_menu().height();
	dim.menu_y -= y_shift;

	//shift the extra buttons up
	std::map<gui::dialog_button *const, std::pair<int,int> >::iterator i;
	for(i = dim.buttons.begin(); i != dim.buttons.end(); ++i)
	{
		const int btn_h = i->first->height();
		int& btn_y = i->second.second;
		y_max = maximum<int>(y_max, btn_y + btn_h);
		btn_y -= y_shift;
	}

	//shift the textbox down
	const int textbox_bottom_y = dim.textbox.y + get_textbox().height();
	const int label_bottom_y = dim.label_y + get_textbox().get_label()->height();
	y_shift = y_max - maximum<int>(textbox_bottom_y, label_bottom_y);
	dim.textbox.y += y_shift;
	dim.label_y += y_shift;

	set_layout(dim);
	return dim;
}

const std::string file_dialog::unformat_filename(const std::string& filename) const
{
	return files_list_->add_path(files_list_->get_directory(), filename);
}

const std::string file_dialog::format_filename(const std::string& filename) const
{
	if(files_list_->is_directory(filename)) {
		return "";
	}
	std::string::size_type last_delim = filename.find_last_of(gui::file_menu::path_delim);
	if(last_delim != std::string::npos) {
		return filename.substr(last_delim + 1);
	}
	return filename;
}

const std::string file_dialog::format_dirname(const std::string& dirname) const
{
	int menu_font_size = font::SIZE_NORMAL;
	std::string tmp = files_list_->strip_last_delim(dirname);

	// If the text get out of bounds, make it shorter;
	// Take the prefix of the dir (ie. /home/ or c:/) put three dot's behind it
	// and shorten the rest of the dir:
	// /home/.../rest_of_the_dir
	// Note that this is a dirty hack and fundemental changes in the widget subdir
	// needs to be made...
	if(font::line_width(tmp, menu_font_size) <= 390) {
		return tmp;
	}
	static const int filler_width = font::line_width("...", menu_font_size);

	// Find the first part of the dir
	std::string dir_prefix = "";
	std::string::size_type pos_first = 0;
	if((pos_first = tmp.find_first_of("\\/", 1)) != std::string::npos)
	{
		dir_prefix = tmp.substr(0, pos_first) + "/...";
		tmp = tmp.substr(pos_first);
	}

	static const int prefix_width = font::line_width(dir_prefix, menu_font_size);

	// Try to cut off text at the '/' or '\' tokens
	while(font::line_width(tmp, menu_font_size) + filler_width + prefix_width > 390 && tmp.length() != 0)
	{
		std::string::size_type pos;
		if((pos = tmp.find_first_of("\\/", 1)) != std::string::npos)
			tmp = tmp.substr(pos, tmp.length());
		else
			tmp = tmp.substr(1, tmp.length());
	}

	return dir_prefix + tmp;
}


void file_dialog::action(gui::dialog_process_info &dp_info) {
	if(result() == gui::CLOSE_DIALOG)
		return;

	//handle "delete item" requests
	if(result() == gui::DELETE_ITEM)
	{
		if(!chosen_file_.empty())
		{
			if(files_list_->delete_chosen_file() == -1) {
				gui::message_dialog d(get_display(), _("Deletion of the file failed."));
				d.show();
				dp_info.clear_buttons();
			} else {
				dp_info.first_time = true;
			}
		}
		set_result(gui::CONTINUE_DIALOG);
	}
	//handle "create item" requests
	else if(result() == gui::CREATE_ITEM)
	{
		gui::dialog d(get_display(), _("New Folder"), "", gui::OK_CANCEL);
		d.set_textbox(_("Name: "));
		d.show();
		if(d.result() != gui::CLOSE_DIALOG && !d.textbox_text().empty())
		{
			if( !files_list_->make_directory(d.textbox_text()) ) {
				gui::message_dialog d2(get_display(), _("Creation of the directory failed."));
				d2.show();
			} else {
				dp_info.first_time = true;
			}
		}
		dp_info.clear_buttons();
		set_result(gui::CONTINUE_DIALOG);
	}

	//update the chosen file
	if(dp_info.selection != last_selection_
		|| dp_info.first_time
		|| dp_info.double_clicked)
	{
		chosen_file_ = files_list_->get_choice();
		get_textbox().set_text(format_filename(chosen_file_));
		last_selection_ = (dp_info.double_clicked) ? -1 : dp_info.selection;
	}
	else if(textbox_text() != last_textbox_text_)
	{
		chosen_file_ = unformat_filename(textbox_text());
		last_textbox_text_ = textbox_text();
	}

	if(result() >=0) {
		//if a directory has been chosen, enter it
		if(files_list_->is_directory(chosen_file_))
		{
			files_list_->change_directory(chosen_file_);
			get_message().set_text(format_dirname(files_list_->get_directory()));

			//reset the chosen file
			chosen_file_ = files_list_->get_choice();
			get_textbox().set_text(format_filename(chosen_file_));
			set_result(gui::CONTINUE_DIALOG);
		}
		//if a file has been chosen, return button index "Ok"
		else
		{
			set_result(0);
		}
	}
}

} //end namespace dialogs
