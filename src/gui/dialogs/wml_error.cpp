/*
	Copyright (C) 2014 - 2024
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

#include "gui/dialogs/wml_error.hpp"

#include "addon/info.hpp"
// Needs the full path to avoid confusion with gui/dialogs/addon/manager.hpp.
#include "../../addon/manager.hpp"
#include "desktop/clipboard.hpp"
#include "filesystem.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/styled_widget.hpp"
#include "gui/widgets/window.hpp"
#include "serialization/string_utils.hpp"

#include "gettext.hpp"

#include <functional>

namespace
{

void strip_trailing_dir_separators(std::string& str)
{
	while(filesystem::is_path_sep(str.back())) {
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

			if(base.size() > filesystem::wml_extension.size()) {
				const std::size_t suffix_pos = base.size() - filesystem::wml_extension.size();
				if(base.substr(suffix_pos) == filesystem::wml_extension) {
					base.erase(suffix_pos);
				}
			}
		}

		if(have_addon_install_info(base)) {
			// _info.cfg may have the add-on's title starting with 1.11.7,
			// if the add-on was downloaded using the revised _info.cfg writer.
			config info_cfg;
			get_addon_install_info(base, info_cfg);

			if(!info_cfg.empty() && !info_cfg["title"].empty()) {
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

namespace gui2::dialogs
{

REGISTER_DIALOG(wml_error)

wml_error::wml_error(const std::string& summary,
					   const std::string& post_summary,
					   const std::vector<std::string>& files,
					   const std::string& details)
	: modal_dialog(window_id())
	, have_files_(!files.empty())
	, have_post_summary_(!post_summary.empty())
	, report_()
{
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

void wml_error::pre_show()
{
	if(!have_files_) {
		styled_widget& filelist = find_widget<styled_widget>("files");
		filelist.set_visible(widget::visibility::invisible);
	}

	if(!have_post_summary_) {
		styled_widget& post_summary
				= find_widget<styled_widget>("post_summary");
		post_summary.set_visible(widget::visibility::invisible);
	}

	button& copy_button = find_widget<button>("copy");

	connect_signal_mouse_left_click(
			copy_button, std::bind(&wml_error::copy_report_callback, this));
}

void wml_error::copy_report_callback()
{
	desktop::clipboard::copy_to_clipboard(report_);
}

} // end namespace dialogs
