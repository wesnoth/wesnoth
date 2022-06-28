/*
	Copyright (C) 2009 - 2022
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

#include "gui/core/event/distributor.hpp"

#include "events.hpp"
#include "gui/core/log.hpp"
#include "gui/core/timer.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box_base.hpp"
#include "gui/widgets/widget.hpp"
#include "gui/widgets/window.hpp"
#include "sdl/userevent.hpp"
#include "sdl/input.hpp" // get_mouse_button_mask

#include <array>
#include <functional>

namespace gui2::event
{
/**
 * Small helper to keep a resource (boolean) locked.
 *
 * Some of the event handling routines can't be called recursively, this due to
 * the fact that they are attached to the pre queue and when the forward an
 * event the pre queue event gets triggered recursively causing infinite
 * recursion.
 *
 * To prevent that those functions check the lock and exit when the lock is
 * held otherwise grab the lock here.
 */
class resource_locker
{
public:
	resource_locker(bool& locked) : locked_(locked)
	{
		assert(!locked_);
		locked_ = true;
	}

	~resource_locker()
	{
		assert(locked_);
		locked_ = false;
	}

private:
	bool& locked_;
};


/***** ***** ***** ***** mouse_motion ***** ***** ***** ***** *****/

#define LOG_HEADER "distributor mouse motion [" << owner_.id() << "]: "

mouse_motion::mouse_motion(widget& owner, const dispatcher::queue_position queue_position)
	: mouse_focus_(nullptr)
	, mouse_captured_(false)
	, owner_(owner)
	, hover_timer_(0)
	, hover_widget_(nullptr)
	, hover_position_(0, 0)
	, hover_shown_(true)
	, signal_handler_sdl_mouse_motion_entered_(false)
{
	owner.connect_signal<event::SDL_MOUSE_MOTION>(
		std::bind(&mouse_motion::signal_handler_sdl_mouse_motion,
			this,
			std::placeholders::_2,
			std::placeholders::_3,
			std::placeholders::_5),
		queue_position);

	owner.connect_signal<event::SDL_TOUCH_MOTION>(
		std::bind(&mouse_motion::signal_handler_sdl_touch_motion,
			this,
			std::placeholders::_2,
			std::placeholders::_3,
			std::placeholders::_5,
			std::placeholders::_6),
		queue_position);

	owner_.connect_signal<event::SDL_WHEEL_UP>(
		std::bind(&mouse_motion::signal_handler_sdl_wheel,
			this,
			std::placeholders::_2,
			std::placeholders::_3,
			std::placeholders::_5
		));

	owner_.connect_signal<event::SDL_WHEEL_DOWN>(
		std::bind(&mouse_motion::signal_handler_sdl_wheel,
			this,
			std::placeholders::_2,
			std::placeholders::_3,
			std::placeholders::_5
		));

	owner_.connect_signal<event::SDL_WHEEL_LEFT>(
		std::bind(&mouse_motion::signal_handler_sdl_wheel,
			this,
			std::placeholders::_2,
			std::placeholders::_3,
			std::placeholders::_5
		));

	owner_.connect_signal<event::SDL_WHEEL_RIGHT>(
		std::bind(&mouse_motion::signal_handler_sdl_wheel,
			this,
			std::placeholders::_2,
			std::placeholders::_3,
			std::placeholders::_5
		));

	owner.connect_signal<event::SHOW_HELPTIP>(
		std::bind(&mouse_motion::signal_handler_show_helptip,
			this,
			std::placeholders::_2,
			std::placeholders::_3,
			std::placeholders::_5),
		queue_position);
}

mouse_motion::~mouse_motion()
{
	stop_hover_timer();
}

void mouse_motion::capture_mouse(const bool capture)
{
	assert(mouse_focus_);
	mouse_captured_ = capture;
}

