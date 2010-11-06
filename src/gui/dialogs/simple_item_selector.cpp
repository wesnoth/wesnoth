/* $Id$ */
/*
   Copyright (C) 2010 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "foreach.hpp"
#include "gui/widgets/label.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
	#include "gui/widgets/list.hpp"
#else
	#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

namespace gui2 {

REGISTER_WINDOW(simple_item_selector)

tsimple_item_selector::tsimple_item_selector(const std::string& title, const std::string& message, list_type const& items, bool title_uses_markup, bool message_uses_markup)
	: index_(-1)
	, title_(title)
	, msg_(message)
	, markup_title_(title_uses_markup)
	, markup_msg_(message_uses_markup)
	, items_(items)
{
}

void tsimple_item_selector::pre_show(CVideo& /*video*/, twindow& window)
{
	tlabel& ltitle = find_widget<tlabel>(&window, "title", false);
	tlabel& lmessage = find_widget<tlabel>(&window, "message", false);
	tlistbox& list = find_widget<tlistbox>(&window, "listbox", false);

	ltitle.set_label(title_);
	ltitle.set_use_markup(markup_title_);

	lmessage.set_label(msg_);
	lmessage.set_use_markup(markup_msg_);

	foreach(const tsimple_item_selector::item_type& it, items_) {
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
