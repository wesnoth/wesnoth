/*
	Copyright (C) 2013 - 2024
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

#pragma once

#include "gui/dialogs/modal_dialog.hpp"

#include "build_info.hpp"

#include <map>

#include <array>

namespace gui2
{
class listbox;

namespace dialogs
{

/**
 * @ingroup GUIWindowDefinitionWML
 *
 * Dialog displaying the various paths used by the game to locate resource and configuration files.
 *
 * There are several item types used to build widget ids in this dialog.
 * All references to TYPE below refer to the following suffixes: datadir, config, userdata, saves, addons, cache.
 * Key               |Type          |Mandatory|Description
 * ------------------|--------------|---------|-----------
 * path_TYPE         | text_box     |yes      |Textbox containing the filesystem path for the given item.
 * copy_TYPE         | @ref button  |yes      |Copies the given item's path to clipboard.
 * browse_TYPE       | @ref button  |yes      |Launches the default file browser on the given item's path.
 */
class game_version : public modal_dialog
{
public:
	/**
	 * Constructor.
	 */
	game_version();

	/**
	 * The display function.
	 *
	 * See @ref modal_dialog for more information.
	 */
	DEFINE_SIMPLE_DISPLAY_WRAPPER(game_version)

private:
	const std::string path_wid_stem_;
	const std::string copy_wid_stem_;
	const std::string browse_wid_stem_;

	std::map<std::string, std::string> path_map_;

	/** path to current log file */
	const std::string log_path_;

	typedef std::array<std::string, 4> deplist_entry;
	std::vector<deplist_entry> deps_;

	std::vector<game_config::optional_feature> opts_;

	std::string report_;

	void generate_plain_text_report();

	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	//
	// Widget event callbacks.
	//

	/**
	 * Callback function called when switching tabs.
	 */
	void tab_switch_callback();

	/**
	 * Callback function for the dialog-wide copy-to-clipboard button.
	 */
	void report_copy_callback();

	/**
	 * Callback function for copy-to-clipboard action buttons.
	 *
	 * @param path Filesystem path associated with the widget.
	 */
	void copy_to_clipboard_callback(const std::string& path);

	/**
	 * Callback function for browse-directory action buttons.
	 *
	 * @param path Filesystem path associated with the widget.
	 */
	void browse_directory_callback(const std::string& path);
};
} // namespace dialogs
} // namespace gui2
