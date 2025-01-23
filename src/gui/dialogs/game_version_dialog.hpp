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

namespace gui2::dialogs
{
class game_version : public modal_dialog
{
public:
	/**
	 * Constructor.
	 */
	game_version(unsigned start_page = 0);

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

	unsigned start_page_;

	void generate_plain_text_report();

	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

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
	 * @param btn_id Id of the button that calls this method
	 */
	void copy_to_clipboard_callback(const std::string& path, const std::string& btn_id);

	/**
	 * Callback function for browse-directory action buttons.
	 *
	 * @param path Filesystem path associated with the widget.
	 */
	void browse_directory_callback(const std::string& path);

	/**
	 * Show credits dialogs
	 */
	void show_credits_dialog();
	/**
	 * Show license
	 */
	void show_license();

	/**
	 * Open browser to report issue
	 */
	void report_issue();

	/**
	 * Re-runs the version migration dialog.
	 */
	void run_migrator();

	/**
	 * Opens the game manual in the platform's browser
	 */
	void show_manual();
};

} // namespace gui2::dialogs
