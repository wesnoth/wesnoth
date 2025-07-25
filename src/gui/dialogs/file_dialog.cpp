/*
	Copyright (C) 2011 - 2025
	by Iris Morelle <shadowm2006@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "desktop/paths.hpp"
#include "desktop/open.hpp"
#include "filesystem.hpp"
#include "formula/string_utils.hpp"
#include "gui/dialogs/folder_create.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "gui/widgets/window.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "serialization/markup.hpp"
#include "serialization/unicode.hpp"

#include <boost/filesystem/path.hpp>

#include <algorithm>
#include <functional>

static lg::log_domain log_filedlg{"gui/dialogs/file_dialog"};
#define ERR_FILEDLG   LOG_STREAM(err,   log_filedlg)
#define WRN_FILEDLG   LOG_STREAM(warn,  log_filedlg)
#define LOG_FILEDLG   LOG_STREAM(info,  log_filedlg)
#define DBG_FILEDLG   LOG_STREAM(debug, log_filedlg)

namespace fs = filesystem;

namespace
{
const std::string icon_dir = "icons/action/browse_25.png";
const std::string icon_parent = "icons/action/undo_25.png";
const std::string icon_file = "misc/file.png";
// NOTE: Does not need to be the same as PARENT_DIR! Use PARENT_DIR to build
//       relative paths for non-presentational purposes instead.
const std::string label_parent = "..";

const std::string CURRENT_DIR = ".";
const std::string PARENT_DIR = "..";

// Some fonts used for internationalization don't define this glyph
const std::string error_marker = markup::span_attribute("face", "DejaVuSans", "✘");

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

inline bool path_contains_space(const std::string& path)
{
	return std::find_if(path.begin(), path.end(), utils::portable_isspace) != path.end();
}

} // unnamed namespace

namespace gui2::dialogs
{

REGISTER_DIALOG(file_dialog)

file_dialog::file_dialog()
	: modal_dialog(window_id())
	, title_(_("Find File"))
	, msg_()
	, ok_label_()
	, extension_()
	, current_entry_()
	, current_dir_()
	, read_only_(false)
	, save_mode_(false)
	, dir_files_()
	, dir_subdirs_()
	, bookmark_paths_()
	, current_bookmark_()
	, user_bookmarks_begin_()
	, extra_paths_()
{
}

std::string file_dialog::path() const
{
	const std::string& dir_norm = fs::normalize_path(current_dir_, true);

	if(current_entry_.empty() || current_entry_ == CURRENT_DIR) {
		return dir_norm;
	} else if(current_entry_ == PARENT_DIR) {
		return fs::directory_name(dir_norm);
	}

	return concat_path(dir_norm, current_entry_);
}

file_dialog& file_dialog::set_path(const std::string& value)
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

file_dialog& file_dialog::set_filename(const std::string& value)
{
	current_entry_ = value;
	return *this;
}

bool file_dialog::check_filename()
{
	if(!save_mode_) {
		return true;
	}

	auto& validation_msg = find_widget<styled_widget>("validation_msg");
	auto& save_btn = find_widget<button>("ok");

	// Most codepaths want to disable the button; do it preemptively.
	save_btn.set_active(false);

	boost::filesystem::path filename(find_widget<text_box>("filename").get_value());

	// Empty filename
	if(filename.stem().empty()) {
		validation_msg.set_label(markup::span_color("#00dcff", _("enter filename")));
		return false;
	}

	// Invalid extension
	if(!utils::contains(extensions_, filename.extension())) {
		// TODO: make extensions_ itself a vector<t_string> if possible
		auto as_tstrings = std::vector<t_string>(extensions_.begin(), extensions_.end());
		utils::string_map i18n_strings{{"extensions", utils::format_disjunct_list("", as_tstrings)}};

		std::string message = VGETTEXT("invalid extension (use $extensions)", i18n_strings);
		validation_msg.set_label(markup::span_color("red", error_marker, ' ', message));
		return false;
	}

	// Whitespace
	if(path_contains_space(filename.string())) {
		std::string message = _("filename contains whitespace");
		validation_msg.set_label(markup::span_color("red", error_marker, ' ', message));
		return false;
	}

	// Ensure there's always *some* value in the validation message (hence, whitespace)
	// so it doesn't get hidden if there's no validation message when the dialog is opened.
	// Since this function is called from pre_show, it's fine to do this here.
	validation_msg.set_label(" ");
	save_btn.set_active(true);
	return true;
}

void file_dialog::pre_show()
{
	styled_widget& title = find_widget<styled_widget>("title");
	styled_widget& message = find_widget<styled_widget>("message");
	styled_widget& ok = find_widget<styled_widget>("ok");

	title.set_label(title_);

	if(msg_.empty()) {
		message.set_visible(gui2::widget::visibility::invisible);
	} else {
		message.set_label(msg_);
		message.set_use_markup(true);
	}

	if(ok_label_.empty()) {
		ok.set_label(save_mode_ ? _("Save") : _("Open"));
	} else {
		ok.set_label(ok_label_);
	}

	listbox& bookmarks_bar = find_widget<listbox>("bookmarks");

	find_widget<styled_widget>("current_dir").set_text_ellipse_mode(PANGO_ELLIPSIZE_START);

	//
	// Push hard-coded bookmarks.
	//

	extra_paths_.emplace(desktop::GAME_CORE_DATA_DIR);
	extra_paths_.emplace(desktop::GAME_USER_DATA_DIR);
	std::vector<desktop::path_info> bookmarks = desktop::game_paths(extra_paths_);
	const auto& sys_paths = desktop::system_paths({desktop::SYSTEM_ALL_DRIVES, desktop::SYSTEM_USER_PROFILE, desktop::SYSTEM_ROOTFS});
	bookmarks.insert(bookmarks.end(), sys_paths.begin(), sys_paths.end());

	bookmark_paths_.clear();
	current_bookmark_ = user_bookmarks_begin_ = -1;

	for(const auto& pinfo : bookmarks) {
		bookmark_paths_.push_back(pinfo.path);
		bookmarks_bar.add_row(widget_data{{ "bookmark", {{ "label", pinfo.display_name() }}}});
	}

	//
	// Push user-defined bookmarks.
	//

	const std::vector<desktop::bookmark_info>& user_bookmarks = desktop::user_bookmarks();

	if(!user_bookmarks.empty()) {
		user_bookmarks_begin_ = bookmark_paths_.size();
	}

	for(const auto& bookmark : user_bookmarks) {
		bookmark_paths_.push_back(bookmark.path);
		bookmarks_bar.add_row(widget_data{{ "bookmark", {{ "label", bookmark.label }}}});
	}

	sync_bookmarks_bar();

	listbox& filelist = find_widget<listbox>("filelist");
	text_box& file_textbox = find_widget<text_box>("filename");

	connect_signal_notify_modified(filelist,
			std::bind(&file_dialog::on_row_selected, this));
	connect_signal_notify_modified(bookmarks_bar,
			std::bind(&file_dialog::on_bookmark_selected, this));
	connect_signal_notify_modified(file_textbox,
			std::bind(&file_dialog::check_filename, this));

	check_filename();

	button& mkdir_button = find_widget<button>("new_dir");
	button& rm_button = find_widget<button>("delete_file");
	button& bookmark_add_button = find_widget<button>("add_bookmark");
	button& bookmark_del_button = find_widget<button>("remove_bookmark");
	button& open_ext_button = find_widget<button>("open_ext");

	connect_signal_mouse_left_click(mkdir_button,
			std::bind(&file_dialog::on_dir_create_cmd, this));
	connect_signal_mouse_left_click(rm_button,
			std::bind(&file_dialog::on_file_delete_cmd, this));
	connect_signal_mouse_left_click(bookmark_add_button,
			std::bind(&file_dialog::on_bookmark_add_cmd, this));
	connect_signal_mouse_left_click(bookmark_del_button,
			std::bind(&file_dialog::on_bookmark_del_cmd, this));

	if (desktop::open_object_is_supported()) {
		connect_signal_mouse_left_click(open_ext_button,
			[this](auto&&...) { desktop::open_object(path()); });
	} else {
		open_ext_button.set_active(false);
		open_ext_button.set_tooltip(_("Opening files is not supported, contact your packager"));
	}

	if(read_only_) {
		mkdir_button.set_active(false);
		rm_button.set_active(false);

		mkdir_button.set_visible(widget::visibility::invisible);
		rm_button.set_visible(widget::visibility::invisible);
	}

	refresh_fileview();

	//window.keyboard_capture(find_widget<text_box>("filename", false, true));
	keyboard_capture(&file_textbox);
	add_to_keyboard_chain(&filelist);
	set_exit_hook(window::exit_hook::always, [this] { return on_exit(); });
}

bool file_dialog::on_exit()
{
	if(get_retval() == FILE_DIALOG_ITEM_RETVAL) {
		// Attempting to exit by double clicking items -- only proceeds if the item
		// was a file.
		if(process_fileview_submit()) {
			set_retval(retval::OK, false);
			return true;
		} else {
			return false;
		}
	}

	if(get_retval() == retval::OK) {
		// Attempting to exit by pressing Enter/clicking OK -- only proceeds if the
		// textbox was not altered by the user to point to a different directory.
		return process_textbox_submit();
	}

	return true;
}

bool file_dialog::is_selection_type_acceptable(file_dialog::SELECTION_TYPE stype) const
{
	// TODO: Adapt for implementing directory selection mode.
	return save_mode_
			? stype != SELECTION_IS_DIR && stype != SELECTION_PARENT_NOT_FOUND
			: stype == SELECTION_IS_FILE;
}

bool file_dialog::confirm_overwrite(file_dialog::SELECTION_TYPE stype)
{
	// TODO: Adapt for implementing directory selection mode.
	if(stype != SELECTION_IS_FILE) {
		return true;
	}

	const std::string& message
			= _("The file already exists. Do you wish to overwrite it?");
	return gui2::show_message(_("Confirm"), message, message::yes_no_buttons) != gui2::retval::CANCEL;
}

bool file_dialog::process_submit_common(const std::string& name)
{
	const auto stype = register_new_selection(name);

	//DBG_FILEDLG << "current_dir_=" << current_dir_ << "  current_entry_=" << current_entry_;

	if(is_selection_type_acceptable(stype)) {
		// TODO: evaluate if we want to call check_filename() here
		return save_mode_ ? check_filename() && confirm_overwrite(stype) : true;
	}

	switch(stype) {
		case SELECTION_IS_DIR:
			// TODO: Adapt for implementing directory selection mode.
			sync_bookmarks_bar();
			refresh_fileview();
			break;
		case SELECTION_PARENT_NOT_FOUND:
			// We get here in save mode or not. Use the file creation language only in
			// save mode.
			if(save_mode_) {
				show_transient_error_message(VGETTEXT("The file or folder $path cannot be created.", {{"path", name}}));
				break;
			}
			[[fallthrough]];
		case SELECTION_NOT_FOUND:
			// We only get here if we aren't in save mode.
			show_transient_error_message(VGETTEXT("The file or folder $path does not exist.", {{"path", name}}));
			break;
		case SELECTION_IS_FILE:
			// TODO: Adapt for implementing directory selection mode.
		default:
			assert(false && "Unimplemented selection mode or semantics");
	}

	return false;
}

bool file_dialog::process_fileview_submit()
{
	listbox& filelist = find_widget<listbox>("filelist");
	const std::string& selected_name = get_filelist_selection(filelist);
	return process_submit_common(selected_name);
}

bool file_dialog::process_textbox_submit()
{
	text_box& file_textbox = find_widget<text_box>("filename");
	const std::string& input_name = file_textbox.get_value();
	return !input_name.empty() && process_submit_common(input_name);
}

std::string file_dialog::get_filelist_selection(listbox& filelist)
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
		std::size_t n = i_am_root ? row : row - 1;

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

file_dialog::SELECTION_TYPE file_dialog::register_new_selection(const std::string& name)
{
	std::string new_path, new_parent;

	if(fs::is_relative(name)) {
		// On Windows, \ represents a path relative to the root of the process'
		// current working drive specified by the current working dir, so we get
		// here. This makes it the only platform where is_relative() and is_root()
		// aren't mutually exclusive.
		if(fs::is_root(name)) {
			DBG_FILEDLG << "register_new_selection(): new selection '" << name << "' is relative to a root resource";
			// Using the browsed dir's root drive instead of the cwd's makes the most
			// sense for users.
			new_parent = fs::root_name(current_dir_);
			new_path = fs::normalize_path(concat_path(new_parent, name), true, true);
		} else {
			DBG_FILEDLG << "register_new_selection(): new selection '" << name << "' seems relative";
			new_parent = current_dir_;
			new_path = fs::normalize_path(concat_path(current_dir_, name), true, true);
		}
	} else {
		DBG_FILEDLG << "register_new_selection(): new selection '" << name << "' seems absolute";
		new_parent = fs::directory_name(name);
		new_path = fs::normalize_path(name, true, true);
		DBG_FILEDLG << "register_new_selection(): new selection is " << new_path;
	}

	if(!new_path.empty()) {
		if(fs::is_directory(new_path)) {
			DBG_FILEDLG << "register_new_selection(): new selection '" << name << "' is a directory: " << new_path;
			current_dir_ = new_path;
			current_entry_.clear();
			return SELECTION_IS_DIR;
		} else if(fs::file_exists(new_path)) {
			// FIXME: Perhaps redundant since the three-params call to normalize_path()
			//        above necessarily validates existence.
			DBG_FILEDLG << "register_new_selection(): new selection '" << name << "' is a file, symbolic link, or special: " << new_path;
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
		DBG_FILEDLG << "register_new_selection(): new selection '" << name << "' does not exist or is not accessible, but parent exists";
		current_dir_ = absolute_parent;
		current_entry_ = fs::base_name(name);
		return SELECTION_NOT_FOUND;
	}

	DBG_FILEDLG << "register_new_selection(): new selection '" << name << "' does not exist or is not accessible";
	return SELECTION_PARENT_NOT_FOUND;
}

void file_dialog::set_input_text(text_box& t, const std::string& value)
{
	if(value.empty()) {
		clear_input_text(t);
		return;
	}

	t.set_value(value);
	check_filename();

	const std::size_t vallen = t.get_length();
	const std::size_t extlen = utf8::size(extension_);

	if(save_mode_ && extlen && vallen > extlen) {
		// Highlight everything but the extension if it matches
		if(value.substr(vallen - extlen) == extension_) {
			t.set_selection(0, vallen - extlen);
		}
	}
}

void file_dialog::clear_input_text(text_box& t)
{
	if(save_mode_ && !extension_.empty()) {
		t.set_value(extension_);
		t.set_selection(0, 0);
	} else {
		t.clear();
	}
}

void file_dialog::refresh_fileview()
{
	cursor::setter cur{cursor::WAIT};

	dir_files_.clear();
	dir_subdirs_.clear();

	// TODO: Need to detect and handle cases where we don't have search permission
	//       on current_dir_, otherwise things may get weird.
	filesystem::get_files_in_dir(current_dir_, &dir_files_, &dir_subdirs_, filesystem::name_mode::FILE_NAME_ONLY);
	isort_dir_entries(dir_files_);
	isort_dir_entries(dir_subdirs_);

	//
	// Clear and refill the filelist box.
	//

	listbox& filelist = find_widget<listbox>("filelist");
	button& rm_button = find_widget<button>("delete_file");

	filelist.clear();

	// Parent entry
	if(!fs::is_root(current_dir_)) {
		// label_parent may not necessarily be always ".." in the future, so push
		// with check_selection = false and check the selection ourselves here.
		push_fileview_row(filelist, label_parent, icon_parent, false);
		if(current_entry_ == PARENT_DIR || current_entry_.empty()) {
			filelist.select_row(0, true);
			rm_button.set_active(false);
		} else {
			rm_button.set_active(true);
		}
	}

	for(const auto& dir : dir_subdirs_) {
		push_fileview_row(filelist, dir, icon_dir);
	}

	for(const auto& file : dir_files_) {
		push_fileview_row(filelist, file, icon_file);
	}

	find_widget<styled_widget>("current_dir").set_label(current_dir_);
	set_input_text(find_widget<text_box>("filename"), current_entry_);

	on_row_selected();
}

void file_dialog::push_fileview_row(listbox& filelist, const std::string& name, const std::string& icon, bool check_selection)
{
	// TODO: Hopefully some day GUI2 will allow us to make labels be ellipsized
	//       dynamically at layout/rendering time.
	std::string label = name;
	utils::ellipsis_truncate(label, FILE_DIALOG_MAX_ENTRY_LENGTH);

	grid& last_grid = filelist.add_row(widget_data{
		{ "icon", {{ "label", icon }}},
		{ "file", {{ "label", label }}},
	});

	//
	// Crummy hack around the lack of an option to hook into row double click
	// events for all rows using the GUI2 listbox API. Assign a special retval to
	// each row that triggers a special check during dialog exit.
	//
	last_grid
		.find_widget<toggle_panel>("item_panel")
		.set_retval(FILE_DIALOG_ITEM_RETVAL);

	if(check_selection && name == current_entry_) {
		filelist.select_last_row(true);
	}
}

void file_dialog::sync_bookmarks_bar()
{
	listbox& bookmarks_bar = find_widget<listbox>("bookmarks");

	// Internal state has normalized path delimiters but dot entries aren't
	// resolved after callers call set_path(), so compare against a canonical
	// version. The bookmark paths are already canonical, though.
	const std::string& canon_current_dir = fs::normalize_path(current_dir_, true, true);

	// Go backwards so we can match user-defined bookmarks first (otherwise it may
	// become impossible for the user to delete them if they match any of the
	// predefined paths).
	auto it = std::find(bookmark_paths_.rbegin(), bookmark_paths_.rend(), canon_current_dir);

	if(it == bookmark_paths_.rend()) {
		if(current_bookmark_ >= 0) {
			bookmarks_bar.select_row(static_cast<unsigned>(current_bookmark_), false);
		}
		current_bookmark_ = -1;
	} else {
		const int new_selection = static_cast<int>(std::distance(bookmark_paths_.begin(), it.base()) - 1);
		if(new_selection != current_bookmark_) {
			assert(static_cast<unsigned>(new_selection) < bookmarks_bar.get_item_count());
			if(current_bookmark_ >= 0) {
				bookmarks_bar.select_row(static_cast<unsigned>(current_bookmark_), false);
			}
			bookmarks_bar.select_row(static_cast<unsigned>(new_selection), true);
			current_bookmark_ = new_selection;
		}
	}

	// Update bookmark edit controls.
	button& del_button = find_widget<button>("remove_bookmark");

	if(user_bookmarks_begin_ == -1) {
		del_button.set_active(false);
	} else {
		del_button.set_active(current_bookmark_ >= user_bookmarks_begin_);
	}
}

void file_dialog::on_row_selected()
{
	listbox& filelist = find_widget<listbox>("filelist");
	text_box& file_textbox = find_widget<text_box>("filename");
	button& rm_button = find_widget<button>("delete_file");

	// Don't use register_new_selection() here, we don't want any parsing to be
	// performed at this point.
	current_entry_ = get_filelist_selection(filelist);

	// Clear the textbox when selecting ..
	if(current_entry_ != PARENT_DIR) {
		set_input_text(file_textbox, current_entry_);
		rm_button.set_active(true);
	} else {
		clear_input_text(file_textbox);
		rm_button.set_active(false);
	}

	// Need to do this every time so that input can still be sent to the
	// textbox without clicking on it.
	keyboard_capture(&file_textbox);
}

void file_dialog::on_bookmark_selected()
{
	// Don't let us steal the focus from the primary widgets.
	text_box& file_textbox = find_widget<text_box>("filename");
	keyboard_capture(&file_textbox);

	listbox& bookmarks_bar = find_widget<listbox>("bookmarks");
	const int new_selection = bookmarks_bar.get_selected_row();

	if(new_selection < 0) {
		if(current_bookmark_ >= 0) {
			// Don't allow the user to deselect the selected bookmark. That wouldn't
			// make any sense.
			bookmarks_bar.select_row(static_cast<unsigned>(current_bookmark_));
		}

		return;
	}

	assert(static_cast<unsigned>(new_selection) < bookmark_paths_.size());
	current_bookmark_ = new_selection;
	set_path(bookmark_paths_[new_selection]);
	refresh_fileview();

	// Update bookmark edit controls.
	button& del_button = find_widget<button>("remove_bookmark");
	del_button.set_active(user_bookmarks_begin_ >= 0
						  && current_bookmark_ >= user_bookmarks_begin_);
}

void file_dialog::on_bookmark_add_cmd()
{
	const std::string& default_label = fs::base_name(current_dir_);

	std::string label = default_label;

	const bool confirm = bookmark_create::execute(label);
	if(!confirm) {
		return;
	}

	if(label.empty()) {
		label = default_label;
	}

	listbox& bookmarks_bar = find_widget<listbox>("bookmarks");

	desktop::add_user_bookmark(label, current_dir_);
	bookmark_paths_.push_back(current_dir_);
	const unsigned top_bookmark = bookmark_paths_.size() - 1;

	if(user_bookmarks_begin_ == -1) {
		user_bookmarks_begin_ = top_bookmark;
	}

	widget_data data;
	data["bookmark"]["label"] = label;
	bookmarks_bar.add_row(data);

	current_bookmark_ = -1;

	sync_bookmarks_bar();
}

void file_dialog::on_bookmark_del_cmd()
{
	assert(user_bookmarks_begin_ >= 0
		   && current_bookmark_ >= 0
		   && current_bookmark_ >= user_bookmarks_begin_
		   && current_bookmark_ < static_cast<int>(bookmark_paths_.size()));

	listbox& bookmarks_bar = find_widget<listbox>("bookmarks");
	desktop::remove_user_bookmark(current_bookmark_ - user_bookmarks_begin_);
	bookmark_paths_.erase(bookmark_paths_.begin() + current_bookmark_);
	bookmarks_bar.remove_row(current_bookmark_);

	current_bookmark_ = -1;

	sync_bookmarks_bar();
}

void file_dialog::on_dir_create_cmd()
{
	std::string new_dir_name;

	if(folder_create::execute(new_dir_name)) {
		const std::string& new_path = concat_path(current_dir_, new_dir_name);

		if(!fs::make_directory(new_path)) {
			show_transient_error_message(
					VGETTEXT("Could not create a new folder at $path|. Make sure you have the appropriate permissions to write to this location.",
					{{"path", new_path}}));
		} else {
			refresh_fileview();
		}
	}
}

void file_dialog::on_file_delete_cmd()
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

	if(gui2::show_message(_("Confirm"), message, message::yes_no_buttons) == gui2::retval::CANCEL) {
		return;
	}

	const bool result = is_dir
			? fs::delete_directory(selection)
			: fs::delete_file(selection);

	if(!result) {
		show_transient_error_message(
				VGETTEXT("Could not delete $path|. Make sure you have the appropriate permissions to write to this location.",
						 {{"path", selection}}));
	} else {
		refresh_fileview();
	}
}

} // namespace dialogs
