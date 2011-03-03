/* $Id$ */
/*
   Copyright (C) 2008 - 2011 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/image_message/option_message.hpp"

#include "gui/auxiliary/old_markup.hpp"
#include "gui/widgets/button.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

namespace gui2 {

void toption_message_::set_option_list(
		const std::vector<std::string>& option_list, int* chosen_option)
{
	assert(!option_list.empty());
	assert(chosen_option);

	option_list_ = option_list;
	chosen_option_ = chosen_option;
}


/**
 * @todo This function enables the wml markup for all items, but the interface
 * is a bit hacky. Especially the fiddling in the internals of the listbox is
 * ugly. There needs to be a clean interface to set whether a widget has a
 * markup and what kind of markup. These fixes will be post 1.6.
 */
void toption_message_::pre_show(CVideo& video, twindow& window)
{
	timage_message_::pre_show(video, window);

	// Find the option list related fields.
	tlistbox& options = find_widget<tlistbox>(&window, "option_list", true);

	std::map<std::string, string_map> data;
	for(size_t i = 0; i < option_list_.size(); ++i) {
		/**
		 * @todo This syntax looks like a bad hack, it would be nice to write
		 * a new syntax which doesn't use those hacks (also avoids the problem
		 * with special meanings for certain characters.
		 */
		tlegacy_menu_item item(option_list_[i]);

		if(item.is_default()) {
			// Number of items hasn't been increased yet so i is ok.
			*chosen_option_ = i;
		}

		// Add the data.
		data["icon"]["label"] = item.icon();
		data["label"]["label"] = item.label();
		data["label"]["use_markup"] = "true";
		data["description"]["label"] = item.description();
		data["description"]["use_markup"] = "true";
		options.add_row(data);
	}

	// Avoid negetive and 0 since item 0 is already selected.
	if(*chosen_option_ > 0
			&& static_cast<size_t>(*chosen_option_)
			< option_list_.size()) {

		options.select_row(*chosen_option_);
		}

	window.keyboard_capture(&options);
	window.set_click_dismiss(false);
	window.set_escape_disabled(true);

	window.add_to_keyboard_chain(&options);
	//options.set_visible(twidget::INVISIBLE);

}

void toption_message_::post_show(twindow& window)
{
	*chosen_option_ =
			find_widget<tlistbox>(&window, "option_list", true).get_selected_row();
}

REGISTER_DIALOG(option_message_left)
REGISTER_DIALOG(option_message_right)

int show_option_message(const bool left_side
		, CVideo& video
		, const std::string& title
		, const std::string& message
		, const std::string& portrait
		, const bool mirror
		, const std::vector<std::string>& option_list
		, int* chosen_option)
{
	std::auto_ptr<toption_message_> dlg;
	if(left_side) {
		dlg.reset(new toption_message_left(title, message, portrait, mirror));
	} else {
		dlg.reset(new toption_message_right(title, message, portrait, mirror));
	}
	assert(dlg.get());

	dlg->set_option_list(option_list, chosen_option);

	dlg->show(video);
	return dlg->get_retval();
}

} // namespace gui2

