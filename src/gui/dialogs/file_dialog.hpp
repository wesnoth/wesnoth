/*
   Copyright (C) 2011, 2017 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_FILE_DIALOG_HPP_INCLUDED
#define GUI_DIALOGS_FILE_DIALOG_HPP_INCLUDED

#include "gui/dialogs/modal_dialog.hpp"

/**
 * Generic file dialog.
 *
 * This provides UI elements for browsing the filesystem and choosing a file
 * path to open or create, and optionally allows creating new directories or
 * deleting existing files and directories.
 *
 * Because of the sheer amount of unrelated options provided by this dialog,
 * no parameter-based constructors or static @a execute() functions are
 * provided. Use individual property setters after construction and before
 * invoking show(), instead.
 */
namespace gui2
{

class listbox;
class text_box;

namespace dialogs
{

class file_dialog : public modal_dialog
{
public:
	file_dialog();

	/**
	 * Gets the current dialog title text.
	 */
	const std::string& title() const
	{
		return title_;
	}

	/**
	 * Sets the current dialog title text.
	 */
	file_dialog& set_title(const std::string& value)
	{
		title_ = value;
		return *this;
	}

	/**
	 * Gets the current dialog instructions/message text.
	 */
	const std::string& message() const
	{
		return msg_;
	}

	/**
	 * Sets the current dialog instructions/message text.
	 *
	 * The message text may contain Pango markup.
	 */
	file_dialog& set_message(const std::string& value)
	{
		msg_ = value;
		return *this;
	}

	/**
	 * Gets the current file selection.
	 *
	 * @returns An absolute path to the selected file.
	 */
	std::string path() const;

	/**
	 * Sets the initial file selection.
	 *
	 * If the path is found to refer to a file (more specifically, any
	 * non-directory object), that file is initially selected on the directory
	 * contents view and the file name box is set to contain its name. If the file
	 * does not exist, but the path leading up to it does, the directory contents
	 * view displays that path and there isn't an initial file selection or name
	 * set unless set_filename() is used first.
	 *
	 * If you want to set an initial file name hint/template, use set_filename()
	 * <b>after</b> calling this method.
	 */
	file_dialog& set_path(const std::string& value);

	/**
	 * Sets the initial file name input but not the path.
	 *
	 * The file name needs not exist in the initial path selected with set_path().
	 *
	 * If this is used before set_path() and the path passed there points to a
	 * file, that file name will replace the one given here.
	 */
	file_dialog& set_filename(const std::string& value);

	/**
	 * Sets the default file extension for file names in save mode.
	 *
	 * When this is set to a non-empty string and save mode is active, selecting
	 * file entries will cause their name portions to be highlighted in the name
	 * text box if their extensions match the provided template, and any time the
	 * text box is cleared it will position the cursor before the extension as a
	 * hint for the user.
	 *
	 * The value provided to this method should be preceded by a dot if
	 * applicable (e.g. ".cfg").
	 */
	file_dialog& set_extension(const std::string& value)
	{
		extension_ = value;
		return *this;
	}

	/**
	 * Whether user interface elements for manipulating existing objects are provided.
	 */
	bool read_only() const
	{
		return read_only_;
	}

	/**
	 * Whether to provide user interface elements for manipulating existing objects.
	 *
	 * This is initially disabled.
	 */
	file_dialog& set_read_only(bool value)
	{
		read_only_ = value;
		return *this;
	}

	/**
	 * Returns whether save mode is enabled.
	 *
	 * See set_save_mode() for more information.
	 */
	bool save_mode() const
	{
		return save_mode_;
	}

	/**
	 * Sets the dialog's behavior on non-existent file name inputs.
	 *
	 * This is initially disabled.
	 *
	 * When save mode is enabled, file names entered into the dialog by the user
	 * need not exist already (but their parent directories still do). Otherwise,
	 * the user is only able to select existing files.
	 */
	file_dialog& set_save_mode(bool value)
	{
		save_mode_ = value;
		return *this;
	}

