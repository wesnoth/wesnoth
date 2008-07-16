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

#include "gui/widgets/scrollbar.hpp"

#include "foreach.hpp"
#include "gui/widgets/event_handler.hpp"
#include "log.hpp"

#include <cassert>


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

void tscrollbar_::scroll(const tscroll scroll)
{
	switch(scroll) {
		case BEGIN : 
			set_item_position(0);
			break;

		case ITEM_BACKWARDS :
			if(item_position_) {
				set_item_position(item_position_ - 1);
			}
			break;

		case HALF_JUMP_BACKWARDS :
			set_item_position(item_position_ > (visible_items_ / 2) ? 
				item_position_ - (visible_items_ / 2) : 0);
			break;

		case JUMP_BACKWARDS :
			set_item_position(item_position_ > visible_items_ ? 
				item_position_ - visible_items_  : 0);
			break;

		case END :
			set_item_position(item_count_ - 1);
			break;
			
		case ITEM_FORWARD :
			set_item_position(item_position_ + 1);
			break;

		case HALF_JUMP_FORWARD :
			set_item_position(item_position_ +  (visible_items_ / 2));
			break;

		case JUMP_FORWARD :
			set_item_position(item_position_ +  visible_items_ );
			break;

		default :
			assert(false);
		}
}

void tscrollbar_::mouse_move(tevent_handler& event)
{
	tpoint mouse = event.get_mouse();
	mouse.x -= get_x();
	mouse.y -= get_y();

	DBG_G_E << "Scrollbar: mouse move at " << mouse << ".\n";

	switch(state_) {
		case ENABLED :
			if(on_positioner(mouse)) {
				set_state(FOCUSSED);
			}

			break;
		case DISABLED :
			// do nothing
			break;

		case PRESSED : {
				const int distance = get_length_difference(mouse_, mouse);
				mouse_ = mouse;
				move_positioner(distance);
			}
			break; 

		case FOCUSSED :
			if(!on_positioner(mouse)) {
				set_state(ENABLED);
			}
			break;

		default :
			assert(false);
	}
}

void tscrollbar_::mouse_leave(tevent_handler&)
{
	DBG_G_E << "Scrollbar: mouse leave.\n";

	if(state_ == FOCUSSED) {
		set_state(ENABLED);
	}
}

void tscrollbar_::mouse_left_button_down(tevent_handler& event)
{
	tpoint mouse = event.get_mouse();
	mouse.x -= get_x();
	mouse.y -= get_y();

	DBG_G_E << "Scrollbar: mouse down at " << mouse << ".\n";

	if(on_positioner(mouse)) {
		mouse_ = mouse;
		event.mouse_capture();
		set_state(PRESSED);
	}
}

void tscrollbar_::mouse_left_button_up(tevent_handler& event)
{
	tpoint mouse = event.get_mouse();
	mouse.x -= get_x();
	mouse.y -= get_y();

	DBG_G_E << "Scrollbar: mouse up at " << mouse << ".\n";

	if(state_ != PRESSED) {
		return;
	}

	event.mouse_capture(false);

	if(on_positioner(mouse)) {
		set_state(FOCUSSED);
	} else {
		set_state(ENABLED);
	}
}

void tscrollbar_::set_size(const SDL_Rect& rect)
{
	// Inherited.
	tcontrol::set_size(rect);

	recalculate();
}

void tscrollbar_::set_item_position(const unsigned item_position)
{
	// Set the value always execute since we update a part of the state.
	item_position_ = item_position + visible_items_ > item_count_ ? 
		item_count_ - visible_items_ : item_position;

	item_position_ = (item_position_ + step_size_ - 1) / step_size_;

	// Determine the pixel offset of the item position.
	positioner_offset_ = static_cast<unsigned>(item_position_ * pixels_per_step_);

	update_canvas();
}

void tscrollbar_::set_state(const tstate state)
{
	if(state != state_) {
		state_ = state;
		set_dirty(true);
	}
}

void tscrollbar_::recalculate()
{
	// We can be called before the size has been set up in that case we can't do
	// the proper recalcultion so stop before we die with an assert.
	if(!get_length()) {
		return;
	}

	// Get the available size for the slider to move.
	int available_length = 
		get_length() - offset_before() - minimum_positioner_length() - offset_after();

	assert(available_length > 0);

	// All visible.
	if(item_count_ <= visible_items_) {
		positioner_offset_ = offset_before();
		positioner_length_ = available_length + minimum_positioner_length();
		item_position_ = 0;
		update_canvas();
		return;
	}

	assert(step_size_);
	assert(visible_items_);

	const unsigned steps = (item_count_ + step_size_ - 1) / step_size_;

	if(static_cast<int>(steps) < available_length) {

		// We can show them all.
		available_length += minimum_positioner_length();

		pixels_per_step_ = available_length / static_cast<float>(steps);

		positioner_length_ = static_cast<unsigned>(pixels_per_step_ * visible_items_) + available_length % steps;

	} else {

		// We'll skip some.
		WRN_G << "The scrollbar is too small for the"
			" number of items, movement might seem jerky.\n";
		
		available_length += minimum_positioner_length();

		pixels_per_step_ = available_length / static_cast<float>(steps);

		// When a listbox is in 'pixel mode' (not fixed row height) the
		// positioner might still be rather big so scale it proportional but
		// respect the minimum.
		positioner_length_ = std::max(
			available_length * visible_items_ / item_count_, 
			minimum_positioner_length());
	}

	set_item_position(item_position_);
}

void tscrollbar_::update_canvas() {

	foreach(tcanvas& tmp, canvas()) {
		tmp.set_variable("positioner_offset", variant(positioner_offset_));
		tmp.set_variable("positioner_length", variant(positioner_length_));
	}
	set_dirty();
}

void tscrollbar_::move_positioner(const int distance)
{
  if(distance < 0 && -distance > static_cast<int>(positioner_offset_)) {
		positioner_offset_ = 0;
	} else {
		positioner_offset_ += distance;
	}
	const unsigned length = get_length() - offset_before() - offset_after();

	if(positioner_offset_ + positioner_length_ > length) {
		positioner_offset_ = length - positioner_length_;
	}

	const unsigned position =
		static_cast<unsigned>(positioner_offset_ / pixels_per_step_); 

	if(position != item_position_) {
		item_position_ = position; 

		if(callback_positioner_move_) {
			callback_positioner_move_(this);
		}
	}

	update_canvas();
}

void tscrollbar_::load_config_extra()
{
	// These values won't change so set them here.
	foreach(tcanvas& tmp, canvas()) {
		tmp.set_variable("offset_before", variant(offset_before()));
		tmp.set_variable("offset_after", variant(offset_after()));
	}
}

} // namespace gui2