void mouse_motion::signal_handler_sdl_mouse_motion(const event::ui_event event, bool& handled, const point& coordinate)
{
	if(signal_handler_sdl_mouse_motion_entered_) {
		return;
	}
	resource_locker lock{signal_handler_sdl_mouse_motion_entered_};

	DBG_GUI_E << LOG_HEADER << event << ".\n";

	if(mouse_captured_) {
		assert(mouse_focus_);
		if(!owner_.fire(event, *mouse_focus_, coordinate)) {
			mouse_hover(mouse_focus_, coordinate);
		}
	} else {
		widget* mouse_over = owner_.find_at(coordinate, true);
		while(mouse_over && !mouse_over->can_mouse_focus() && mouse_over->parent()) {
			mouse_over = mouse_over->parent();
		}

		if(mouse_over) {
			DBG_GUI_E << LOG_HEADER << "Firing: " << event << ".\n";
			if(owner_.fire(event, *mouse_over, coordinate)) {
				return;
			}
		}

		if(!mouse_focus_ && mouse_over) {
			mouse_enter(mouse_over);
		} else if(mouse_focus_ && !mouse_over) {
			mouse_leave();
		} else if(mouse_focus_ && mouse_focus_ == mouse_over) {
			mouse_hover(mouse_over, coordinate);
		} else if(mouse_focus_ && mouse_over) {
			// moved from one widget to the next
			mouse_leave();
			mouse_enter(mouse_over);
		} else {
			assert(!mouse_focus_ && !mouse_over);
		}
	}
	handled = true;
}

void mouse_motion::signal_handler_sdl_touch_motion(
	const event::ui_event event, bool& handled, const point& coordinate, const point& distance)
{
	DBG_GUI_E << LOG_HEADER << event << ".\n";

	if(mouse_captured_) {
		assert(mouse_focus_);
		owner_.fire(event, *mouse_focus_, coordinate, distance);
	} else {
		if(widget* mouse_over = owner_.find_at(coordinate, true)) {
			owner_.fire(event, *mouse_over, coordinate, distance);
		}
	}
	handled = true;
}

void mouse_motion::signal_handler_sdl_wheel(const event::ui_event event, bool& handled, const point& coordinate)
{
	DBG_GUI_E << LOG_HEADER << event << ".\n";

	if(mouse_captured_) {
		assert(mouse_focus_);
		owner_.fire(event, *mouse_focus_, coordinate);
	} else {
		if(widget* mouse_over = owner_.find_at(coordinate, true)) {
			owner_.fire(event, *mouse_over, coordinate);
		}
	}
	handled = true;
}

void mouse_motion::signal_handler_show_helptip(const event::ui_event event, bool& handled, const point& coordinate)
{
	DBG_GUI_E << LOG_HEADER << event << ".\n";

	if(mouse_captured_) {
		assert(mouse_focus_);
		if(owner_.fire(event, *mouse_focus_, coordinate)) {
			stop_hover_timer();
		}
	} else {
		if(widget* mouse_over = owner_.find_at(coordinate, true)) {
			DBG_GUI_E << LOG_HEADER << "Firing: " << event << ".\n";
			if(owner_.fire(event, *mouse_over, coordinate)) {
				stop_hover_timer();
			}
		}
	}

	handled = true;
}

void mouse_motion::mouse_enter(widget* mouse_over)
{
	DBG_GUI_E << LOG_HEADER << "Firing: " << event::MOUSE_ENTER << ".\n";

	assert(mouse_over);

	mouse_focus_ = mouse_over;
	owner_.fire(event::MOUSE_ENTER, *mouse_over);

	hover_shown_ = false;
	start_hover_timer(mouse_over, get_mouse_position());
}

void mouse_motion::mouse_hover(widget* mouse_over, const point& coordinate)
{
	DBG_GUI_E << LOG_HEADER << "Firing: " << event::MOUSE_MOTION << ".\n";

	assert(mouse_over);

	owner_.fire(event::MOUSE_MOTION, *mouse_over, coordinate);

	if(hover_timer_) {
		if((std::abs(hover_position_.x - coordinate.x) > 5) || (std::abs(hover_position_.y - coordinate.y) > 5)) {
			stop_hover_timer();
			start_hover_timer(mouse_over, coordinate);
		}
	}
}

