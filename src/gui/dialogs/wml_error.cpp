/*
   Copyright (C) 2014 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "gui/dialogs/wml_error.hpp"

#include "addon/info.hpp"
#include "gui/auxiliary/find_widget.tpp"
#include "gui/widgets/control.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "serialization/string_utils.hpp"

namespace
{

std::string format_file_list(const std::vector<std::string>& files)
{
	if(files.empty()) {
		return "";
	}

	if(files.size() == 1) {
		return files.front();
	}

	return utils::bullet_list(files);
}

}

namespace gui2
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
 * summary & & control & m &
 *         Label used for displaying a brief summary of the error(s). $
 *
 * files & & control & m &
 *         Label used to display the list of affected add-ons or files, if
 *         applicable. It is hidden otherwise. It is recommended to place it
 *         after the summary label. $
 *
 * details & & control & m &
 *         Full report of the parser or preprocessor error(s) found. $
 *
 * @end{table}
 */

REGISTER_DIALOG(wml_error)

twml_error::twml_error(const std::string& summary,
					   const std::vector<std::string>& files,
					   const std::string& details)
	: have_files_(!files.empty())
{
	register_label("summary", true, summary);
	register_label("files", true, format_file_list(files));
	register_label("details", true, details);
}

void twml_error::pre_show(CVideo& /*video*/, twindow& window)
{
    if(!have_files_) {
		tcontrol& filelist = find_widget<tcontrol>(&window, "files", false);
		filelist.set_visible(tcontrol::tvisible::invisible);
	}
}

} // end namespace gui2
