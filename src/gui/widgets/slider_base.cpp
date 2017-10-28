/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/slider_base.hpp"

#include "gui/core/log.hpp"
#include "gui/widgets/window.hpp" // Needed for invalidate_layout()

#include "utils/functional.hpp"
#include "utils/math.hpp"

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace {
	int rounded_division(int value, int new_base, int old_base)
	{
		if (old_base == 0) {
			return new_base / 2;
		}
		else {
			return ::rounded_division(value * new_base, old_base);
		}
	}
}
namespace gui2
{

slider_base::slider_base(const implementation::builder_styled_widget& builder, const std::string& control_type)
	: styled_widget(builder, control_type)
	, state_(ENABLED)
	, item_last_(0)
	, item_position_(0)
	, drag_initial_mouse_(0, 0)
	, drag_initial_position_(0)
	, drag_initial_offset_(0)
	, positioner_offset_(0)
	, positioner_length_(0)
	, snap_(true)
{
	connect_signal<event::MOUSE_ENTER>(std::bind(
			&slider_base::signal_handler_mouse_enter, this, _2, _3, _4));
	connect_signal<event::MOUSE_MOTION>(std::bind(
			&slider_base::signal_handler_mouse_motion, this, _2, _3, _4, _5));
	connect_signal<event::MOUSE_LEAVE>(std::bind(
			&slider_base::signal_handler_mouse_leave, this, _2, _3));
	connect_signal<event::LEFT_BUTTON_DOWN>(std::bind(
			&slider_base::signal_handler_left_button_down, this, _2, _3));
	connect_signal<event::LEFT_BUTTON_UP>(std::bind(
			&slider_base::signal_handler_left_button_up, this, _2, _3));
}

void slider_base::scroll(const scroll_mode scroll)
{
	switch(scroll) {
		case BEGIN:
			set_slider_position(0);
			break;

		case ITEM_BACKWARDS:
			set_slider_position(item_position_ - 1);
			break;

		case HALF_JUMP_BACKWARDS:
			set_slider_position(item_position_  - jump_size() / 2);
			break;

		case JUMP_BACKWARDS:
			set_slider_position(item_position_ - jump_size());
			break;

		case END:
			set_slider_position(item_last_);
			break;

		case ITEM_FORWARD:
			set_slider_position(item_position_ + 1);
			break;

		case HALF_JUMP_FORWARD:
			set_slider_position(item_position_ + jump_size() / 2);
			break;

		case JUMP_FORWARD:
			set_slider_position(item_position_ + jump_size());
			break;

		default:
			assert(false);
	}

	fire(event::NOTIFY_MODIFIED, *this, nullptr);
}

void slider_base::place(const point& origin, const point& size)
{
	// Inherited.
	styled_widget::place(origin, size);

	recalculate();
}

void slider_base::set_active(const bool active)
{
	if(get_active() != active) {
		set_state(active ? ENABLED : DISABLED);
	}
}

bool slider_base::get_active() const
{
	return state_ != DISABLED;
}

unsigned slider_base::get_state() const
{
	return state_;
}

void slider_base::set_slider_position(int item_position)
{
	// Set the value always execute since we update a part of the state.
	item_position_ = utils::clamp(item_position, 0, item_last_);

	// Determine the pixel offset of the item position.
	positioner_offset_ = rounded_division(item_position_, max_offset(), item_last_) + offset_before();

	update_canvas();
}

void slider_base::update_canvas()
{

	for(auto & tmp : get_canvases())
	{
		tmp.set_variable("positioner_offset", wfl::variant(positioner_offset_));
		tmp.set_variable("positioner_length", wfl::variant(positioner_length_));
	}
	set_is_dirty(true);
}

void slider_base::set_state(const state_t state)
{
	if(state != state_) {
		state_ = state;
		set_is_dirty(true);
	}
}

void slider_base::recalculate()
{
	// We can be called before the size has been set up in that case we can't do
	// the proper recalcultion so stop before we die with an assert.
	if(!get_length()) {
		return;
	}
	assert(available_length() > 0);

	recalculate_positioner();

	set_slider_position(item_position_);

}

void slider_base::move_positioner(int new_offset)
{
	int max_offset = this->max_offset();
	new_offset = utils::clamp(new_offset, 0, max_offset);

	slider_base::slider_position_t final_offset = {new_offset, max_offset};
	update_slider_position(final_offset);

	assert(final_offset.max_offset == max_offset);
	positioner_offset_ = final_offset.offset + offset_before();

	update_canvas();
}

void slider_base::update_slider_position(slider_base::slider_position_t& pos)
{
	int new_position = rounded_division(pos.offset, item_last_, pos.max_offset);
	
	if(snap_) {
		pos.offset = rounded_division(new_position, pos.max_offset, item_last_);
	}

	if(new_position != item_position_) {
		
		item_position_ = new_position;

		child_callback_positioner_moved();

		fire(event::NOTIFY_MODIFIED, *this, nullptr);
	}
}

void slider_base::signal_handler_mouse_enter(const event::ui_event event,
											 bool& handled,
											 bool& halt)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	// Send the motion under our event id to make debugging easier.
	signal_handler_mouse_motion(event, handled, halt, get_mouse_position());
}

