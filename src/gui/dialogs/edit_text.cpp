/*
   Copyright (C) 2013 - 2018 by Iris Morelle <shadowm2006@gmail.com>
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

#include "gui/dialogs/edit_text.hpp"

#include "gui/auxiliary/field.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"

#include <boost/algorithm/string/trim.hpp>

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_rename_unit
 *
 * == Rename unit ==
 *
 * Dialog for renaming units in-game.
 *
 * @begin{table}{dialog_widgets}
 *
 * name & & text_box & m &
 *         Input field for the unit name. $
 *
 * @end{table}
 */

REGISTER_DIALOG(edit_text)

edit_text::edit_text(const std::string& title,
					   const std::string& label,
					   std::string& text,
					   bool disallow_empty)
	: disallow_empty_(disallow_empty)
{
	register_label("title", true, title, true);
	register_label("label", true, label, true);
	register_text("text", true, text, true);
}

void edit_text::pre_show(window& window)
{
	if(disallow_empty_) {
		text_box& text = find_widget<text_box>(&window, "text", false);
		connect_signal_notify_modified(text, std::bind(&edit_text::on_text_change, this));
		on_text_change();
	}
}

void edit_text::on_text_change()
{
	text_box& text = find_widget<text_box>(get_window(), "text", false);
	button& ok_button = find_widget<button>(get_window(), "ok", false);

	ok_button.set_active(!boost::trim_copy(text.get_value()).empty());
}

} // namespace dialogs
} // namespace gui2
