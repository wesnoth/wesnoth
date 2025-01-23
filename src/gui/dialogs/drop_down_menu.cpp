/*
	Copyright (C) 2008 - 2024
	by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/core/event/dispatcher.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/scrollbar.hpp"
#include "gui/widgets/styled_widget.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "gui/widgets/window.hpp"

#include "sdl/rect.hpp"
#include <functional>

namespace gui2::dialogs
{
REGISTER_DIALOG(drop_down_menu)

drop_down_menu::entry_data::entry_data(const config& cfg)
	: checkbox()
	, icon(cfg["icon"].str())
	, image()
	, label(cfg["label"].t_str())
	, details()
	, tooltip(cfg["tooltip"].t_str())
{
	// Checkboxes take precedence in column 1
	if(cfg.has_attribute("checkbox")) {
		checkbox = cfg["checkbox"].to_bool(false);
	}

	// Images take precedence in column 2
	if(cfg.has_attribute("image")) {
		image = cfg["image"].str();
	}

	if(cfg.has_attribute("details")) {
		details = cfg["details"].t_str();
	}
}

namespace
{
	void callback_flip_embedded_toggle(window& window)
	{
		listbox& list = window.find_widget<listbox>("list", true);

		/* If the currently selected row has a toggle button, toggle it.
		 * Note this cannot be handled in mouse_up_callback since at that point the new row selection has not registered,
		 * meaning the currently selected row's button is toggled.
		 */
		grid* row_grid = list.get_row_grid(list.get_selected_row());
		if(toggle_button* checkbox = row_grid->find_widget<toggle_button>("checkbox", false, false)) {
			checkbox->set_value_bool(!checkbox->get_value_bool(), true);
		}
	}

	void resize_callback(window& window)
	{
		window.set_retval(retval::CANCEL);
	}
}

drop_down_menu::drop_down_menu(styled_widget* parent, const std::vector<config>& items, int selected_item, bool keep_open)
	: modal_dialog(window_id())
	, parent_(parent)
	, items_(items.begin(), items.end())
	, button_pos_(parent->get_rectangle())
	, selected_item_(selected_item)
	, selected_item_pos_(-1, -1)
	, use_markup_(parent->get_use_markup())
	, keep_open_(keep_open)
	, mouse_down_happened_(false)
{
}

drop_down_menu::drop_down_menu(SDL_Rect button_pos, const std::vector<config>& items, int selected_item, bool use_markup, bool keep_open)
	: modal_dialog(window_id())
	, parent_(nullptr)
	, items_(items.begin(), items.end())
	, button_pos_(button_pos)
	, selected_item_(selected_item)
	, use_markup_(use_markup)
	, keep_open_(keep_open)
	, mouse_down_happened_(false)
{
}

