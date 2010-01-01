/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/menubar.hpp"

#include "gui/widgets/selectable.hpp"

namespace gui2 {

static void callback_select_item(twidget* caller)
{
	get_parent<tmenubar>(caller)->item_selected(caller);
}

size_t tmenubar::get_item_count() const
{
	if(direction_ == VERTICAL) {
		return grid().get_rows();
	} else {
		return grid().get_cols();
	}
}

void tmenubar::set_must_select(const bool must_select)
{
	assert(!must_select || selected_item_ != -1);

	must_select_ = must_select;
}

void tmenubar::set_selected_item(const int item)
{
	assert(!must_select_ || item != -1);

	if(item == selected_item_) {
		return;
	}

	if(selected_item_ != -1) {
		(*(*this)[selected_item_]).set_value(false);
	}

	selected_item_ = item;
	if(selected_item_ != -1) {
		(*(*this)[selected_item_]).set_value(true);
	}
}

void tmenubar::set_state(const tstate state)
{
	if(state != state_) {
		state_ = state;
		set_dirty(true);
	}
}

void tmenubar::item_selected(twidget* widget)
{
	//std::cerr << "click.\n";

	tselectable_* item = dynamic_cast<tselectable_*>(widget);
	assert(item);

	// Find the widget clicked upon.
	size_t index = 0;
	for(; index < get_item_count(); ++index) {

		if((*this)[index] == item) {
			break;
		}
	}
	assert(index < get_item_count());

	// Set the selection.
	if(!item->get_value()) {
		// Deselected an item, allowed?
		if(must_select_) {
			item->set_value(true);
		} else {
			selected_item_ = -1;
		}
	} else {
		set_selected_item(index);
	}
}

const tselectable_* tmenubar::operator[](const size_t index) const
{
	assert(index < get_item_count());

	const tselectable_* widget = NULL;
	if(direction_ == VERTICAL) {
		widget = dynamic_cast<const tselectable_*>(grid().widget(index, 0));
	} else {
		widget = dynamic_cast<const tselectable_*>(grid().widget(0, index));
	}

	assert(widget);

	return widget;
}

tselectable_* tmenubar::operator[](const size_t index)
{
	assert(index < get_item_count());

	tselectable_* widget = NULL;
	if(direction_ == VERTICAL) {
		widget = dynamic_cast<tselectable_*>(grid().widget(index, 0));
	} else {
		widget = dynamic_cast<tselectable_*>(grid().widget(0, index));
	}

	assert(widget);

	return widget;
}

void tmenubar::finalize_setup()
{
	for(unsigned row = 0; row < grid().get_rows(); ++row) {
		for(unsigned col = 0; col < grid().get_cols(); ++col) {
			twidget* widget = grid().widget(row, col);
			assert(widget);

			tselectable_* btn = dynamic_cast<tselectable_*>(widget);

			if(btn) {
				btn->set_callback_state_change(callback_select_item);
			} else {
				//std::cerr << "Widget type " << typeid(*widget).name() << ".\n";
				assert(false);
			}
		}
	}
}

const std::string& tmenubar::get_control_type() const
{
	static const std::string type = "menubar";
	return type;
}
} // namespace gui2