void slider_base::signal_handler_mouse_motion(const event::ui_event event,
											  bool& handled,
											  bool& halt,
											  const point& coordinate)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << " at " << coordinate << ".\n";

	point mouse = coordinate;
	mouse.x -= get_x();
	mouse.y -= get_y();

	switch(state_) {
		case ENABLED:
			if(on_positioner(mouse)) {
				set_state(FOCUSED);
			}

			break;

		case PRESSED: {
			move_positioner(get_length_difference(drag_initial_mouse_, mouse) + drag_initial_offset_);

		} break;

		case FOCUSED:
			if(!on_positioner(mouse)) {
				set_state(ENABLED);
			}
			break;

		case DISABLED:
			// Shouldn't be possible, but seems to happen in the lobby
			// if a resize layout happens during dragging.
			halt = true;
			break;

		default:
			assert(false);
	}
	handled = true;
}

void slider_base::signal_handler_mouse_leave(const event::ui_event event,
											 bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	if(state_ == FOCUSED) {
		set_state(ENABLED);
	}
	handled = true;
}


void slider_base::signal_handler_left_button_down(const event::ui_event event,
												  bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	point mouse = get_mouse_position();
	mouse.x -= get_x();
	mouse.y -= get_y();

	if(on_positioner(mouse)) {
		assert(get_window());
		drag_initial_mouse_ = mouse;
		drag_initial_position_ = item_position_;
		drag_initial_offset_ = positioner_offset_ - offset_before();

		get_window()->mouse_capture();
		set_state(PRESSED);
	}

	const int bar = on_bar(mouse);

	if(bar == -1) {
		scroll(JUMP_BACKWARDS);
	} else if(bar == 1) {
		scroll(JUMP_FORWARD);
	} else {
		assert(bar == 0);
	}

	handled = true;
}

void slider_base::signal_handler_left_button_up(const event::ui_event event,
												bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	point mouse = get_mouse_position();
	mouse.x -= get_x();
	mouse.y -= get_y();

	if(state_ != PRESSED) {
		return;
	}

	assert(get_window());
	get_window()->mouse_capture(false);

	if(on_positioner(mouse)) {
		set_state(FOCUSED);
	} else {
		set_state(ENABLED);
	}

	drag_initial_mouse_ = {0, 0};
	drag_initial_position_ = 0;
	drag_initial_offset_ = 0;

	handled = true;
}

void slider_base::finalize_setup()
{
	// These values won't change so set them once.
	for(auto& tmp : get_canvases()) {
		tmp.set_variable("offset_before", wfl::variant(offset_before()));
		tmp.set_variable("offset_after", wfl::variant(offset_after()));
	}
}

} // namespace gui2
