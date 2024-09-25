/*
	Copyright (C) 2010 - 2024
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

#include "gui/dialogs/simple_item_selector.hpp"

#include "gui/widgets/button.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/window.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(simple_item_selector)

simple_item_selector::simple_item_selector(const std::string& title,
											 const std::string& message,
											 const list_type& items,
											 bool title_uses_markup,
											 bool message_uses_markup)
	: modal_dialog(window_id())
	, index_(-1)
	, single_button_(false)
	, items_(items)
	, ok_label_()
	, cancel_label_()
{
	register_label("title", true, title, title_uses_markup);
	register_label("message", true, message, message_uses_markup);
}

void simple_item_selector::pre_show()
{
	listbox& list = find_widget<listbox>("listbox");
	keyboard_capture(&list);

	for(const auto & it : items_)
	{
		widget_data data;
		widget_item column;

		column["label"] = it;
		data.emplace("item", column);

		list.add_row(data);
	}

	if(index_ != -1 && static_cast<unsigned>(index_) < list.get_item_count()) {
		list.select_row(index_);
	}

	index_ = -1;

	button& button_ok = find_widget<button>("ok");
	button& button_cancel = find_widget<button>("cancel");

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

void simple_item_selector::post_show()
{
	if(get_retval() == retval::OK || single_button_) {
		index_ = find_widget<listbox>("listbox").get_selected_row();
	}
}

} // namespace dialogs