void mouse_motion::show_tooltip()
{
	DBG_GUI_E << LOG_HEADER << "Firing: " << event::SHOW_TOOLTIP << ".\n";

	if(!hover_widget_) {
		// See mouse_motion::stop_hover_timer.
		ERR_GUI_E << LOG_HEADER << event::SHOW_TOOLTIP << " bailing out, no hover widget.\n";
		return;
	}

	/*
	 * Ignore the result of the event, always mark the tooltip as shown. If
	 * there was no handler, there is no reason to assume there will be one
	 * next time.
	 */
	owner_.fire(SHOW_TOOLTIP, *hover_widget_, hover_position_);

	hover_shown_ = true;

	hover_timer_ = 0;
	hover_widget_ = nullptr;
	hover_position_ = point();
}

void mouse_motion::mouse_leave()
{
	DBG_GUI_E << LOG_HEADER << "Firing: " << event::MOUSE_LEAVE << ".\n";

	styled_widget* control = dynamic_cast<styled_widget*>(mouse_focus_);
	if(!control || control->get_active()) {
		owner_.fire(event::MOUSE_LEAVE, *mouse_focus_);
	}

	owner_.fire(NOTIFY_REMOVE_TOOLTIP, *mouse_focus_, nullptr);

	mouse_focus_ = nullptr;

	stop_hover_timer();
}

void mouse_motion::start_hover_timer(widget* widget, const point& coordinate)
{
	assert(widget);

#ifdef __IPHONEOS__
	// Guessing a crash location in a nasty stack in gui2::execute_timer.
	// Either this or a long-touch menu.
	// Remove this when the crash in gui2::execute_timer() and gui2::timer_callback() is gone and try again.
	return;
#endif

	stop_hover_timer();

	if(hover_shown_ || !widget->wants_mouse_hover()) {
		return;
	}

	DBG_GUI_E << LOG_HEADER << "Start hover timer for widget '" << widget->id()
			  << "' at address " << widget << ".\n";

	hover_timer_ = add_timer(50, std::bind(&mouse_motion::show_tooltip, this));

	if(hover_timer_) {
		hover_widget_ = widget;
		hover_position_ = coordinate;
	} else {
		ERR_GUI_E << LOG_HEADER << "Failed to add hover timer." << std::endl;
	}
}

void mouse_motion::stop_hover_timer()
{
	if(hover_timer_) {
		assert(hover_widget_);
		DBG_GUI_E << LOG_HEADER << "Stop hover timer for widget '"
				  << hover_widget_->id() << "' at address " << hover_widget_
				  << ".\n";

		if(!remove_timer(hover_timer_)) {
			ERR_GUI_E << LOG_HEADER << "Failed to remove hover timer." << std::endl;
		}

		hover_timer_ = 0;
		hover_widget_ = nullptr;
		hover_position_ = point();
	}
}

/***** ***** ***** ***** mouse_button ***** ***** ***** ***** *****/

#undef LOG_HEADER
#define LOG_HEADER "distributor mouse button " << I << " [" << owner_.id() << "]: "

namespace
{
struct data_pod
{
	const ui_event sdl_button_down_event;
	const ui_event sdl_button_up_event;
	const ui_event button_down_event;
	const ui_event button_up_event;
	const ui_event button_click_event;
	const ui_event button_double_click_event;
};

constexpr std::array mouse_data{
	data_pod{
		SDL_LEFT_BUTTON_DOWN,
		SDL_LEFT_BUTTON_UP,
		LEFT_BUTTON_DOWN,
		LEFT_BUTTON_UP,
		LEFT_BUTTON_CLICK,
		LEFT_BUTTON_DOUBLE_CLICK,
	},
	data_pod{
		SDL_MIDDLE_BUTTON_DOWN,
		SDL_MIDDLE_BUTTON_UP,
		MIDDLE_BUTTON_DOWN,
		MIDDLE_BUTTON_UP,
		MIDDLE_BUTTON_CLICK,
		MIDDLE_BUTTON_DOUBLE_CLICK,
	},
	data_pod{
		SDL_RIGHT_BUTTON_DOWN,
		SDL_RIGHT_BUTTON_UP,
		RIGHT_BUTTON_DOWN,
		RIGHT_BUTTON_UP,
		RIGHT_BUTTON_CLICK,
		RIGHT_BUTTON_DOUBLE_CLICK,
	},
};

} // namespace

