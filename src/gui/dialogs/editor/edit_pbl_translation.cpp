/*
	Copyright (C) 2023 - 2024
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

#include "gui/dialogs/editor/edit_pbl_translation.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/text_box.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(editor_edit_pbl_translation)

editor_edit_pbl_translation::editor_edit_pbl_translation(
	std::string& language, std::string& title, std::string& description)
	: modal_dialog(window_id())
	, language_(language)
	, title_(title)
	, description_(description)
{
}

void editor_edit_pbl_translation::pre_show(window& win)
{
	text_box* language = find_widget<text_box>(&win, "language", false, true);
	win.keyboard_capture(language);
}

void editor_edit_pbl_translation::post_show(window& win)
{
	language_ = find_widget<text_box>(&win, "language", false).get_value();
	title_ = find_widget<text_box>(&win, "lang_title", false).get_value();
	description_ = find_widget<text_box>(&win, "description", false).get_value();
}

} // namespace gui2::dialogs
