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

#include "gui/widgets/vertical_scrollbar_container.hpp"

#include "foreach.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/scrollbar.hpp"
#include "gui/widgets/window.hpp"
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

void callback_scrollbar_button(twidget* caller)
{
	get_parent<tvertical_scrollbar_container_>(caller)->scrollbar_click(caller);
}

void callback_scrollbar(twidget* caller)
{
	get_parent<tvertical_scrollbar_container_>(caller)->scrollbar_moved(caller);
}

namespace {

	static const std::string button_up_names[] = {
		"_begin", "_line_up", "_half_page_up", "_page_up" };

	static const std::string button_down_names[] = {
		"_end", "_line_down", "_half_page_down", "_page_down" };

/**
 * Returns a map with the names of all buttons and the scrollbar jump they're
 * supposed to execute.
 */
const std::map<std::string, tscrollbar_::tscroll>& scroll_lookup()
{
	static std::map<std::string, tscrollbar_::tscroll> lookup;
	if(lookup.empty()) {
		lookup["_begin"] = tscrollbar_::BEGIN;
		lookup["_line_up"] = tscrollbar_::ITEM_BACKWARDS;
		lookup["_half_page_up"] = tscrollbar_::HALF_JUMP_BACKWARDS;
		lookup["_page_up"] = tscrollbar_::JUMP_BACKWARDS;

		lookup["_end"] = tscrollbar_::END;
		lookup["_line_down"] = tscrollbar_::ITEM_FORWARD;
		lookup["_half_page_down"] = tscrollbar_::HALF_JUMP_FORWARD;
		lookup["_page_down"] = tscrollbar_::JUMP_FORWARD;
	}

	return lookup;
}

} // namespace

void tvertical_scrollbar_container_::key_press(tevent_handler& /*event*/, 
		bool& handled, SDLKey key, SDLMod /*modifier*/, Uint16 /*unicode*/)
{
	DBG_G_E << "Listbox: key press.\n";

	tscrollbar_* sb = find_scrollbar();
	int row = get_selected_row();
	switch(key) {

		case SDLK_PAGEUP :
			row -= sb->get_visible_items() - 1;
			if(row <= 0) {
				row = 1;
			}
			// FALL DOWN

		case SDLK_UP : 

			--row;
			while(row >= 0 && !get_item_active(row)) {
				--row;
			}
			if(row >= 0) {
				select_row(row);
				handled = true;

				if(row < sb->get_item_position()) {
					sb->set_item_position(row);
					set_scrollbar_button_status();
				}
				value_changed();
			}
			break;
			
		case SDLK_PAGEDOWN :
			row += sb->get_visible_items() - 1;
			if(row + 1 >= sb->get_item_count()) {
				row = sb->get_item_count() - 2;
			}
			// FALL DOWN

		case SDLK_DOWN : 

			++row;
			while(row < sb->get_item_count() && !get_item_active(row)) {
				++row;
			}
			if(row < sb->get_item_count()) {
				select_row(row);
				handled = true;
				if(row >= sb->get_item_position() + sb->get_visible_items()) {

					sb->set_item_position(row + 1 - sb->get_visible_items());
					set_scrollbar_button_status();
				}
				value_changed();
			}
			break;

		default :
			/* DO NOTHING */
			break;
	}
}

void tvertical_scrollbar_container_::value_changed()
{
	if(callback_value_change_) {
		callback_value_change_(this);
	}
}

tscrollbar_* tvertical_scrollbar_container_::find_scrollbar(const bool must_exist)
{
    return find_widget<tscrollbar_>("_scrollbar", false, must_exist);
}

const tscrollbar_* tvertical_scrollbar_container_::find_scrollbar(
		const bool must_exist) const
{
    return find_widget<const tscrollbar_>("_scrollbar", false, must_exist);
}

void tvertical_scrollbar_container_::set_scrollbar_button_status()
{
	// Set scroll up button status
	foreach(const std::string& name, button_up_names) {
		tbutton* button = 
			dynamic_cast<tbutton*>(tcontainer_::find_widget(name, false));

		if(button) {
			button->set_active(!find_scrollbar()->at_begin());
		}
	}

	// Set scroll down button status
	foreach(const std::string& name, button_down_names) {
		tbutton* button = 
			dynamic_cast<tbutton*>(tcontainer_::find_widget(name, false));

		if(button) {
			button->set_active(!find_scrollbar()->at_end());
		}
	}

	// Set the scrollbar itself
	find_scrollbar()->set_active(
		!(find_scrollbar()->at_begin() && find_scrollbar()->at_end()));
}

void tvertical_scrollbar_container_::finalize_setup()
{
	find_scrollbar()->set_callback_positioner_move(callback_scrollbar);

	// typedef boost problem work around.
	typedef std::pair<std::string, tscrollbar_::tscroll> hack;
	foreach(const hack& item, scroll_lookup()) {

		tbutton* button = 
			dynamic_cast<tbutton*>(tcontainer_::find_widget(item.first, false));

		if(button) {
			button->set_callback_mouse_left_click(callback_scrollbar_button);
		}
	}
}

void tvertical_scrollbar_container_::scrollbar_click(twidget* caller)
{
	/** @todo Hack to capture the keyboard focus. */
	get_window()->keyboard_capture(this);

	const std::map<std::string, tscrollbar_::tscroll>::const_iterator 
		itor = scroll_lookup().find(caller->id());

	assert(itor != scroll_lookup().end());
	find_scrollbar()->scroll(itor->second);

	set_scrollbar_button_status();
}

unsigned tvertical_scrollbar_container_::get_selected_row() const
{
	return find_scrollbar()->get_item_position();
}
} // namespace gui2