template<std::size_t I>
mouse_button<I>::mouse_button(widget& owner, const dispatcher::queue_position queue_position)
	: mouse_motion(owner, queue_position)
	, last_click_stamp_(0)
	, last_clicked_widget_(nullptr)
	, focus_(nullptr)
	, is_down_(false)
	, signal_handler_sdl_button_down_entered_(false)
	, signal_handler_sdl_button_up_entered_(false)
{
	static_assert(I < mouse_data.size(), "Out-of-bounds mouse_button template index");

	owner_.connect_signal<mouse_data[I].sdl_button_down_event>(
		std::bind(&mouse_button::signal_handler_sdl_button_down,
			this,
			std::placeholders::_2,
			std::placeholders::_3,
			std::placeholders::_5),
		queue_position);

	owner_.connect_signal<mouse_data[I].sdl_button_up_event>(
		std::bind(&mouse_button::signal_handler_sdl_button_up,
			this,
			std::placeholders::_2,
			std::placeholders::_3,
			std::placeholders::_5),
		queue_position);
}

template<std::size_t I>
void mouse_button<I>::initialize_state(int32_t button_state)
{
	last_click_stamp_ = 0;
	last_clicked_widget_ = nullptr;
	focus_ = 0;
	// SDL_BUTTON_LEFT, SDL_BUTTON_MIDDLE, and SDL_BUTTON_RIGHT correspond to 1,2,3
	is_down_ = button_state & SDL_BUTTON(I + 1);
}

template<std::size_t I>
void mouse_button<I>::signal_handler_sdl_button_down(
	const event::ui_event event, bool& handled, const point& coordinate)
{
	if(signal_handler_sdl_button_down_entered_) {
		return;
	}
	resource_locker lock{signal_handler_sdl_button_down_entered_};

	DBG_GUI_E << LOG_HEADER << event << ".\n";

	if(is_down_) {
#ifdef GUI2_SHOW_UNHANDLED_EVENT_WARNINGS
		WRN_GUI_E << LOG_HEADER << event << ". The mouse button is already down, we missed an event.\n";
#endif
		return;
	}
	is_down_ = true;

	if(mouse_captured_) {
		assert(mouse_focus_);
		focus_ = mouse_focus_;
		DBG_GUI_E << LOG_HEADER << "Firing: " << mouse_data[I].sdl_button_down_event << ".\n";
		if(!owner_.fire(mouse_data[I].sdl_button_down_event, *focus_, coordinate)) {
			DBG_GUI_E << LOG_HEADER << "Firing: " << mouse_data[I].button_down_event << ".\n";
			owner_.fire(mouse_data[I].button_down_event, *mouse_focus_);
		}
	} else {
		widget* mouse_over = owner_.find_at(coordinate, true);
		if(!mouse_over) {
			return;
		}

		if(mouse_over != mouse_focus_) {
#ifdef GUI2_SHOW_UNHANDLED_EVENT_WARNINGS
			WRN_GUI_E << LOG_HEADER << ". Mouse down on non focused widget "
					  << "and mouse not captured, we missed events.\n";
#endif
			mouse_focus_ = mouse_over;
		}

		focus_ = mouse_over;
		DBG_GUI_E << LOG_HEADER << "Firing: " << mouse_data[I].sdl_button_down_event << ".\n";
		if(!owner_.fire(mouse_data[I].sdl_button_down_event, *focus_, coordinate)) {
			DBG_GUI_E << LOG_HEADER << "Firing: " << mouse_data[I].button_down_event << ".\n";
			owner_.fire(mouse_data[I].button_down_event, *focus_);
		}
	}
	handled = true;
}

