/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/scrollbar.hpp"

#include "gui/core/log.hpp"
#include "gui/widgets/window.hpp" // Needed for invalidate_layout()

#include "utils/functional.hpp"

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

scrollbar_base::scrollbar_base(const implementation::builder_styled_widget& builder, const std::string& control_type)
	: styled_widget(builder, control_type)
	, state_(ENABLED)
	, item_count_(0)
	, item_position_(0)
	, visible_items_(1)
	, step_size_(1)
	, pixels_per_step_(0.0)
	, mouse_(0, 0)
	, positioner_offset_(0)
	, positioner_length_(0)
{
	connect_signal<event::MOUSE_ENTER>(std::bind(
			&scrollbar_base::signal_handler_mouse_enter, this, _2, _3, _4));
	connect_signal<event::MOUSE_MOTION>(std::bind(
			&scrollbar_base::signal_handler_mouse_motion, this, _2, _3, _4, _5));
	connect_signal<event::MOUSE_LEAVE>(std::bind(
			&scrollbar_base::signal_handler_mouse_leave, this, _2, _3));
	connect_signal<event::LEFT_BUTTON_DOWN>(std::bind(
			&scrollbar_base::signal_handler_left_button_down, this, _2, _3));
	connect_signal<event::LEFT_BUTTON_UP>(std::bind(
			&scrollbar_base::signal_handler_left_button_up, this, _2, _3));
}

void scrollbar_base::finalize_setup()
{
	// These values won't change so set them once.
	for(auto& tmp : get_canvases()) {
		tmp.set_variable("offset_before", wfl::variant(offset_before()));
		tmp.set_variable("offset_after", wfl::variant(offset_after()));
	}
}

void scrollbar_base::scroll(const scroll_mode scroll)
{
	switch(scroll) {
		case BEGIN:
			set_item_position(0);
			break;

		case ITEM_BACKWARDS:
			if(item_position_) {
				set_item_position(item_position_ - 1);
			}
			break;

		case HALF_JUMP_BACKWARDS:
			set_item_position(item_position_ > (visible_items_ / 2)
									  ? item_position_ - (visible_items_ / 2)
									  : 0);
			break;

		case JUMP_BACKWARDS:
			set_item_position(item_position_ > visible_items_
									  ? item_position_ - visible_items_
									  : 0);
			break;

		case END:
			set_item_position(item_count_ - 1);
			break;

		case ITEM_FORWARD:
			set_item_position(item_position_ + 1);
			break;

		case HALF_JUMP_FORWARD:
			set_item_position(item_position_ + (visible_items_ / 2));
			break;

		case JUMP_FORWARD:
			set_item_position(item_position_ + visible_items_);
			break;

		default:
			assert(false);
	}

	/*
	 * @note I'm not sure if this should be in @ref set_item_position, but right now it
	 * causes weird viewport issues with listboxes. Selecting certain items makes the
	 * viewport to jump to the top, likely having to do with @ref recalculate dispatching
	 * an event (since it calls set_item_position). This seems like a safe spot for now.
	 *
	 * -- vultraz, 2017-09-13
	 */
	fire(event::NOTIFY_MODIFIED, *this, nullptr);
}

void scrollbar_base::place(const point& origin, const point& size)
{
	// Inherited.
	styled_widget::place(origin, size);

	recalculate();
}

void scrollbar_base::set_active(const bool active)
{
	if(get_active() != active) {
		set_state(active ? ENABLED : DISABLED);
	}
}

bool scrollbar_base::get_active() const
{
	return state_ != DISABLED;
}

unsigned scrollbar_base::get_state() const
{
	return state_;
}

void scrollbar_base::set_item_position(const unsigned item_position)
{
	// Set the value always execute since we update a part of the state.
	item_position_ = item_position > item_count_ - visible_items_
							 ? item_count_ - visible_items_
							 : item_position;

	item_position_ = (item_position_ + step_size_ - 1) / step_size_;

	if(all_items_visible()) {
		item_position_ = 0;
	}

	// Determine the pixel offset of the item position.
	positioner_offset_
			= static_cast<unsigned>(item_position_ * pixels_per_step_);

	update_canvas();

	assert(item_position_ <= item_count_ - visible_items_);

#if 0
	/** See comment in @ref scroll for more info. */
	fire(event::NOTIFY_MODIFIED, *this, nullptr);
#endif
}

void scrollbar_base::update_canvas()
{
	for(auto & tmp : get_canvases())
	{
		tmp.set_variable("positioner_offset", wfl::variant(positioner_offset_));
		tmp.set_variable("positioner_length", wfl::variant(positioner_length_));
	}
	set_is_dirty(true);
}

void scrollbar_base::set_state(const state_t state)
{
	if(state != state_) {
		state_ = state;
		set_is_dirty(true);
	}
}

