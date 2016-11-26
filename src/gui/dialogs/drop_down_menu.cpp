/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/drop_down_menu.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/integer_selector.hpp"
#include "gui/widgets/scrollbar.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "gui/widgets/window.hpp"

#include "sdl/rect.hpp"
#include "utils/functional.hpp"

namespace gui2
{
namespace dialogs
{

REGISTER_DIALOG(drop_down_menu)

namespace {
	void click_callback(window& window, bool&, bool&, point coordinate)
	{
		/* FIXME: This dialog uses a listbox with 'has_minimum = false'. This allows a listbox to have 0 or more selections,
		 * and selecting the same entry toggles that entry's state (ie, if it was selected, it will be deselected). Because
		 * of this, selecting the same entry in the dropdown list essentially sets the list's selected row to -1, causing problems.
		 *
		 * In order to work around this, we first manually deselect the selected entry here. This handler is called *before*
		 * the listbox's click handler, and as such the selected item will remain toggled on when the click handler fires.
		 */
		listbox& list = find_widget<listbox>(&window, "list", true);
		const int sel = list.get_selected_row();
		if(sel >= 0) {
			list.select_row(sel, false);
		}

		// Disregard clicks if they're on the scrollbar
		// This check works since this function is called before scrollbar_base's left-button-up handler
		if(list.vertical_scrollbar()->get_state() == scrollbar_base::PRESSED) {
			return;
		}

		SDL_Rect rect = window.get_rectangle();
		if(!sdl::point_in_rect(coordinate, rect)) {
			window.set_retval(window::CANCEL);
		} else {
			window.set_retval(window::OK);
		}
	}

	void resize_callback(window& window)
	{
		window.set_retval(window::CANCEL);
		window.close();
	}
}

void drop_down_menu::pre_show(window& window)
{
	window.set_variable("button_x", variant(button_pos_.x));
	window.set_variable("button_y", variant(button_pos_.y));
	window.set_variable("button_w", variant(button_pos_.w));
	window.set_variable("button_h", variant(button_pos_.h));

	listbox& list = find_widget<listbox>(&window, "list", true);

	std::map<std::string, string_map> data;
	string_map item;

	for(const auto& entry : items_) {
		data.clear();

		item["label"] = entry["icon"];
		data.emplace("icon", item);

		if(!entry.has_attribute("image")) {
			item["label"] = entry["label"];
			item["use_markup"] = use_markup_ ? "true" : "false";
			data.emplace("label", item);
		}

		if(entry.has_attribute("details")) {
			item["label"] = entry["details"];
			item["use_markup"] = use_markup_ ? "true" : "false";
			data.emplace("details", item);
		}

		grid& new_row = list.add_row(data);

		// Set the tooltip on the whole panel
		find_widget<toggle_panel>(&new_row, "panel", false).set_tooltip(entry["tooltip"]);

		if(entry.has_attribute("image")) {
			image* img = new image;
			img->set_definition("default");
			img->set_label(entry["image"]);

			grid* mi_grid = dynamic_cast<grid*>(new_row.find("menu_item", false));
			if(mi_grid) {
				delete mi_grid->swap_child("label", img, false);
			}
		}
	}

	if(selected_item_ >= 0 && unsigned(selected_item_) < list.get_item_count()) {
		list.select_row(selected_item_);
	}

	window.keyboard_capture(&list);

	// Dismiss on click outside the window
	window.connect_signal<event::SDL_LEFT_BUTTON_UP>(std::bind(&click_callback, std::ref(window), _3, _4, _5), event::dispatcher::front_child);

	// Dismiss on resize
	window.connect_signal<event::SDL_VIDEO_RESIZE>(std::bind(&resize_callback, std::ref(window)), event::dispatcher::front_child);
}

void drop_down_menu::post_show(window& window)
{
	selected_item_ = find_widget<listbox>(&window, "list", true).get_selected_row();
}

} // namespace dialogs
} // namespace gui2