template<std::size_t I>
void mouse_button<I>::signal_handler_sdl_button_up(
	const event::ui_event event, bool& handled, const point& coordinate)
{
	if(signal_handler_sdl_button_up_entered_) {
		return;
	}
	resource_locker lock{signal_handler_sdl_button_up_entered_};

	DBG_GUI_E << LOG_HEADER << event << ".\n";

	if(!is_down_) {
#ifdef GUI2_SHOW_UNHANDLED_EVENT_WARNINGS
		WRN_GUI_E << LOG_HEADER << event << ". The mouse button is already up, we missed an event.\n";
#endif
		return;
	}
	is_down_ = false;

	if(focus_) {
		DBG_GUI_E << LOG_HEADER << "Firing: " << mouse_data[I].sdl_button_up_event << ".\n";
		if(!owner_.fire(mouse_data[I].sdl_button_up_event, *focus_, coordinate)) {
			DBG_GUI_E << LOG_HEADER << "Firing: " << mouse_data[I].button_up_event << ".\n";
			owner_.fire(mouse_data[I].button_up_event, *focus_);
		}
	}

	// FIXME: The block below is strange diamond inheritance - it's code that could be in
	// mouse_motion which applies to all three buttons, but it will be run in one of the
	// three mouse_button<T> subclasses, and then the other two mouse_button<T> subclasses
	// will reach here with mouse_captured_ == false.
	widget* mouse_over = owner_.find_at(coordinate, true);
	if(mouse_captured_) {
		const unsigned mask = SDL_BUTTON_LMASK | SDL_BUTTON_MMASK | SDL_BUTTON_RMASK;

		if((sdl::get_mouse_button_mask() & mask) == 0) {
			mouse_captured_ = false;
		}

		if(mouse_focus_ == mouse_over) {
			mouse_button_click(mouse_focus_);
		} else if(!mouse_captured_) {
			mouse_leave();

			if(mouse_over) {
				mouse_enter(mouse_over);
			}
		}
	} else if(focus_ && focus_ == mouse_over) {
		mouse_button_click(focus_);
	}

	focus_ = nullptr;
	handled = true;
}

template<std::size_t I>
void mouse_button<I>::mouse_button_click(widget* widget)
{
	uint32_t stamp = SDL_GetTicks();
	if(last_click_stamp_ + settings::double_click_time >= stamp && last_clicked_widget_ == widget) {
		DBG_GUI_E << LOG_HEADER << "Firing: " << mouse_data[I].button_double_click_event << ".\n";

		owner_.fire(mouse_data[I].button_double_click_event, *widget);
		last_click_stamp_ = 0;
		last_clicked_widget_ = nullptr;

	} else {
		DBG_GUI_E << LOG_HEADER << "Firing: " << mouse_data[I].button_click_event << ".\n";
		owner_.fire(mouse_data[I].button_click_event, *widget);
		last_click_stamp_ = stamp;
		last_clicked_widget_ = widget;
	}
}

/***** ***** ***** ***** distributor ***** ***** ***** ***** *****/

#undef LOG_HEADER
#define LOG_HEADER "distributor mouse motion [" << owner_.id() << "]: "

/**
 * @todo Test whether the state is properly tracked when an input blocker is
 * used.
 */