	/**
	 * Sets the OK button label.
	 *
	 * By default, "Save" is used when save_mode is enabled, and "Open" otherwise.
	 * Calling this method with an empty string will reset the label to the
	 * default.
	 */
	file_dialog& set_ok_label(const std::string& value)
	{
		ok_label_ = value;
		return *this;
	}

private:
	std::string title_;
	std::string msg_;
	std::string ok_label_;

	std::string extension_;
	std::string current_entry_;
	std::string current_dir_;

	bool read_only_;
	bool save_mode_;

	std::vector<std::string> dir_files_;
	std::vector<std::string> dir_subdirs_;

	std::vector<std::string> bookmark_paths_;
	int current_bookmark_;
	int user_bookmarks_begin_;

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from modal_dialog. */
	void pre_show(window& window);

	/** Handles dialog exit events and decides whether to proceed or not. */
	bool on_exit(window& window);
	/** Handles file/directory selection on single-click. */
	void on_row_selected(window& window);
	/** Handles selection or deselection of bookmarks. */
	void on_bookmark_selected(window& window);
	/** Handles Add Bookmark button press events. */
	void on_bookmark_add_cmd(window& window);
	/** Handles Remove Bookmark button press events. */
	void on_bookmark_del_cmd(window& window);
	/** Handles New Folder button press events. */
	void on_dir_create_cmd(window& window);
	/** Handles Delete button press events. */
	void on_file_delete_cmd(window& window);

	/**
	 * Processes file view selection in reaction to row double-click events.
	 *
	 * It takes care of synchronizing the state, browsing to the new selection,
	 * and/or displaying an error message if appropriate
	 *
	 * @returns Whether to exit the dialog successfully (@a true) or continue
	 *          (@a false).
	 */
	bool process_fileview_submit(window& window);

	/**
	 * Processes textbox input in reaction to OK button/Enter key events.
	 *
	 * It takes care of synchronizing the state, browsing to the new selection,
	 * and/or displaying an error message if appropriate
	 *
	 * @returns Whether to exit the dialog successfully (@a true) or continue
	 *          (@a false).
	 */
	bool process_textbox_submit(window& window);

	bool process_submit_common(window& window, const std::string& name);

	/**
	 * Updates the bookmarks bar state to reflect the internal state.
	 */
	void sync_bookmarks_bar(window& window);

	std::string get_filelist_selection(class listbox& filelist);

	enum SELECTION_TYPE
	{
		SELECTION_NOT_FOUND,
		SELECTION_PARENT_NOT_FOUND,
		SELECTION_IS_DIR,
		SELECTION_IS_FILE,
	};

	/**
	 * Returns whether the given selection type is acceptable for closing the dialog.
	 *
	 * @todo This currently never returns @a true for SELECTION_IS_DIR, awaiting
	 *       a need to implement a directory selection mode.
	 */
	bool is_selection_type_acceptable(SELECTION_TYPE stype) const;

	/**
	 * Prompts the user before overwriting an existing file.
	 *
	 * This only makes sense in save mode.
	 *
	 * @returns @a true if the selection does not refer to an existing file or the
	 *          user accepted the overwrite prompt; @a false otherwise.
	 */
	bool confirm_overwrite(window& window, SELECTION_TYPE stype);

	/**
	 * Updates the internal state and returns the type of the selection.
	 *
	 * If the given @a name refers to a non-existent object, the internal state is
	 * unchanged.
	 */
	SELECTION_TYPE register_new_selection(const std::string& name);

	void set_input_text(class text_box& t, const std::string& value);
	void clear_input_text(class text_box& t);

	/**
	 * Updates the dialog contents to match the internal state.
	 */
	void refresh_fileview(window& window);

	/**
	 * Row building helper for refresh_fileview().
	 *
	 * @param filelist        Target for adding the new row.
	 * @param name            Label, assumed to be a file name if
	 *                        check_selection = true.
	 * @param icon            Row icon.
	 * @param check_selection Whether to set the row to selected if the current
	 *                        file name in the internal state matches the row's
	 *                        label/name.
	 */
	void push_fileview_row(class listbox& filelist, const std::string& name, const std::string& icon, bool check_selection = true);
};

} // namespace dialogs
} // namespace gui2

#endif