void drop_down_menu::mouse_up_callback(bool&, bool&, const point& coordinate)
{
	if(!mouse_down_happened_) {
		return;
	}

	listbox& list = find_widget<listbox>("list", true);

	/* Disregard clicks on scrollbars and toggle buttons so the dropdown menu can be scrolled or have an embedded
	 * toggle button selected without the menu closing.
	 *
	 * This works since this mouse_up_callback function is called before widgets' left-button-up handlers.
	 *
	 * Additionally, this is done before row deselection so selecting/deselecting a toggle button doesn't also leave
	 * the list with no row visually selected. Oddly, the visual deselection doesn't seem to cause any crashes, and
	 * the previously selected row is reselected when the menu is opened again. Still, it's odd to see your selection
	 * vanish.
	 */
	if(list.vertical_scrollbar()->get_state() == scrollbar_base::PRESSED) {
		return;
	}

	if(dynamic_cast<toggle_button*>(find_at(coordinate, true)) != nullptr) {
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

	if(!get_rectangle().contains(coordinate)) {
		set_retval(retval::CANCEL);
	} else if(!keep_open_) {
		set_retval(retval::OK);
	}
}

void drop_down_menu::mouse_down_callback()
{
	mouse_down_happened_ = true;
}

void drop_down_menu::pre_show()
{
	set_variable("button_x", wfl::variant(button_pos_.x));
	set_variable("button_y", wfl::variant(button_pos_.y));
	set_variable("button_w", wfl::variant(button_pos_.w));
	set_variable("button_h", wfl::variant(button_pos_.h));

	listbox& list = find_widget<listbox>("list", true);

	for(const auto& entry : items_) {
		widget_data data;
		widget_item item;

		//
		// These widgets can be initialized here since they don't require widget type swapping.
		//
		item["use_markup"] = utils::bool_string(use_markup_);
		if(!entry.checkbox) {
			item["label"] = entry.icon;
			data.emplace("icon", item);
		}

		if(!entry.image) {
			item["label"] = entry.label;
			data.emplace("label", item);
		}

		if(entry.details) {
			item["label"] = *entry.details;
			data.emplace("details", item);
		}

		grid& new_row = list.add_row(data);
		grid& mi_grid = new_row.find_widget<grid>("menu_item");

		// Set the tooltip on the whole panel
		new_row.find_widget<toggle_panel>("panel").set_tooltip(entry.tooltip);

		if(entry.checkbox) {
			auto checkbox = build_single_widget_instance<toggle_button>(config{"definition", "no_label"});
			checkbox->set_id("checkbox");
			checkbox->set_value_bool(*entry.checkbox);

			// Fire a NOTIFIED_MODIFIED event in the parent widget when the toggle state changes
			if(parent_) {
				connect_signal_notify_modified(
					*checkbox, [this](auto&&...) { parent_->fire(event::NOTIFY_MODIFIED, *parent_, nullptr); });
			}

			mi_grid.swap_child("icon", std::move(checkbox), false);
		}

		if(entry.image) {
			auto img = build_single_widget_instance<image>();
			img->set_label(*entry.image);

			mi_grid.swap_child("label", std::move(img), false);
		}
	}

	if(selected_item_ >= 0 && static_cast<unsigned>(selected_item_) < list.get_item_count()) {
		list.select_row(selected_item_);
	}

	keyboard_capture(&list);

	// Dismiss on clicking outside the window.
	connect_signal<event::SDL_LEFT_BUTTON_UP>(
		std::bind(&drop_down_menu::mouse_up_callback, this, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5), event::dispatcher::front_child);

	connect_signal<event::SDL_RIGHT_BUTTON_UP>(
		std::bind(&drop_down_menu::mouse_up_callback, this, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5), event::dispatcher::front_child);

	connect_signal<event::SDL_LEFT_BUTTON_DOWN>(
		std::bind(&drop_down_menu::mouse_down_callback, this), event::dispatcher::front_child);

	// Dismiss on resize.
	connect_signal<event::SDL_VIDEO_RESIZE>(
		[this](auto&&...){ resize_callback(*this); }, event::dispatcher::front_child);

	// Handle embedded button toggling.
	connect_signal_notify_modified(list,
		[this](auto&&...){ callback_flip_embedded_toggle(*this); });
}

void drop_down_menu::post_show()
{
	const listbox& list = find_widget<listbox>("list", true);
	selected_item_ = list.get_selected_row();
	if(selected_item_ != -1) {
		const grid* row_grid = list.get_row_grid(selected_item_);
		if(row_grid) {
			selected_item_pos_.x = row_grid->get_x();
			selected_item_pos_.y = row_grid->get_y();
		}
	} else {
		selected_item_pos_.x = selected_item_pos_.y = -1;
	}
}

boost::dynamic_bitset<> drop_down_menu::get_toggle_states() const
{
	const listbox& list = find_widget<const listbox>("list", true);

	boost::dynamic_bitset<> states;

	for(unsigned i = 0; i < list.get_item_count(); ++i) {
		const grid* row_grid = list.get_row_grid(i);

		if(const toggle_button* checkbox = row_grid->find_widget<const toggle_button>("checkbox", false, false)) {
			states.push_back(checkbox->get_value_bool());
		} else {
			states.push_back(false);
		}
	}

	return states;
}

} // namespace dialogs