distributor::distributor(widget& owner,const dispatcher::queue_position queue_position)
	: mouse_motion(owner, queue_position)
	, mouse_button_left(owner, queue_position)
	, mouse_button_middle(owner, queue_position)
	, mouse_button_right(owner, queue_position)
	, keyboard_focus_(nullptr)
	, keyboard_focus_chain_()
{
	if(SDL_WasInit(SDL_INIT_TIMER) == 0) {
		if(SDL_InitSubSystem(SDL_INIT_TIMER) == -1) {
			assert(false);
		}
	}

	owner_.connect_signal<event::SDL_KEY_DOWN>(
		std::bind(&distributor::signal_handler_sdl_key_down,
			this,
			std::placeholders::_5,
			std::placeholders::_6,
			std::placeholders::_7
		));

	owner_.connect_signal<event::SDL_TEXT_INPUT>(
		std::bind(&distributor::signal_handler_sdl_text_input,
			this,
			std::placeholders::_5,
			std::placeholders::_6,
			std::placeholders::_7
		));

	owner_.connect_signal<event::SDL_TEXT_EDITING>(
		std::bind(&distributor::signal_handler_sdl_text_editing,
			this,
			std::placeholders::_5,
			std::placeholders::_6,
			std::placeholders::_7
		));

	owner_.connect_signal<event::NOTIFY_REMOVAL>(
		std::bind(&distributor::signal_handler_notify_removal,
			this,
			std::placeholders::_1,
			std::placeholders::_2
		));

	initialize_state();
}

distributor::~distributor()
{
	owner_.disconnect_signal<event::SDL_KEY_DOWN>(
		std::bind(&distributor::signal_handler_sdl_key_down,
			this,
			std::placeholders::_5,
			std::placeholders::_6,
			std::placeholders::_7
		));

	owner_.disconnect_signal<event::SDL_TEXT_INPUT>(
		std::bind(&distributor::signal_handler_sdl_text_input,
			this,
			std::placeholders::_5,
			std::placeholders::_6,
			std::placeholders::_7
		));

	owner_.disconnect_signal<event::SDL_TEXT_EDITING>(
		std::bind(&distributor::signal_handler_sdl_text_editing,
			this,
			std::placeholders::_5,
			std::placeholders::_6,
			std::placeholders::_7
		));

	owner_.disconnect_signal<event::NOTIFY_REMOVAL>(
		std::bind(&distributor::signal_handler_notify_removal,
			this,
			std::placeholders::_1,
			std::placeholders::_2
		));
}

void distributor::initialize_state()
{
	const uint32_t button_state = sdl::get_mouse_button_mask();

	mouse_button_left::initialize_state(button_state);
	mouse_button_middle::initialize_state(button_state);
	mouse_button_right::initialize_state(button_state);

	init_mouse_location();
}

widget* distributor::keyboard_focus() const
{
	return keyboard_focus_;
}

void distributor::keyboard_capture(widget* widget)
{
	if(keyboard_focus_) {
		DBG_GUI_E << LOG_HEADER << "Firing: " << event::LOSE_KEYBOARD_FOCUS << ".\n";
		owner_.fire(event::LOSE_KEYBOARD_FOCUS, *keyboard_focus_, nullptr);
	}

	keyboard_focus_ = widget;

	if(keyboard_focus_) {
		DBG_GUI_E << LOG_HEADER << "Firing: " << event::RECEIVE_KEYBOARD_FOCUS << ".\n";
		owner_.fire(event::RECEIVE_KEYBOARD_FOCUS, *keyboard_focus_, nullptr);
	}
}

void distributor::keyboard_add_to_chain(widget* widget)
{
	assert(widget);
	assert(std::find(keyboard_focus_chain_.begin(), keyboard_focus_chain_.end(), widget) == keyboard_focus_chain_.end());
	keyboard_focus_chain_.push_back(widget);
}

void distributor::keyboard_remove_from_chain(widget* w)
{
	assert(w);
	auto itor = std::find(keyboard_focus_chain_.begin(), keyboard_focus_chain_.end(), w);

	if(itor != keyboard_focus_chain_.end()) {
		keyboard_focus_chain_.erase(itor);
	}
}

