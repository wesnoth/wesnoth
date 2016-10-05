/*
   Copyright (C) 2011, 2016 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/file_dialog.hpp"

#include "cursor.hpp"
#include "filesystem.hpp"
#include "formula/string_utils.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/folder_create.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/button.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
	#include "gui/widgets/list.hpp"
#else
	#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "gui/widgets/window.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "serialization/unicode.hpp"

#include <functional>

static lg::log_domain log_filedlg{"gui/dialogs/file_dialog"};
#define ERR_FILEDLG   LOG_STREAM(err,   log_filedlg)
#define WRN_FILEDLG   LOG_STREAM(warn,  log_filedlg)
#define LOG_FILEDLG   LOG_STREAM(info,  log_filedlg)
#define DBG_FILEDLG   LOG_STREAM(debug, log_filedlg)

namespace fs = filesystem;

namespace
{
const std::string icon_dir = "misc/folder-icon.png";
const std::string icon_file = "";
const std::string icon_parent = "";
// NOTE: Does not need to be the same as PARENT_DIR! Use PARENT_DIR to build
//       relative paths for non-presentational purposes instead.
const std::string label_parent = "..";

const std::string CURRENT_DIR = ".";
const std::string PARENT_DIR = "..";

const int FILE_DIALOG_ITEM_RETVAL = 9001;
const int FILE_DIALOG_MAX_ENTRY_LENGTH = 42;

inline std::string concat_path(const std::string& a, const std::string& b)
{
	//
	// As of Boost 1.61, normalize_path() displays unusual behavior when passing
	// it paths with extra path separators (e.g. //opt becomes
	// //opt/home/shadowm/src/wesnoth, where the extraneous sequence is probably
	// the current working dir), so avoid leaving those around.
	//
	// TODO: Maybe handle this corner case in filesystem::normalize_path()
	//       instead, really.
	//
	if((a.empty() || !fs::is_path_sep(a.back())) && (b.empty() || !fs::is_path_sep(b.front()))) {
		return a + fs::path_separator() + b;
	} else {
		return a + b;
	}
}

inline std::string filesystem_root()
{
	// TODO: Multiple drives support (may require cooperation from the caller).
	return std::string(1, fs::path_separator());
}

inline void isort_dir_entries(std::vector<std::string>& entries)
{
	// Yes, this uses Wesnoth's locale and not the filesystem/OS locale. Yes, this
	// isn't ideal. No, we don't really need to worry about it. It's just a
	// cosmetic procedure anyway.
	std::sort(entries.begin(), entries.end(),
			  [](const std::string& a, const std::string& b) { return translation::icompare(a, b) < 0; });
}

} // unnamed namespace

namespace gui2
{

REGISTER_DIALOG(file_dialog)

tfile_dialog::tfile_dialog()
	: title_(_("Find File"))
	, msg_()
	, ok_label_()
	, extension_()
	, current_entry_()
	, current_dir_()
	, read_only_(false)
	, save_mode_(false)
	, dir_files_()
	, dir_subdirs_()
{
	set_restore(true);
}

std::string tfile_dialog::path() const
{
	const std::string& dir_norm = fs::normalize_path(current_dir_, true);

	if(current_entry_.empty() || current_entry_ == CURRENT_DIR) {
		return dir_norm;
	} else if(current_entry_ == PARENT_DIR) {
		return fs::directory_name(dir_norm);
	}

	return concat_path(dir_norm, current_entry_);
}

tfile_dialog& tfile_dialog::set_path(const std::string& value)
{
	if(value.empty()) {
		current_dir_ = filesystem_root();
	}

	const std::string& norm = fs::normalize_path(value, true);

	if(fs::is_directory(norm)) {
		current_dir_ = norm;
	} else {
		current_dir_ = fs::nearest_extant_parent(norm);
		if(current_dir_.empty()) {
			current_dir_ = filesystem_root();
		}
		// The file may or may not exist. We'll find out eventually when setting up
		// the dialog.
		current_entry_ = fs::base_name(norm);
	}

	return *this;
}

tfile_dialog& tfile_dialog::set_filename(const std::string& value)
{
	current_entry_ = value;

	return *this;
}

void tfile_dialog::pre_show(twindow& window)
{
	tcontrol& title = find_widget<tcontrol>(&window, "title", false);
	tcontrol& message = find_widget<tcontrol>(&window, "message", false);
	tcontrol& ok = find_widget<tcontrol>(&window, "ok", false);

	title.set_label(title_);

	if(msg_.empty()) {
		message.set_visible(gui2::twidget::tvisible::invisible);
	} else {
		message.set_label(msg_);
		message.set_use_markup(true);
	}

	if(ok_label_.empty()) {
		ok.set_label(save_mode_ ? _("Save") : _("Open"));
	} else {
		ok.set_label(ok_label_);
	}

	tlistbox& filelist = find_widget<tlistbox>(&window, "filelist", false);

#ifdef GUI2_EXPERIMENTAL_LISTBOX
	connect_signal_notify_modified(filelist, std::bind(
				  &tfile_dialog::on_row_selected
				, *this
				, std::ref(window)));
#else
	filelist.set_callback_value_change(
			dialog_callback<tfile_dialog, &tfile_dialog::on_row_selected>);
#endif

	tbutton& mkdir_button = find_widget<tbutton>(&window, "new_dir", false);
	tbutton& rm_button = find_widget<tbutton>(&window, "delete_file", false);

	connect_signal_mouse_left_click(mkdir_button,
			std::bind(&tfile_dialog::on_dir_create_cmd, this, std::ref(window)));
	connect_signal_mouse_left_click(rm_button,
			std::bind(&tfile_dialog::on_file_delete_cmd, this, std::ref(window)));

	if(read_only_) {
		mkdir_button.set_active(false);
		rm_button.set_active(false);

		mkdir_button.set_visible(twidget::tvisible::invisible);
		rm_button.set_visible(twidget::tvisible::invisible);
	}

	refresh_fileview(window);

	window.keyboard_capture(&find_widget<ttext_box>(&window, "filename", false));
	window.add_to_keyboard_chain(&filelist);
	window.set_exit_hook(std::bind(&tfile_dialog::on_exit, this, std::ref(window)));
}

bool tfile_dialog::on_exit(twindow& window)
{
	if(window.get_retval() == FILE_DIALOG_ITEM_RETVAL) {
		// Attempting to exit by double clicking items -- only proceeds if the item
		// was a file.
		if(process_fileview_submit(window)) {
			window.set_retval(twindow::OK, false);
			return true;
		} else {
			return false;
		}
	}


	if(window.get_retval() == twindow::OK) {
		// Attempting to exit by pressing Enter/clicking OK -- only proceeds if the
		// textbox was not altered by the user to point to a different directory.
		return process_textbox_submit(window);
	}

	return true;
}

bool tfile_dialog::is_selection_type_acceptable(tfile_dialog::SELECTION_TYPE stype) const
{
	// TODO: Adapt for implementing directory selection mode.
	return save_mode_
			? stype != SELECTION_IS_DIR && stype != SELECTION_PARENT_NOT_FOUND
			: stype == SELECTION_IS_FILE;
}

bool tfile_dialog::process_submit_common(twindow& window, const std::string& name)
{
	const auto stype = register_new_selection(name);

	//DBG_FILEDLG << "current_dir_=" << current_dir_ << "  current_entry_=" << current_entry_ << '\n';

	if(is_selection_type_acceptable(stype)) {
		return true;
	}

	switch(stype) {
		case SELECTION_IS_DIR:
			// TODO: Adapt for implementing directory selection mode.
			refresh_fileview(window);
			break;
		case SELECTION_PARENT_NOT_FOUND:
			// We get here in save mode or not. Use the file creation language only in
			// save mode.
			if(save_mode_) {
				show_transient_error_message(window.video(), vgettext("The file or folder $path cannot be created.", {{"path", name}}));
				break;
			}
		case SELECTION_NOT_FOUND:
			// We only get here if we aren't in save mode.
			show_transient_error_message(window.video(), vgettext("The file or folder $path does not exist.", {{"path", name}}));
			break;
		case SELECTION_IS_FILE:
			// TODO: Adapt for implementing directory selection mode.
		default:
			assert(false && "Unimplemented selection mode or semantics");
	}

	return false;
}

bool tfile_dialog::process_fileview_submit(twindow& window)
{
	tlistbox& filelist = find_widget<tlistbox>(&window, "filelist", false);
	const std::string& selected_name = get_filelist_selection(filelist);
	return process_submit_common(window, selected_name);
}

bool tfile_dialog::process_textbox_submit(twindow& window)
{
	ttext_box& file_textbox = find_widget<ttext_box>(&window, "filename", false);
	const std::string& input_name = file_textbox.get_value();
	return !input_name.empty() && process_submit_common(window, input_name);
}

std::string tfile_dialog::get_filelist_selection(tlistbox& filelist)
{
	const int row = filelist.get_selected_row();

	if(row == -1) {
		// Shouldn't happen...
		return "";
	}

	const bool i_am_root = fs::is_root(current_dir_);

	if(row == 0 && !i_am_root) {
		return PARENT_DIR;
	} else {
		size_t n = i_am_root ? row : row - 1;

		if(n < dir_subdirs_.size()) {
			return dir_subdirs_[n];
		} else {
			n -= dir_subdirs_.size();

			if(n < dir_files_.size()) {
				return dir_files_[n];
			} else {
				assert(false && "File list selection is out of range!");
			}
		}
	}

	return "";
}

tfile_dialog::SELECTION_TYPE tfile_dialog::register_new_selection(const std::string& name)
{
	std::string new_path, new_parent;

	if(fs::is_relative(name)) {
		// On Windows, \ represents a path relative to the root of the process'
		// current working drive specified by the current working dir, so we get
		// here. This makes it the only platform where is_relative() and is_root()
		// aren't mutually exclusive.
		if(fs::is_root(name)) {
			DBG_FILEDLG << "register_new_selection(): new selection '" << name << "' is relative to a root resource\n";
			// Using the browsed dir's root drive instead of the cwd's makes the most
			// sense for users.
			new_parent = fs::root_name(current_dir_);
			new_path = fs::normalize_path(concat_path(new_parent, name), true, true);
		} else {
			DBG_FILEDLG << "register_new_selection(): new selection '" << name << "' seems relative\n";
			new_parent = current_dir_;
			new_path = fs::normalize_path(concat_path(current_dir_, name), true, true);
		}
	} else {
		DBG_FILEDLG << "register_new_selection(): new selection '" << name << "' seems absolute\n";
		new_parent = fs::directory_name(name);
		new_path = fs::normalize_path(name, true, true);
		DBG_FILEDLG << "register_new_selection(): new selection is " << new_path << '\n';
	}

	if(!new_path.empty()) {
		if(fs::is_directory(new_path)) {
			DBG_FILEDLG << "register_new_selection(): new selection '" << name << "' is a directory: " << new_path << '\n';
			current_dir_ = new_path;
			current_entry_.clear();
			return SELECTION_IS_DIR;
		} else if(fs::file_exists(new_path)) {
			// FIXME: Perhaps redundant since the three-params call to normalize_path()
			//        above necessarily validates existence.
			DBG_FILEDLG << "register_new_selection(): new selection '" << name << "' is a file, symbolic link, or special: " << new_path << '\n';
			current_dir_ = fs::directory_name(new_path);
			current_entry_ = fs::base_name(new_path);
			return SELECTION_IS_FILE;
		}
	}

	// The path does not exist, at least not entirely. See if the parent does
	// (in save mode non-existent files are accepted as long as the parent dir
	// exists).
	const std::string& absolute_parent = fs::normalize_path(new_parent, true, true);
	if(!absolute_parent.empty()) {
		DBG_FILEDLG << "register_new_selection(): new selection '" << name << "' does not exist or is not accessible, but parent exists\n";
		current_dir_ = absolute_parent;
		current_entry_ = fs::base_name(name);
		return SELECTION_NOT_FOUND;
	}

	DBG_FILEDLG << "register_new_selection(): new selection '" << name << "' does not exist or is not accessible\n";
	return SELECTION_PARENT_NOT_FOUND;
}

void tfile_dialog::set_input_text(ttext_box& t, const std::string& value)
{
	if(value.empty()) {
		clear_input_text(t);
		return;
	}

	t.set_value(value);

	const size_t vallen = t.get_length();
	const size_t extlen = utf8::size(extension_);

	if(save_mode_ && extlen && vallen > extlen) {
		// Highlight everything but the extension if it matches
		if(value.substr(vallen - extlen) == extension_) {
			t.set_selection(0, vallen - extlen);
		}
	}
}

void tfile_dialog::clear_input_text(ttext_box& t)
{
	if(save_mode_ && !extension_.empty()) {
		t.set_value(extension_);
		t.set_selection(0, 0);
	} else {
		t.clear();
	}
}

void tfile_dialog::refresh_fileview(twindow& window)
{
	cursor::setter cur{cursor::WAIT};

	dir_files_.clear();
	dir_subdirs_.clear();

	// TODO: Need to detect and handle cases where we don't have search permission
	//       on current_dir_, otherwise things may get weird.
	filesystem::get_files_in_dir(current_dir_, &dir_files_, &dir_subdirs_, filesystem::FILE_NAME_ONLY);
	isort_dir_entries(dir_files_);
	isort_dir_entries(dir_subdirs_);

	//
	// Clear and refill the filelist box.
	//

	tlistbox& filelist = find_widget<tlistbox>(&window, "filelist", false);

	filelist.clear();

	// Parent entry
	if(!fs::is_root(current_dir_)) {
		// label_parent may not necessarily be always ".." in the future, so push
		// with check_selection = false and check the selection ourselves here.
		push_fileview_row(filelist, label_parent, icon_parent, false);
		if(current_entry_ == PARENT_DIR || current_entry_.empty()) {
			filelist.select_row(0, true);
		}
	}

	for(const auto& dir : dir_subdirs_) {
		push_fileview_row(filelist, dir, icon_dir);
	}

	for(const auto& file : dir_files_) {
		push_fileview_row(filelist, file, icon_file);
	}

	find_widget<tcontrol>(&window, "current_dir", false).set_label(current_dir_);
	set_input_text(find_widget<ttext_box>(&window, "filename", false), current_entry_);
}

void tfile_dialog::push_fileview_row(tlistbox& filelist, const std::string& name, const std::string& icon, bool check_selection)
{
	// TODO: Hopefully some day GUI2 will allow us to make labels be ellipsized
	//       dynamically at layout/rendering time.
	std::string label = name;
	utils::ellipsis_truncate(label, FILE_DIALOG_MAX_ENTRY_LENGTH);

	std::map<std::string, string_map> data;
	data["icon"]["label"] = icon;
	data["file"]["label"] = label;

	tgrid& last_grid = filelist.add_row(data);
	const unsigned last_pos = filelist.get_item_count() - 1;

	//
	// Crummy hack around the lack of an option to hook into row double click
	// events for all rows using the GUI2 listbox API. Assign a special retval to
	// each row that triggers a special check during dialog exit.
	//
	find_widget<ttoggle_panel>(&last_grid, "item_panel", false)
			.set_retval(FILE_DIALOG_ITEM_RETVAL);

	if(check_selection && name == current_entry_) {
		filelist.select_row(last_pos, true);
	}
}

void tfile_dialog::on_row_selected(twindow& window)
{
	tlistbox& filelist = find_widget<tlistbox>(&window, "filelist", false);
	ttext_box& file_textbox = find_widget<ttext_box>(&window, "filename", false);

	// Don't use register_new_selection() here, we don't want any parsing to be
	// performed at this point.
	current_entry_ = get_filelist_selection(filelist);

	// Clear the textbox when selecting ..
	if(current_entry_ != PARENT_DIR) {
		set_input_text(file_textbox, current_entry_);
	} else {
		clear_input_text(file_textbox);
	}

	// Need to do this every time so that input can still be sent to the
	// textbox without clicking on it.
	window.keyboard_capture(&file_textbox);
}

void tfile_dialog::on_dir_create_cmd(twindow& window)
{
	std::string new_dir_name;

	if(tfolder_create::execute(new_dir_name, window.video())) {
		const std::string& new_path = concat_path(current_dir_, new_dir_name);

		if(!fs::make_directory(new_path)) {
			show_transient_error_message(window.video(),
					vgettext("Could not create a new folder at $path|. Make sure you have the appropriate permissions to write to this location.",
					{{"path", new_path}}));
		} else {
			refresh_fileview(window);
		}
	}
}

void tfile_dialog::on_file_delete_cmd(twindow& window)
{
	if(current_entry_.empty()) {
		return;
	}

	const std::string& selection = concat_path(current_dir_, current_entry_);
	const bool is_dir = fs::is_directory(selection);

	const std::string& message = (is_dir
			? _("The following folder and its contents will be permanently deleted:")
			: _("The following file will be permanently deleted:"))
			+ "\n\n" + selection + "\n\n" + _("Do you wish to continue?");

	if(gui2::show_message(window.video(), _("Confirm"), message, gui2::tmessage::yes_no_buttons) == gui2::twindow::CANCEL) {
		return;
	}

	const bool result = is_dir
			? fs::delete_directory(selection)
			: fs::delete_file(selection);

	if(!result) {
		show_transient_error_message(window.video(),
				vgettext("Could not delete $path|. Make sure you have the appropriate permissions to write to this location.",
						 {{"path", selection}}));
	} else {
		refresh_fileview(window);
	}
}

} // namespace gui2