void scrollbar_base::recalculate()
{
	// We can be called before the size has been set up in that case we can't do
	// the proper recalculation so stop before we die with an assert.
	if(!get_length()) {
		return;
	}

	// Get the available size for the slider to move.
	const int available_length = get_length() - offset_before()
								 - offset_after();

	assert(available_length > 0);

	// All visible.
	if(item_count_ <= visible_items_) {
		positioner_offset_ = offset_before();
		positioner_length_ = available_length;
		recalculate_positioner();
		item_position_ = 0;
		update_canvas();
		return;
	}

	/**
	 * @todo In the MP lobby it can happen that a listbox has first zero items,
	 * then gets filled and since there are no visible items the second assert
	 * after this block will be triggered. Use this ugly hack to avoid that
	 * case. (This hack also added the gui/widgets/window.hpp include.)
	 */
	if(!visible_items_) {
		window* window = get_window();
		assert(window);
		window->invalidate_layout();
		ERR_GUI_G << LOG_HEADER
				  << " Can't recalculate size, force a window layout phase.\n";
		return;
	}

	assert(step_size_);
	assert(visible_items_);

	const unsigned steps = (item_count_ - visible_items_ - step_size_)
						   / step_size_;

	positioner_length_ = available_length * visible_items_ / item_count_;
	recalculate_positioner();

	// Make sure we can also show the last step, so add one more step.
	pixels_per_step_ = (available_length - positioner_length_)
					   / static_cast<float>(steps + 1);

	set_item_position(item_position_ * step_size_);
#if 0
	std::cerr << "Scrollbar recalculate overview:\n"
		<< "item_count_ " << item_count_
		<< " visible_items_ " << visible_items_
		<< " step_size_ " << step_size_
		<< " steps " << steps
		<< "\n"
		<< "minimum_positioner_length() " << minimum_positioner_length()
		<< " maximum_positioner_length() " << maximum_positioner_length()
		<< "\n"
		<< " positioner_length_ " << positioner_length_
		<< " positioner_offset_ " << positioner_offset_
		<< "\n"
		<< "available_length " << available_length
		<< " pixels_per_step_ " << pixels_per_step_
		<< ".\n\n";
#endif
}

void scrollbar_base::recalculate_positioner()
{
	const unsigned minimum = minimum_positioner_length();
	const unsigned maximum = maximum_positioner_length();

	if(minimum == maximum) {
		positioner_length_ = maximum;
	} else if(maximum != 0 && positioner_length_ > maximum) {
		positioner_length_ = maximum;
	} else if(positioner_length_ < minimum) {
		positioner_length_ = minimum;
	}
}

void scrollbar_base::move_positioner(const int distance)
{
	if(distance < 0 && -distance > static_cast<int>(positioner_offset_)) {
		positioner_offset_ = 0;
	} else {
		positioner_offset_ += distance;
	}

	const unsigned length = get_length() - offset_before() - offset_after()
							- positioner_length_;

	if(positioner_offset_ > length) {
		positioner_offset_ = length;
	}

	unsigned position
			= static_cast<unsigned>(positioner_offset_ / pixels_per_step_);

	// Note due to floating point rounding the position might be outside the
	// available positions so set it back.
	if(position > item_count_ - visible_items_) {
		position = item_count_ - visible_items_;
	}

	if(position != item_position_) {
		item_position_ = position;

		child_callback_positioner_moved();

		fire(event::NOTIFY_MODIFIED, *this, nullptr);

		// positioner_moved_notifier_.notify();
	}
#if 0
	std::cerr << "Scrollbar move overview:\n"
		<< "item_count_ " << item_count_
		<< " visible_items_ " << visible_items_
		<< " step_size_ " << step_size_
		<< "\n"
		<< "minimum_positioner_length() " << minimum_positioner_length()
		<< " maximum_positioner_length() " << maximum_positioner_length()
		<< "\n"
		<< " positioner_length_ " << positioner_length_
		<< " positioner_offset_ " << positioner_offset_
		<< "\n"
		<< " pixels_per_step_ " << pixels_per_step_
		<< " item_position_ " << item_position_
		<< ".\n\n";
#endif
	update_canvas();
}

void scrollbar_base::signal_handler_mouse_enter(const event::ui_event event,
											 bool& handled,
											 bool& halt)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	// Send the motion under our event id to make debugging easier.
	signal_handler_mouse_motion(event, handled, halt, get_mouse_position());
}

void scrollbar_base::signal_handler_mouse_motion(const event::ui_event event,
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
			if(in_orthogonal_range(mouse)) {
				const int distance = get_length_difference(mouse_, mouse);
				mouse_ = mouse;
				move_positioner(distance);
			}

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

void scrollbar_base::signal_handler_mouse_leave(const event::ui_event event,
											 bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	if(state_ == FOCUSED) {
		set_state(ENABLED);
	}
	handled = true;
}


void scrollbar_base::signal_handler_left_button_down(const event::ui_event event,
												  bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	point mouse = get_mouse_position();
	mouse.x -= get_x();
	mouse.y -= get_y();

	if(on_positioner(mouse)) {
		assert(get_window());
		mouse_ = mouse;
		get_window()->mouse_capture();
		set_state(PRESSED);
	}

	const int bar = on_bar(mouse);

	if(bar == -1) {
		scroll(HALF_JUMP_BACKWARDS);
		// positioner_moved_notifier_.notify();
	} else if(bar == 1) {
		scroll(HALF_JUMP_FORWARD);
		// positioner_moved_notifier_.notify();
	} else {
		assert(bar == 0);
	}

	handled = true;
}

void scrollbar_base::signal_handler_left_button_up(const event::ui_event event,
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

	handled = true;
}

} // namespace gui2