template<typename Fcn, typename P1, typename P2, typename P3>
void distributor::signal_handler_keyboard_internal(event::ui_event evt, P1&& p1, P2&& p2, P3&& p3)
{
	/** @todo Test whether recursion protection is needed. */

	DBG_GUI_E << LOG_HEADER << evt << ".\n";

	if(keyboard_focus_) {
		// Attempt to cast to styled_widget, to avoid sending events if the
		// widget is disabled. If the cast fails, we assume the widget
		// is enabled and ready to receive events.
		styled_widget* control = dynamic_cast<styled_widget*>(keyboard_focus_);
		if(!control || control->get_active()) {
			DBG_GUI_E << LOG_HEADER << "Firing: " << evt << ".\n";
			if(owner_.fire(evt, *keyboard_focus_, p1, p2, p3)) {
				return;
			}
		}

		if(text_box_base* tb = dynamic_cast<text_box_base*>(keyboard_focus_)) {
			if(tb->is_composing()) {
				return; // Skip the keyboard chain if composition is in progress.
			}
		}
	}

	for(auto ritor = keyboard_focus_chain_.rbegin(); ritor != keyboard_focus_chain_.rend(); ++ritor) {
		if(*ritor == keyboard_focus_) {
			continue;
		}

		if(*ritor == &owner_) {
			/**
			 * @todo Make sure we're not in the event chain.
			 *
			 * No idea why we're here, but needs to be fixed, otherwise we keep
			 * calling this function recursively upon unhandled events...
			 *
			 * Probably added to make sure the window can grab the events and
			 * handle + block them when needed, this is no longer needed with
			 * the chain.
			 */
			continue;
		}

		// Attempt to cast to styled_widget, to avoid sending events if the
		// widget is disabled. If the cast fails, we assume the widget
		// is enabled and ready to receive events.
		styled_widget* control = dynamic_cast<styled_widget*>(keyboard_focus_);
		if(control != nullptr && !control->get_active()) {
			continue;
		}

		DBG_GUI_E << LOG_HEADER << "Firing: " << evt << ".\n";
		if(owner_.fire(evt, **ritor, p1, p2, p3)) {
			return;
		}
	}
}

void distributor::signal_handler_sdl_key_down(const SDL_Keycode key, const SDL_Keymod modifier, const std::string& unicode)
{
	signal_handler_keyboard_internal<signal_keyboard>(event::SDL_KEY_DOWN, key, modifier, unicode);
}

void distributor::signal_handler_sdl_text_input(const std::string& unicode, int32_t start, int32_t end)
{
	signal_handler_keyboard_internal<signal_text_input>(event::SDL_TEXT_INPUT, unicode, start, end);
}

void distributor::signal_handler_sdl_text_editing(const std::string& unicode, int32_t start, int32_t end)
{
	signal_handler_keyboard_internal<signal_text_input>(event::SDL_TEXT_EDITING, unicode, start, end);
}

void distributor::signal_handler_notify_removal(dispatcher& w, const ui_event event)
{
	DBG_GUI_E << LOG_HEADER << event << ".\n";

	/**
	 * @todo Evaluate whether moving the cleanup parts in the subclasses.
	 *
	 * It might be cleaner to do it that way, but creates extra small
	 * functions...
	 */

	if(hover_widget_ == &w) {
		stop_hover_timer();
	}

	if(mouse_button_left::last_clicked_widget_ == &w) {
		mouse_button_left::last_clicked_widget_ = nullptr;
	}
	if(mouse_button_left::focus_ == &w) {
		mouse_button_left::focus_ = nullptr;
	}

	if(mouse_button_middle::last_clicked_widget_ == &w) {
		mouse_button_middle::last_clicked_widget_ = nullptr;
	}
	if(mouse_button_middle::focus_ == &w) {
		mouse_button_middle::focus_ = nullptr;
	}

	if(mouse_button_right::last_clicked_widget_ == &w) {
		mouse_button_right::last_clicked_widget_ = nullptr;
	}
	if(mouse_button_right::focus_ == &w) {
		mouse_button_right::focus_ = nullptr;
	}

	if(mouse_focus_ == &w) {
		mouse_focus_ = nullptr;
	}

	if(keyboard_focus_ == &w) {
		keyboard_focus_ = nullptr;
	}

	const auto itor = std::find(keyboard_focus_chain_.begin(), keyboard_focus_chain_.end(), &w);
	if(itor != keyboard_focus_chain_.end()) {
		keyboard_focus_chain_.erase(itor);
	}
}

} // namespace gui2::event
