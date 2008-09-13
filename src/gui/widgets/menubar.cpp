/* $Id$ */
/*
   copyright (C) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#include "gui/widgets/menubar.hpp"

#include "foreach.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "log.hpp"

#define DBG_G LOG_STREAM_INDENT(debug, gui)
#define LOG_G LOG_STREAM_INDENT(info, gui)
#define WRN_G LOG_STREAM_INDENT(warn, gui)
#define ERR_G LOG_STREAM_INDENT(err, gui)

#define DBG_G_D LOG_STREAM_INDENT(debug, gui_draw)
#define LOG_G_D LOG_STREAM_INDENT(info, gui_draw)
#define WRN_G_D LOG_STREAM_INDENT(warn, gui_draw)
#define ERR_G_D LOG_STREAM_INDENT(err, gui_draw)

#define DBG_G_E LOG_STREAM_INDENT(debug, gui_event)
#define LOG_G_E LOG_STREAM_INDENT(info, gui_event)
#define WRN_G_E LOG_STREAM_INDENT(warn, gui_event)
#define ERR_G_E LOG_STREAM_INDENT(err, gui_event)

#define DBG_G_P LOG_STREAM_INDENT(debug, gui_parse)
#define LOG_G_P LOG_STREAM_INDENT(info, gui_parse)
#define WRN_G_P LOG_STREAM_INDENT(warn, gui_parse)
#define ERR_G_P LOG_STREAM_INDENT(err, gui_parse)

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
	std::cerr << "click.\n";

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
				std::cerr << "Widget type " << typeid(*widget).name() << ".\n";
				assert(false);
			}
		}
	}
}

} // namespace gui2

