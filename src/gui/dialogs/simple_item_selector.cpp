/*
   Copyright (C) 2010 - 2018 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "gui/dialogs/simple_item_selector.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_simple_item_selector
 *
 * == Simple item selector ==
 *
 * A simple one-column listbox with OK and Cancel buttons.
 *
 * @begin{table}{dialog_widgets}
 *
 * title & & label & m &
 *         Dialog title label. $
 *
 * message & & styled_widget & m &
 *         Text label displaying a description or instructions. $
 *
 * listbox & & listbox & m &
 *         Listbox displaying user choices. $
 *
 * -item & & styled_widget & m &
 *         Widget which shows a listbox item label. $
 *
 * ok & & button & m &
 *         OK button. $
 *
 * cancel & & button & m &
 *         Cancel button. $
 *
 * @end{table}
 */

REGISTER_DIALOG(simple_item_selector)

simple_item_selector::simple_item_selector(const std::string& title,
											 const std::string& message,
											 const list_type& items,
											 bool title_uses_markup,
											 bool message_uses_markup)
	: index_(-1)
	, single_button_(false)
	, items_(items)
	, ok_label_()
	, cancel_label_()
{
	register_label("title", true, title, title_uses_markup);
	register_label("message", true, message, message_uses_markup);
}

void simple_item_selector::pre_show(window& window)
{
	listbox& list = find_widget<listbox>(&window, "listbox", false);
	window.keyboard_capture(&list);

	for(const auto & it : items_)
	{
		std::map<std::string, string_map> data;
		string_map column;

		column["label"] = it;
		data.emplace("item", column);

		list.add_row(data);
	}

	if(index_ != -1 && static_cast<unsigned>(index_) < list.get_item_count()) {
		list.select_row(index_);
	}

	index_ = -1;

	button& button_ok = find_widget<button>(&window, "ok", false);
	button& button_cancel = find_widget<button>(&window, "cancel", false);

	if(!ok_label_.empty()) {
		button_ok.set_label(ok_label_);
	}

	if(!cancel_label_.empty()) {
		button_cancel.set_label(cancel_label_);
	}

	if(single_button_) {
		button_cancel.set_visible(gui2::widget::visibility::invisible);
	}
}

void simple_item_selector::post_show(window& window)
{
	if(get_retval() == window::OK || single_button_) {
		index_ = find_widget<listbox>(&window, "listbox", false).get_selected_row();
	}
}

} // namespace dialogs
} // namespace gui2
