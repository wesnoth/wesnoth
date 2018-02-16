/*
   Copyright (C) 2014 - 2018 by Iris Morelle <shadowm2006@gmail.com>
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

#include "gui/dialogs/wml_error.hpp"

#include "addon/info.hpp"
// Needs the full path to avoid confusion with gui/dialogs/addon/manager.hpp.
#include "../../addon/manager.hpp"
#include "desktop/clipboard.hpp"
#include "filesystem.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/styled_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "serialization/string_utils.hpp"

#include "gettext.hpp"

#include "utils/functional.hpp"

namespace
{

void strip_trailing_dir_separators(std::string& str)
{
	while(filesystem::is_path_sep(str[str.size() - 1])) {
		str.erase(str.size() - 1);
	}
}

std::string format_file_list(const std::vector<std::string>& files_original)
{
	if(files_original.empty()) {
		return "";
	}

	const std::string& addons_path = filesystem::get_addons_dir();
	std::vector<std::string> files(files_original);

	for(std::string & file : files)
	{
		std::string base;
		std::string filename = filesystem::base_name(file);
		std::string parent_path;

		const bool is_main_cfg = filename == "_main.cfg";

		if(is_main_cfg) {
			parent_path = filesystem::directory_name(file) + "/..";
		} else {
			parent_path = filesystem::directory_name(file);
		}

		// Only proceed to pretty-format the filename if it's from the add-ons
		// directory.
		if(filesystem::normalize_path(parent_path) != filesystem::normalize_path(addons_path)) {
			continue;
		}

		if(is_main_cfg) {
			base = filesystem::directory_name(file);
			// HACK: fool filesystem::base_name() into giving us the parent directory name
			//       alone by making base seem not like a directory path,
			//       otherwise it returns an empty string.
			strip_trailing_dir_separators(base);
			base = filesystem::base_name(base);
		} else {
			base = filename;
		}

		if(base.empty()) {
			// We did something wrong. In the interest of not messing up the
			// report, leave the original filename intact.
			continue;
		}

		//
		// Display the name as an add-on name instead of a filename.
		//

		if(!is_main_cfg) {
			// Remove the file extension first.
			static const std::string wml_suffix = ".cfg";

			if(base.size() > wml_suffix.size()) {
				const size_t suffix_pos = base.size() - wml_suffix.size();
				if(base.substr(suffix_pos) == wml_suffix) {
					base.erase(suffix_pos);
				}
			}
		}

		if(have_addon_install_info(base)) {
			// _info.cfg may have the add-on's title starting with 1.11.7,
			// if the add-on was downloaded using the revised _info.cfg writer.
			config cfg;
			get_addon_install_info(base, cfg);

			const config& info_cfg = cfg.child("info");

			if(info_cfg && !info_cfg["title"].empty()) {
				file = info_cfg["title"].str();
				continue;
			}
		}

		// Fallback to using a synthetic title with underscores replaced with
		// whitespace.
		file = make_addon_title(base);
	}

	if(files.size() == 1) {
		return utils::indent(files.front());
	}

	return utils::bullet_list(files);
}
}

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_wml_error
 *
 * == WML error ==
 *
 * Dialog used to report WML parser or preprocessor errors.
 *
 * @begin{table}{dialog_widgets}
 *
 * summary & & styled_widget & m &
 *         Label used for displaying a brief summary of the error(s). $
 *
 * files & & styled_widget & m &
 *         Label used to display the list of affected add-ons or files, if
 *         applicable. It is hidden otherwise. It is recommended to place it
 *         after the summary label. $
 *
 * post_summary & & styled_widget & m &
 *         Label used for displaying instructions for reporting the error.
 *         It is recommended to place it after the file list label. It may be
 *         hidden if empty. $
 *
 * details & & styled_widget & m &
 *         Full report of the parser or preprocessor error(s) found. $
 *
 * copy & & button & m &
 *         Button that the user can click on to copy the error report to the
 *         system clipboard. $
 *
 * @end{table}
 */

REGISTER_DIALOG(wml_error)

wml_error::wml_error(const std::string& summary,
					   const std::string& post_summary,
					   const std::vector<std::string>& files,
					   const std::string& details)
	: have_files_(!files.empty())
	, have_post_summary_(!post_summary.empty())
	, report_()
{
	set_restore(true);

	const std::string& file_list_text = format_file_list(files);

	report_ = summary;

	if(!file_list_text.empty()) {
		report_ += "\n" + file_list_text;
	}

	if(!post_summary.empty()) {
		report_ += "\n\n" + post_summary;
	}

	report_ += "\n\n";
	report_ += _("Details:");
	report_ += "\n\n" + utils::indent(details);
	// Since this is likely to be pasted into a text file, add a final line
	// break for convenience, since otherwise the report ends mid-line.
	report_ += "\n";

	register_label("summary", true, summary);
	register_label("post_summary", true, post_summary);
	register_label("files", true, file_list_text);
	register_label("details", true, details);
}

void wml_error::pre_show(window& window)
{
	if(!have_files_) {
		styled_widget& filelist = find_widget<styled_widget>(&window, "files", false);
		filelist.set_visible(widget::visibility::invisible);
	}

	if(!have_post_summary_) {
		styled_widget& post_summary
				= find_widget<styled_widget>(&window, "post_summary", false);
		post_summary.set_visible(widget::visibility::invisible);
	}

	button& copy_button = find_widget<button>(&window, "copy", false);

	connect_signal_mouse_left_click(
			copy_button, std::bind(&wml_error::copy_report_callback, this));

	if (!desktop::clipboard::available()) {
		copy_button.set_active(false);
		copy_button.set_tooltip(_("Clipboard support not found, contact your packager"));
	}
}

void wml_error::copy_report_callback()
{
	desktop::clipboard::copy_to_clipboard(report_, false);
}

} // end namespace dialogs
} // end namespace gui2
