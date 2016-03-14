/*
   Copyright (C) 2010 - 2016 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "gui/auxiliary/find_widget.tpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "utils/foreach.tpp"

namespace gui2
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
 * message & & control & m &
 *         Text label displaying a description or instructions. $
 *
 * listbox & & listbox & m &
 *         Listbox displaying user choices. $
 *
 * -item & & control & m &
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

tsimple_item_selector::tsimple_item_selector(const std::string& title,
											 const std::string& message,
											 list_type const& items,
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

void tsimple_item_selector::pre_show(twindow& window)
{
	tlistbox& list = find_widget<tlistbox>(&window, "listbox", false);
	window.keyboard_capture(&list);

	FOREACH(const AUTO & it, items_)
	{
		std::map<std::string, string_map> data;
		string_map column;

		column["label"] = it;
		data.insert(std::make_pair("item", column));

		list.add_row(data);
	}

	if(index_ != -1 && static_cast<unsigned>(index_) < list.get_item_count()) {
		list.select_row(index_);
	}

	index_ = -1;

	tbutton& button_ok = find_widget<tbutton>(&window, "ok", false);
	tbutton& button_cancel = find_widget<tbutton>(&window, "cancel", false);

	if(!ok_label_.empty()) {
		button_ok.set_label(ok_label_);
	}

	if(!cancel_label_.empty()) {
		button_cancel.set_label(cancel_label_);
	}

	if(single_button_) {
		button_cancel.set_visible(gui2::twidget::tvisible::invisible);
	}
}

void tsimple_item_selector::post_show(twindow& window)
{
	if(get_retval() != twindow::OK) {
		return;
	}

	tlistbox& list = find_widget<tlistbox>(&window, "listbox", false);
	index_ = list.get_selected_row();
}
}
