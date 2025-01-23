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

#pragma once

#include "gui/dialogs/modal_dialog.hpp"
#include "utils/optional_fwd.hpp"

#include <boost/dynamic_bitset.hpp>


class config;

namespace gui2
{
class styled_widget;

namespace dialogs
{
/** Used by the menu_button widget. */
class drop_down_menu : public modal_dialog
{
public:
	/** Menu was invoked from a widget (currently a [multi]menu_button). Its position and markup settings will be derived from there. */
	drop_down_menu(styled_widget* parent, const std::vector<config>& items, int selected_item, bool keep_open);

	/** Menu was invoked manually. Position and markup settings must be provided here. */
	drop_down_menu(SDL_Rect button_pos, const std::vector<config>& items, int selected_item, bool use_markup, bool keep_open);

	int selected_item() const
	{
		return selected_item_;
	}

	point selected_item_pos() const
	{
		return selected_item_pos_;
	}

	/** If a toggle button widget is present, returns the toggled state of each row's button. */
	boost::dynamic_bitset<> get_toggle_states() const;

private:
	// TODO: evaluate exposing this publically via the [multi]menu_button widgets
	struct entry_data
	{
		entry_data(const config& cfg);

		/** If present, column 1 will have a toggle button. The value indicates its initial state. */
		utils::optional<bool> checkbox;

		/** If no checkbox is present, the icon at this path will be shown in column 1. */
		std::string icon;

		/** Is present, column 2 will display the image at this path. */
		utils::optional<std::string> image;

		/** If no image is present, this text will be shown in column 2. */
		t_string label;

		/** If present, this text will be shown in column 3. */
		utils::optional<t_string> details;

		/** Tooltip text for the entire row. */
		t_string tooltip;
	};

	/** The widget that invoked this dialog, if applicable. */
	styled_widget* parent_;

	/** Configuration of each row. */
	std::vector<entry_data> items_;

	/**
	 * The screen location of the menu_button button that triggered this droplist.
	 * Note: we don't adjust the location of this dialog to when resizing the window.
	 * Instead this dialog automatically closes itself on resizing.
	 */
	SDL_Rect button_pos_;

	int selected_item_;
	point selected_item_pos_;

	bool use_markup_;

	/**
	 * Whether to keep this dialog open after a click occurs not handled by special exceptions
	 * such as scrollbars and toggle buttons.
	 */
	bool keep_open_;

	/**
	 * When menu is invoked on a long-touch timer, a following mouse-up event will close it.
	 * This flag prevents that: the menu will only be closed on a mouse-up that follows a mouse-down.
	 * */
	bool mouse_down_happened_;

	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	virtual void post_show() override;

	void mouse_up_callback(bool&, bool&, const point& coordinate);

	void mouse_down_callback();
};

} // namespace dialogs
} // namespace gui2
