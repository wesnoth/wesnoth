/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/drop_down_menu.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/integer_selector.hpp"
#include "gui/widgets/scrollbar.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "gui/widgets/window.hpp"

#include "sdl/rect.hpp"
#include "utils/functional.hpp"

namespace gui2
{
namespace dialogs
{
REGISTER_DIALOG(drop_down_menu)

namespace
{
	void callback_flip_embedded_toggle(window& window)
	{
		listbox& list = find_widget<listbox>(&window, "list", true);

		/* If the currently selected row has a toggle button, toggle it.
		 * Note this cannot be handled in mouse_up_callback since at that point the new row selection has not registered,
		 * meaning the currently selected row's button is toggled.
		 */
		grid* row_grid = list.get_row_grid(list.get_selected_row());
		if(toggle_button* checkbox = find_widget<toggle_button>(row_grid, "checkbox", false, false)) {
			checkbox->set_value_bool(!checkbox->get_value_bool(), true);
		}
	}

	void resize_callback(window& window)
	{
		window.set_retval(retval::CANCEL);
	}
}

void drop_down_menu::mouse_up_callback(window& window, bool&, bool&, const point& coordinate)
{
	if(!mouse_down_happened_) {
		return;
	}

	listbox& list = find_widget<listbox>(&window, "list", true);

	/* Disregard clicks on scrollbars and toggle buttons so the dropdown menu can be scrolled or have an embedded
	 * toggle button selected without the menu closing.
	 *
	 * This works since this mouse_up_callback function is called before widgets' left-button-up handlers.
	 *
	 * Additionally, this is done before row deselection so selecting/deselecting a toggle button doesn't also leave
	 * the list with no row visually selected. Oddly, the visial deselection doesn't seem to cause any crashes, and
	 * the previously selected row is reselected when the menu is opened again. Still, it's odd to see your selection
	 * vanish.
	 */
	if(list.vertical_scrollbar()->get_state() == scrollbar_base::PRESSED) {
		return;
	}

	if(dynamic_cast<toggle_button*>(window.find_at(coordinate, true)) != nullptr) {
		return;
	}

	/* FIXME: This dialog uses a listbox with 'has_minimum = false'. This allows a listbox to have 0 or 1 selections,
	 * and selecting the same entry toggles that entry's state (ie, if it was selected, it will be deselected). Because
	 * of this, selecting the same entry in the dropdown list essentially sets the list's selected row to -1, causing problems.
	 *
	 * In order to work around this, we first manually deselect the selected entry here. This handler is called *before*
	 * the listbox's click handler, and as such the selected item will remain toggled on when the click handler fires.
	 */
	const int sel = list.get_selected_row();
	if(sel >= 0) {
		list.select_row(sel, false);
	}

	SDL_Rect rect = window.get_rectangle();
	if(!sdl::point_in_rect(coordinate, rect)) {
		window.set_retval(retval::CANCEL);
	} else if(!keep_open_) {
		window.set_retval(retval::OK);
	}
}

void drop_down_menu::mouse_down_callback()
{
	mouse_down_happened_ = true;
}

void drop_down_menu::pre_show(window& window)
{
	window.set_variable("button_x", wfl::variant(button_pos_.x));
	window.set_variable("button_y", wfl::variant(button_pos_.y));
	window.set_variable("button_w", wfl::variant(button_pos_.w));
	window.set_variable("button_h", wfl::variant(button_pos_.h));

	listbox& list = find_widget<listbox>(&window, "list", true);

	/* Each row's config can have the following keys. Note the containing [entry]
	 * tag is for illustrative purposes and isn't actually present in the configs.
	 *
	 * [entry]
	 *     icon = "path/to/image.png"          | Column 1 OR
	 *     checkbox = yes/no                   | Column 1
	 *     image = "path/to/image.png"         | Column 2 OR
	 *     label = "text"                      | Column 2
	 *     details = "text"                    | Column 3
	 *     tooltip = "text"                    | Entire row
	 * [/entry]
	 */
	for(const auto& entry : items_) {
		std::map<std::string, string_map> data;
		string_map item;

		const bool has_image_key = entry.has_attribute("image");
		const bool has_ckbox_key = entry.has_attribute("checkbox");

		//
		// These widgets can be initialized here since they don't require widget type swapping.
		//
		item["use_markup"] = utils::bool_string(use_markup_);

		if(!has_ckbox_key) {
			item["label"] = entry["icon"];
			data.emplace("icon", item);
		}

		if(!has_image_key) {
			item["label"] = entry["label"];
			data.emplace("label", item);
		}

		if(entry.has_attribute("details")) {
			item["label"] = entry["details"];
			data.emplace("details", item);
		}

		grid& new_row = list.add_row(data);
		grid& mi_grid = find_widget<grid>(&new_row, "menu_item", false);

		// Set the tooltip on the whole panel
		find_widget<toggle_panel>(&new_row, "panel", false).set_tooltip(entry["tooltip"]);

		if(has_ckbox_key) {
			toggle_button* checkbox = build_single_widget_instance<toggle_button>();
			checkbox->set_id("checkbox");
			checkbox->set_value_bool(entry["checkbox"].to_bool(false));

			if(callback_toggle_state_change_ != nullptr) {
				connect_signal_notify_modified(*checkbox, std::bind(callback_toggle_state_change_));
			}

			mi_grid.swap_child("icon", checkbox, false);
		}

		if(has_image_key) {
			image* img = build_single_widget_instance<image>();
			img->set_label(entry["image"]);

			mi_grid.swap_child("label", img, false);
		}
	}

	if(selected_item_ >= 0 && static_cast<unsigned>(selected_item_) < list.get_item_count()) {
		list.select_row(selected_item_);
	}

	window.keyboard_capture(&list);

	// Dismiss on clicking outside the window.
	window.connect_signal<event::SDL_LEFT_BUTTON_UP>(
		std::bind(&drop_down_menu::mouse_up_callback, this, std::ref(window), _3, _4, _5), event::dispatcher::front_child);

	window.connect_signal<event::SDL_RIGHT_BUTTON_UP>(
		std::bind(&drop_down_menu::mouse_up_callback, this, std::ref(window), _3, _4, _5), event::dispatcher::front_child);

	window.connect_signal<event::SDL_LEFT_BUTTON_DOWN>(
		std::bind(&drop_down_menu::mouse_down_callback, this), event::dispatcher::front_child);

	// Dismiss on resize.
	window.connect_signal<event::SDL_VIDEO_RESIZE>(
		std::bind(&resize_callback, std::ref(window)), event::dispatcher::front_child);

	// Handle embedded button toggling.
	connect_signal_notify_modified(list,
		std::bind(&callback_flip_embedded_toggle, std::ref(window)));
}

void drop_down_menu::post_show(window& window)
{
	selected_item_ = find_widget<listbox>(&window, "list", true).get_selected_row();
}

boost::dynamic_bitset<> drop_down_menu::get_toggle_states() const
{
	const listbox& list = find_widget<const listbox>(get_window(), "list", true);

	boost::dynamic_bitset<> states;

	for(unsigned i = 0; i < list.get_item_count(); ++i) {
		const grid* row_grid = list.get_row_grid(i);

		if(const toggle_button* checkbox = find_widget<const toggle_button>(row_grid, "checkbox", false, false)) {
			states.push_back(checkbox->get_value_bool());
		} else {
			states.push_back(false);
		}
	}

	return states;
}

} // namespace dialogs
} // namespace gui2
