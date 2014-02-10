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

#include "global.hpp"

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "display.hpp"
#include "gettext.hpp"
#include "gui/dialogs/folder_create.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "filechooser.hpp"
#include "widgets/file_menu.hpp"


namespace dialogs
{

int show_file_chooser_dialog(display &disp, std::string &filename,
                             std::string const &title, bool show_directory_buttons,
							 const std::string& type_a_head,
							 int xloc, int yloc) {

	file_dialog d(disp, filename, title, "", show_directory_buttons);
	if (!type_a_head.empty())
		d.select_file(type_a_head);
	if(d.show(xloc, yloc) >= 0) {
		filename = d.get_choice();
	}
	return d.result();
}

int show_file_chooser_dialog_save(display &disp, std::string &filename,
                             std::string const &title,
                             const std::string& default_file_name,
                             bool show_directory_buttons,
							 const std::string& type_a_head,
							 int xloc, int yloc) {

	file_dialog d(disp, filename, title, default_file_name, show_directory_buttons);
    d.set_autocomplete(false);
	if (!type_a_head.empty())
		d.select_file(type_a_head);
	if(d.show(xloc, yloc) >= 0) {
		filename = d.get_choice();
	}
	return d.result();
}

file_dialog::file_dialog(display &disp, const std::string& file_path,
		const std::string& title, const std::string& default_file_name,
		bool show_directory_buttons) :
	gui::dialog(disp, title, file_path, gui::OK_CANCEL),
	show_directory_buttons_(show_directory_buttons),
	files_list_(NULL),
	last_selection_(-1),
	last_textbox_text_(),
	chosen_file_(".."),
    autocomplete_(true)
{
	files_list_ = new gui::file_menu(disp.video(), file_path);
	const unsigned file_list_height = (disp.h() / 2);
	const unsigned file_list_width = std::min<unsigned>(files_list_->width(), (disp.w() / 4));
	files_list_->set_measurements(file_list_width, file_list_height);
	files_list_->set_max_height(file_list_height);
	set_menu(files_list_);
	default_file_name_ = default_file_name;
	get_message().set_text(format_dirname(files_list_->get_directory()));
	set_textbox(_("File: "), format_filename(file_path), 100);
	if (show_directory_buttons_)
	{
		add_button( new gui::dialog_button(disp.video(), _("Delete File"),
					gui::button::TYPE_PRESS, gui::DELETE_ITEM), dialog::BUTTON_EXTRA);
		add_button( new gui::dialog_button(disp.video(), _("New Folder"),
					gui::button::TYPE_PRESS, gui::CREATE_ITEM), dialog::BUTTON_EXTRA_LEFT);
	}
}

gui::dialog::dimension_measurements file_dialog::layout(int xloc, int yloc)
{
	gui::dialog::dimension_measurements dim = dialog::layout(xloc, yloc);

	//shift the menu up
	unsigned y_shift = dim.menu_y - std::min<int>(dim.label_y, dim.textbox.y);
	int y_max = dim.menu_y + get_menu().height();
	dim.menu_y -= y_shift;

	//shift the extra buttons up
	if (show_directory_buttons_)
	{
		std::map<gui::dialog_button *, std::pair<int,int> >::iterator i;
		for(i = dim.buttons.begin(); i != dim.buttons.end(); ++i)
		{
			const int btn_h = i->first->height();
			int& btn_y = i->second.second;
			y_max = std::max<int>(y_max, btn_y + btn_h);
			btn_y -= y_shift;
		}
	}

	//shift the textbox down
	const int textbox_bottom_y = dim.textbox.y + get_textbox().height();
	const int label_bottom_y = dim.label_y + get_textbox().get_label()->height();
	y_shift = y_max - std::max<int>(textbox_bottom_y, label_bottom_y);
	dim.textbox.y += y_shift;
	dim.label_y += y_shift;

	set_layout(dim);
	return dim;
}

std::string file_dialog::unformat_filename(const std::string& filename) const
{
	return files_list_->add_path(files_list_->get_directory(), filename);
}

std::string file_dialog::format_filename(const std::string& filename) const
{
	if(files_list_->is_directory(filename)) {
		return default_file_name_;
	}
	std::string::size_type last_delim = filename.find_last_of(gui::file_menu::path_delim);
	if(last_delim != std::string::npos) {
		return filename.substr(last_delim + 1);
	}
	return filename;
}

void file_dialog::select_file(const std::string& file)
{
	chosen_file_ = file;
	get_textbox().set_text(format_filename(chosen_file_));
	files_list_->select_file(file);
}

std::string file_dialog::format_dirname(const std::string& dirname) const
{
	int menu_font_size = font::SIZE_NORMAL;
	std::string tmp = files_list_->strip_last_delim(dirname);

	// If the text get out of bounds, make it shorter;
	// Take the prefix of the dir (ie. /home/ or c:/) put three dot's behind it
	// and shorten the rest of the dir:
	// /home/.../rest_of_the_dir
	// Note that this is a dirty hack and fundamental changes in the widget subdir
	// needs to be made...
	if(font::line_width(tmp, menu_font_size) <= 390) {
		return tmp;
	}
	static const int filler_width = font::line_width("...", menu_font_size);

	// Find the first part of the dir
	std::string dir_prefix = "/";
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

void file_dialog::set_save_text(const std::string& filename)
{
	const std::string fn = format_filename(filename);
	const size_t filename_dot = fn.find_last_of('.');
	get_textbox().set_text(fn);
	get_textbox().set_selection(0, filename_dot != std::string::npos ? filename_dot : fn.length());
	get_textbox().set_cursor_pos(0);
}

void file_dialog::action(gui::dialog_process_info &dp_info)
{
	if(result() == gui::CLOSE_DIALOG)
		return;

	//handle "delete item" requests
	if(result() == gui::DELETE_ITEM)
	{
		if(!chosen_file_.empty())
		{
			if(files_list_->delete_chosen_file() == -1) {
				gui2::show_transient_error_message(get_display().video()
						, _("Deletion of the file failed."));
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
		std::string new_dir_name = "";
		if(gui2::tfolder_create::execute(new_dir_name, get_display().video()))
		{
			if( !files_list_->make_directory(new_dir_name) ) {
				gui2::show_transient_error_message(get_display().video()
						, _("Creation of the directory failed."));
			} else {
				dp_info.first_time = true;
			}
		}
		dp_info.clear_buttons();
		set_result(gui::CONTINUE_DIALOG);
	}

	//update the chosen file
	if((dp_info.selection != last_selection_
		|| dp_info.first_time
		|| dp_info.double_clicked)
			&& (!files_list_->type_a_head()
				|| dp_info.new_left_button))
	{
		files_list_->reset_type_a_head();

		chosen_file_ = files_list_->get_choice();
		set_save_text(chosen_file_);
		last_selection_ = (dp_info.double_clicked) ? -1 : dp_info.selection;
		last_textbox_text_ = textbox_text();
	}
	else if(textbox_text() != last_textbox_text_)
	{
		chosen_file_ = unformat_filename(textbox_text());
		last_textbox_text_ = textbox_text();

		// Do type-a-head search in listbox
        if (autocomplete_) {
		    files_list_->select_file(textbox_text());
        }
	}

	if(result() >=0) {
		//if a directory has been chosen, enter it
		if(files_list_->is_directory(chosen_file_))
		{
			files_list_->change_directory(chosen_file_);
			get_message().set_text(format_dirname(files_list_->get_directory()));

			//reset the chosen file
			chosen_file_ = "..";
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
