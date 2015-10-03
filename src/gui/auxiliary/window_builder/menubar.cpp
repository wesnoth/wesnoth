/* $Id$ */
/*
   Copyright (C) 2008 - 2011 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/auxiliary/window_builder/menubar.hpp"

#include "config.hpp"
#include "foreach.hpp"
#include "gui/auxiliary/log.hpp"

namespace gui2 {

namespace implementation {

static tmenubar::tdirection read_direction(const std::string& direction)
{
	if(direction == "vertical") {
		return tmenubar::VERTICAL;
	} else if(direction == "horizontal") {
		return tmenubar::HORIZONTAL;
	} else {
		ERR_GUI_E << "Invalid direction "
				<< direction << "' falling back to 'horizontal'.\n";
		return tmenubar::HORIZONTAL;
	}
}

tbuilder_menubar::tbuilder_menubar(const config& cfg)
	: tbuilder_control(cfg)
	, must_have_one_item_selected_(
			utils::string_bool(cfg["must_have_one_item_selected"]))
	, direction_(read_direction(cfg["direction"]))
	, selected_item_(lexical_cast_default<int>(
			cfg["selected_item"], must_have_one_item_selected_ ? 0 : -1))
	, cells_()
{
	const config &data = cfg.child("data");
	if (!data) return;

	BOOST_FOREACH (const config &cell, data.child_range("cell")) {
		cells_.push_back(tbuilder_gridcell(cell));
	}
}

twidget* tbuilder_menubar::build() const
{
	tmenubar* widget = new tmenubar(direction_);

	init_control(widget);

	DBG_GUI_G << "Window builder: placed menubar '"
		<< id << "' with defintion '"
		<< definition << "'.\n";

	if(direction_ == tmenubar::HORIZONTAL) {
		widget->set_rows_cols(1, cells_.size());

		for(size_t i = 0; i < cells_.size(); ++i) {
			widget->set_child(cells_[i].widget->build(),
				0, i, cells_[i].flags, cells_[i].border_size);
		}
	} else {
		// vertical growth
		widget->set_rows_cols(cells_.size(), 1);

		for(size_t i = 0; i < cells_.size(); ++i) {
			widget->set_child(cells_[i].widget->build(),
				i, 0, cells_[i].flags, cells_[i].border_size);
		}
	}

	widget->set_selected_item(selected_item_);
	widget->set_must_select(must_have_one_item_selected_);
	widget->finalize_setup();

	return widget;
}

} // namespace implementation

} // namespace gui2

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_menubar
 *
 * == Menubar ==
 *
 * A menu bar used for menus and tab controls.
 *
 * List with the listbox specific variables:
 * @start_table = config
 *     must_have_one_item_selected (bool = false)
 *                                     Does the menu always have one item
 *                                     selected. This makes sense for
 *                                     tabsheets
 *                                     but not for menus.
 *     direction (direction = "")      The direction of the menubar.
 *     selected_item(int = -1)         The item to select upon creation, when
 *                                     'must_have_one_item_selected' is true
 *                                     the default value is 0 instead of -1.
 *                                     -1 means no item selected.
 * @end_table
 */

