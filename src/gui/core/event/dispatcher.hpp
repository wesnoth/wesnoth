/*
   Copyright (C) 2009 - 2018 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/core/event/handler.hpp"
#include "hotkey/hotkey_command.hpp"
#include "utils/functional.hpp"

#include <SDL_events.h>

#include <list>
#include <map>
#include <type_traits>

struct point;

namespace gui2
{
class widget;

namespace event
{
/**
 * Helper for catching use error of @ref dispatcher::connect_signal.
 *
 * This helper is needed as a user can't supply the wrong kind of callback
 * functions to dispatcher::connect_signal. If a wrong callback would be send
 * it will never get called.
 *
 * This version is for callbacks without extra parameters.
 * NOTE some mouse functions like MOUSE_ENTER don't send the mouse coordinates
 * to the callback function so they are also in this category.
 */
constexpr bool is_general_event(const ui_event event)
{
	return event == DRAW
		|| event == CLOSE_WINDOW
		|| event == MOUSE_ENTER
		|| event == MOUSE_LEAVE
		|| event == LEFT_BUTTON_DOWN
		|| event == LEFT_BUTTON_UP
		|| event == LEFT_BUTTON_CLICK
		|| event == LEFT_BUTTON_DOUBLE_CLICK
		|| event == MIDDLE_BUTTON_DOWN
		|| event == MIDDLE_BUTTON_UP
		|| event == MIDDLE_BUTTON_CLICK
		|| event == MIDDLE_BUTTON_DOUBLE_CLICK
		|| event == RIGHT_BUTTON_DOWN
		|| event == RIGHT_BUTTON_UP
		|| event == RIGHT_BUTTON_CLICK
		|| event == RIGHT_BUTTON_DOUBLE_CLICK;
}

/**
 * Helper for catching use error of @ref dispatcher::connect_signal.
 *
 * This version is for callbacks with a coordinate as extra parameter.
 */
constexpr bool is_mouse_event(const ui_event event)
{
	return event == SDL_VIDEO_RESIZE
		|| event == SDL_MOUSE_MOTION
		|| event == MOUSE_MOTION
		|| event == SDL_LEFT_BUTTON_DOWN
		|| event == SDL_LEFT_BUTTON_UP
		|| event == SDL_MIDDLE_BUTTON_DOWN
		|| event == SDL_MIDDLE_BUTTON_UP
		|| event == SDL_RIGHT_BUTTON_DOWN
		|| event == SDL_RIGHT_BUTTON_UP
		|| event == SHOW_TOOLTIP
		|| event == SHOW_HELPTIP
		|| event == SDL_WHEEL_UP
		|| event == SDL_WHEEL_DOWN
		|| event == SDL_WHEEL_LEFT
		|| event == SDL_WHEEL_RIGHT;
}

/**
 * Helper for catching use error of @ref dispatcher::connect_signal.
 *
 * This version is for callbacks with the keyboard values (these haven't been
 * determined yet).
 */
constexpr bool is_keyboard_event(const ui_event event)
{
	return event == SDL_KEY_DOWN;
}

/**
 * Helper for catching use error of @ref dispatcher::connect_signal.
 *
 * This version is for callbacks of touch events.
 */
constexpr bool is_touch_event(const ui_event event)
{
	return event == SDL_TOUCH_MOTION
		|| event == SDL_TOUCH_UP
		|| event == SDL_TOUCH_DOWN;
}

/**
 * Helper for catching use error of @ref dispatcher::connect_signal.
 *
 * This version is for callbacks with a sender aka notification messages. Like the
 * ones in set_event it has no extra parameters, but this version is only
 * send to the target and not using the pre and post queue.
 */
constexpr bool is_notification_event(const ui_event event)
{
	return event == NOTIFY_REMOVAL
		|| event == NOTIFY_MODIFIED
		|| event == RECEIVE_KEYBOARD_FOCUS
		|| event == LOSE_KEYBOARD_FOCUS
		|| event == NOTIFY_REMOVE_TOOLTIP
		|| event == SDL_ACTIVATE;
}

/**
 * Helper for catching use error of @ref dispatcher::connect_signal.
 *
 * This version is for callbacks with a sender aka notification messages.
 * Unlike the notifications this message is send through the chain. The event
 * is send from a widget all the way up to the window, who always is the
 * receiver of the message (unless somebody grabbed it before).
 */
constexpr bool is_message_event(const ui_event event)
{
	return event == MESSAGE_SHOW_TOOLTIP
		|| event == MESSAGE_SHOW_HELPTIP
		|| event == REQUEST_PLACEMENT;
}

/**
 * Helper for catching use error of @ref dispatcher::connect_signal.
 *
 * This version is for callbacks of raw events.
 */
constexpr bool is_raw_event_event(const ui_event event)
{
	return event == SDL_RAW_EVENT;
}

/**
 * Helper for catching use error of @ref dispatcher::connect_signal.
 *
 * This version is for callbacks of text input events.
 */
constexpr bool is_text_input_event(const ui_event event)
{
	return event == SDL_TEXT_INPUT || event == SDL_TEXT_EDITING;
}

struct message;

/**
 * Callback function signature alias template.
 *
 * All callbacks take these four arguments in addition to any arguments
 * specified by the parameter pack.
 *
 * Parameters:
 * 1. The widget handling this event.
 * 2. The event type.
 * 3. Reference to the flag controlling whether this event has been handled.
 * 4. Reference to the flag controlling whether to halt execution of this event.
 */
template<typename... T>
using dispatcher_callback_func = std::function<void(widget&, const ui_event, bool&, bool&, T...)>;

/**
 * Callback function signature.
 *
 * This is used for events matching @ref is_general_event.
 */
using signal_function = dispatcher_callback_func<>;

/**
 * Callback function signature.
 *
 * This is used for events matching @ref is_mouse_event.
 *
 * Extra parameters:
 * 5. The x,y coordinate of the mouse when this event is fired.
 */
using signal_mouse_function = dispatcher_callback_func<const point&>;

/**
 * Callback function signature.
 *
 * This is used for events matching @ref is_keyboard_event.
 *
 * Extra parameters:
 * 5. The keycode of the key that triggered this event.
 * 6. Any applicable active modifer key.
 * 7. Any applicable text associated with the key.
 */
using signal_keyboard_function =
	dispatcher_callback_func<const SDL_Keycode, const SDL_Keymod, const std::string&>;

/**
 * Callback function signature.
 *
 * This is used for events matching @ref is_touch_event.
 *
 * Extra parameters:
 * 5. Origin of the touch event, in x,y format.
 * 6. Number of pixels dragged, in x,y format.
 */
using signal_touch_function = dispatcher_callback_func<const point&, const point&>;

/**
 * Callback function signature.
 *
 * This is used for events matching @ref is_notification_event.
 *
 * Extra parameters:
 * 5. A dummy void* parameter which will always be nullptr, used to differentiate
 *    this function from signal_function.
 */
using signal_notification_function = dispatcher_callback_func<void*>;

/**
 * Callback function signature.
 *
 * This is used for events matching @ref is_message_event.
 *
 * Extra parameters:
 * 5. The applicable data this event requires.
 */
using signal_message_function = dispatcher_callback_func<const message&>;

/**
 * Raw event callback function signature.
 *
 * This is used for events matching @ref is_raw_event_event.
 *
 * Extra parameters:
 * 5. The raw SDL_Event.
 */
using signal_raw_event_function = dispatcher_callback_func<const SDL_Event&>;

/**
 * Callback function signature.
 *
 * This is used for events matching @ref is_text_input_event.
 *
 * Extra parameters:
 * 5. The text entered.
 * 6. The current input position.
 * 7. The current text selection length.
 */
using signal_text_input_function = dispatcher_callback_func<const std::string&, int32_t, int32_t>;

/** Hotkey function handler signature. */
using hotkey_function = std::function<void(widget& dispatcher, hotkey::HOTKEY_COMMAND id)>;

/**
 * Base class for event handling.
 *
 * A dispatcher has slots for events, when an event arrives it looks for the
 * functions that registered themselves for that event and calls their
 * callbacks.
 *
 * This class is a base class for all widgets[1], what a widget does on a
 * callback can differ greatly, an image might ignore all events a window can
 * track the mouse location and fire MOUSE_ENTER and MOUSE_LEAVE events to the
 * widgets involved.
 *
 * [1] Not really sure whether it will be a base class for a widget or
 * styled_widget yet.
 */
class dispatcher
{
	friend struct dispatcher_implementation;

public:
	dispatcher();
	virtual ~dispatcher();

	/**
	 * Connects the dispatcher to the event handler.
	 *
	 * When a dispatcher is connected to the event handler it will get the
	 * events directly from the event handler. This is wanted for top level
	 * items like windows but not for most other widgets.
	 *
	 * So a window can call connect to register itself, it will automatically
	 * disconnect upon destruction.
	 */
	void connect();

	/**
	 * Determines whether the location is inside an active widget.
	 *
	 * This is used to see whether a mouse event is inside the widget.
	 *
	 * @param coordinate             The coordinate to test whether inside the
	 *                               widget.
	 *
	 * @result                       True if inside an active widget, false
	 *                               otherwise.
	 */
	virtual bool is_at(const point& coordinate) const = 0;

	enum event_queue_type {
		pre = 1,
		child = 2,
		post = 4
	};

	bool has_event(const ui_event event, const event_queue_type event_type);

	/** Fires an event which has no extra parameters. */
	bool fire(const ui_event event, widget& target);

	/**
	 * Fires an event which takes a coordinate parameter.
	 *
	 * @param event                  The event to fire.
	 * @param target                 The widget that should receive the event.
	 * @param coordinate             The mouse position for the event.
	 */
	bool fire(const ui_event event,
			  widget& target,
			  const point& coordinate);

	/**
	 * Fires an event which takes keyboard parameters.
	 *
	 * @param event                  The event to fire.
	 * @param target                 The widget that should receive the event.
	 * @param key                    The SDL key code of the key pressed.
	 * @param modifier               The SDL key modifiers used.
	 * @param unicode                The unicode value for the key pressed.
	 */
	bool fire(const ui_event event,
			  widget& target,
			  const SDL_Keycode key,
			  const SDL_Keymod modifier,
			  const std::string& unicode);

	/**
	 * Fires an event which takes touch parameters.
	 *
	 * @param event                  The event to fire.
	 * @param target                 The widget that should receive the event.
	 * @param pos                    The location touched.
	 * @param distance               The distance moved.
	 */
	bool fire(const ui_event event,
			  widget& target,
			  const point& pos,
			  const point& distance);

	/**
	 * Fires an event which takes notification parameters.
	 *
	 * @note the void* parameter is a dummy needed for SFINAE.
	 *
	 * @param event                  The event to fire.
	 * @param target                 The widget that should receive the event.
	 */
	bool fire(const ui_event event,
			  widget& target,
			  void*);

	/**
	 * Fires an event which takes message parameters.
	 *
	 * @param event                  The event to fire.
	 * @param target                 The widget that should receive the event.
	 *                               Normally this is the window holding the
	 *                               widget.
	 * @param msg                    The extra information needed for a window
	 *                               (or another widget in the chain) to handle
	 *                               the message.
	 */
	bool fire(const ui_event event,
			  widget& target,
			  const message& msg);

	/**
	 * Fires an event that's a raw SDL event
	 * @param event 				The event to fire.
	 * @param target 				The widget that should receive the event.
	 *                              Normally this is the window holding the
	 *                              widget.
	 * @param sdlevent 				The raw SDL event
	 */
	bool fire(const ui_event event,
			  widget& target,
			  const SDL_Event& sdlevent);

	/**
	 * Fires an event which takes text input parameters
	 * @param event 				The event to fire.
	 * @param target 				The widget that should receive the event.
	 *                              Normally this is the window holding the
	 *                              widget.
	 * @param text                  The text involved in the event
	 * @param start                 The start point for IME editing
	 * @param len                   The selection length for IME editing
	 */
	bool fire(const ui_event event,
			  widget& target,
			  const std::string& text,
			  int32_t start,
			  int32_t len);

	/**
	 * The position where to add a new callback in the signal handler.
	 *
	 * The signal handler has three callback queues:
	 * * pre_child These callbacks are called before a container widget sends it
	 *   to the child items. Widgets without children should also use this
	 *   queue.
	 * * child The callbacks for the proper child widget(s) are called.
	 * * post_child The callbacks for the parent container to be called after
	 *   the child.
	 *
	 * For every queue it's possible to add a new event in the front or in the
	 * back.
	 *
	 * Whether all three queues are executed depend on the whether the
	 * callbacks modify the handled and halt flag.
	 * * When the halt flag is set execution of the current queue stops, when
	 *   doing so the handled flag must be set as well.
	 * * When the handled flag is set the events in that queue are executed and
	 *   no more queues afterwards.
	 *
	 * Here are some use case examples.
	 * A button that plays a sound and executes an optional user callback:
	 * * The buttons internal click handler is invoked and sets the handled
	 *   flag
	 * * The callback installed by the user is in the same queue and gets
	 *   executed afterwards.
	 *
	 * A toggle button may or may not be toggled:
	 * * The user inserts a callback, that validates whether the action is
	 *   allowed, if not allowed it sets the halt flag (and handled), else
	 *   leaves the flags untouched.
	 * * The normal buttons toggle function then might get invoked and if so
	 *   sets the handled flag.
	 * * Optionally there is another user callback invoked at this point.
	 */
	enum queue_position {
		front_pre_child,
		back_pre_child,
		front_child,
		back_child,
		front_post_child,
		back_post_child
	};

	/**
	 * Connect a signal for callback in set_event.
	 *
	 * The function uses some enable_if magic to avoid registering the wrong
	 * function, but the common way to use this function is:
	 * widget->connect_signal<EVENT_ID>(
	 * std::bind(&tmy_dialog::my_member, this));
	 * This allows simply adding a member of a dialog to be used as a callback
	 * for widget without a lot of magic. Note most widgets probably will get a
	 * callback like
	 * connect_signal_mouse_left_click(const signal_function& callback)
	 * which hides this function for the average use.
	 *
	 * @tparam E                     The event the callback needs to react to.
	 * @param signal                 The callback function.
	 * @param position               The position to place the callback.
	 */
	template<ui_event E>
	std::enable_if_t<is_general_event(E)>
	connect_signal(const signal_function& signal, const queue_position position = back_child)
	{
		signal_queue_.connect_signal(E, position, signal);
	}

	/**
	 * Disconnect a signal for callback in set_event.
	 *
	 * @tparam E                     The event the callback was used for.
	 * @param signal                 The callback function.
	 * @param position               The place where the function was added.
	 *                               Needed remove the event from the right
	 *                               place. (The function doesn't care whether
	 *                               was added in front or back.)
	 */
	template<ui_event E>
	std::enable_if_t<is_general_event(E)>
	disconnect_signal(const signal_function& signal, const queue_position position = back_child)
	{
		signal_queue_.disconnect_signal(E, position, signal);
	}

	/**
	 * Connect a signal for callback in set_event_mouse.
	 *
	 * @tparam E                     The event the callback needs to react to.
	 * @param signal                 The callback function.
	 * @param position               The position to place the callback.
	 */
	template<ui_event E>
	std::enable_if_t<is_mouse_event(E)>
	connect_signal(const signal_mouse_function& signal, const queue_position position = back_child)
	{
		signal_mouse_queue_.connect_signal(E, position, signal);
	}

	/**
	 * Disconnect a signal for callback in set_event_mouse.
	 *
	 * @tparam E                     The event the callback was used for.
	 * @param signal                 The callback function.
	 * @param position               The place where the function was added.
	 *                               Needed remove the event from the right
	 *                               place. (The function doesn't care whether
	 *                               was added in front or back.)
	 */
	template<ui_event E>
	std::enable_if_t<is_mouse_event(E)>
	disconnect_signal(const signal_mouse_function& signal, const queue_position position = back_child)
	{
		signal_mouse_queue_.disconnect_signal(E, position, signal);
	}

	/**
	 * Connect a signal for callback in set_event_keyboard.
	 *
	 * @tparam E                     The event the callback needs to react to.
	 * @param signal                 The callback function.
	 * @param position               The position to place the callback.
	 */
	template<ui_event E>
	std::enable_if_t<is_keyboard_event(E)>
	connect_signal(const signal_keyboard_function& signal, const queue_position position = back_child)
	{
		signal_keyboard_queue_.connect_signal(E, position, signal);
	}

	/**
	 * Disconnect a signal for callback in set_event_keyboard.
	 *
	 * @tparam E                     The event the callback was used for.
	 * @param signal                 The callback function.
	 * @param position               The place where the function was added.
	 *                               Needed remove the event from the right
	 *                               place. (The function doesn't care whether
	 *                               was added in front or back.)
	 */
	template<ui_event E>
	std::enable_if_t<is_keyboard_event(E)>
	disconnect_signal(const signal_keyboard_function& signal, const queue_position position = back_child)
	{
		signal_keyboard_queue_.disconnect_signal(E, position, signal);
	}

	/**
	 * Connect a signal for callback in set_event_touch.
	 *
	 * @tparam E                     The event the callback needs to react to.
	 * @param signal                 The callback function.
	 * @param position               The position to place the callback.
	 */
	template<ui_event E>
	std::enable_if_t<is_touch_event(E)>
	connect_signal(const signal_touch_function& signal, const queue_position position = back_child)
	{
		signal_touch_queue_.connect_signal(E, position, signal);
	}

	/**
	 * Disconnect a signal for callback in set_event_touch.
	 *
	 * @tparam E                     The event the callback was used for.
	 * @param signal                 The callback function.
	 * @param position               The place where the function was added.
	 *                               Needed remove the event from the right
	 *                               place. (The function doesn't care whether
	 *                               was added in front or back.)
	 */
	template<ui_event E>
	std::enable_if_t<is_touch_event(E)>
	disconnect_signal(const signal_touch_function& signal, const queue_position position = back_child)
	{
		signal_touch_queue_.disconnect_signal(E, position, signal);
	}

	/**
	 * Connect a signal for callback in set_event_notification.
	 *
	 * @tparam E                     The event the callback needs to react to.
	 * @param signal                 The callback function.
	 * @param position               The position to place the callback. Since
	 *                               the message is send to a widget directly
	 *                               the pre and post positions make no sense
	 *                               and shouldn't be used.
	 */
	template<ui_event E>
	std::enable_if_t<is_notification_event(E)>
	connect_signal(const signal_notification_function& signal, const queue_position position = back_child)
	{
		signal_notification_queue_.connect_signal(E, position, signal);
	}

	/**
	 * Disconnect a signal for callback in set_event_notification.
	 *
	 * @tparam E                     The event the callback was used for.
	 * @param signal                 The callback function.
	 * @param position               The place where the function was added.
	 *                               Needed remove the event from the right
	 *                               place. (The function doesn't care whether
	 *                               was added in front or back, but it needs
	 *                               to know the proper queue so it's save to
	 *                               add with front_child and remove with
	 *                               back_child. But it's not save to add with
	 *                               front_child and remove with
	 *                               front_pre_child)
	 */
	template<ui_event E>
	std::enable_if_t<is_notification_event(E)>
	disconnect_signal(const signal_notification_function& signal, const queue_position position = back_child)
	{
		signal_notification_queue_.disconnect_signal(E, position, signal);
	}

	/**
	 * Connect a signal for callback in set_event_message.
	 *
	 * @tparam E                     The event the callback needs to react to.
	 * @param signal                 The callback function.
	 * @param position               The position to place the callback. Since
	 *                               the message is send to a widget directly
	 *                               the pre and post positions make no sense
	 *                               and shouldn't be used.
	 */
	template<ui_event E>
	std::enable_if_t<is_message_event(E)>
	connect_signal(const signal_message_function& signal, const queue_position position = back_child)
	{
		signal_message_queue_.connect_signal(E, position, signal);
	}

	/**
	 * Disconnect a signal for callback in set_event_message.
	 *
	 * @tparam E                     The event the callback was used for.
	 * @param signal                 The callback function.
	 * @param position               The place where the function was added.
	 *                               Needed remove the event from the right
	 *                               place. (The function doesn't care whether
	 *                               was added in front or back, but it needs
	 *                               to know the proper queue so it's save to
	 *                               add with front_child and remove with
	 *                               back_child. But it's not save to add with
	 *                               front_child and remove with
	 *                               front_pre_child)
	 */
	template<ui_event E>
	std::enable_if_t<is_message_event(E)>
	disconnect_signal(const signal_message_function& signal, const queue_position position = back_child)
	{
		signal_message_queue_.disconnect_signal(E, position, signal);
	}

	/**
	 * Connect a signal for callback in set_raw_event.
	 *
	 * @tparam E                     The event the callback needs to react to.
	 * @param signal                 The callback function.
	 * @param position               The position to place the callback.
	 */
	template<ui_event E>
	std::enable_if_t<is_raw_event_event(E)>
	connect_signal(const signal_raw_event_function& signal, const queue_position position = back_child)
	{
		signal_raw_event_queue_.connect_signal(E, position, signal);
	}

	/**
	 * Disconnect a signal for callback in set_raw_event.
	 *
	 * @tparam E                     The event the callback was used for.
	 * @param signal                 The callback function.
	 * @param position               The place where the function was added.
	 *                               Needed remove the event from the right
	 *                               place. (The function doesn't care whether
	 *                               was added in front or back.)
	 */
	template<ui_event E>
	std::enable_if_t<is_raw_event_event(E)>
	disconnect_signal(const signal_raw_event_function& signal, const queue_position position = back_child)
	{
		signal_raw_event_queue_.disconnect_signal(E, position, signal);
	}

	/**
	 * Connect a signal for callback in set_text_input.
	 *
	 * @tparam E                     The event the callback needs to react to.
	 * @param signal                 The callback function.
	 * @param position               The position to place the callback.
	 */
	template<ui_event E>
	std::enable_if_t<is_text_input_event(E)>
	connect_signal(const signal_text_input_function& signal, const queue_position position = back_child)
	{
		signal_text_input_queue_.connect_signal(E, position, signal);
	}

	/**
	 * Disconnect a signal for callback in set_text_input.
	 *
	 * @tparam E                     The event the callback was used for.
	 * @param signal                 The callback function.
	 * @param position               The place where the function was added.
	 *                               Needed remove the event from the right
	 *                               place. (The function doesn't care whether
	 *                               was added in front or back.)
	 */
	template<ui_event E>
	std::enable_if_t<is_text_input_event(E)>
	disconnect_signal(const signal_text_input_function& signal, const queue_position position = back_child)
	{
		signal_text_input_queue_.disconnect_signal(E, position, signal);
	}

	/**
	 * The behavior of the mouse events.
	 *
	 * Normally for mouse events there's first checked whether a dispatcher has
	 * captured the mouse if so it gets the event.
	 * If not the dispatcher is searched from the back to the front in the
	 * layers and its behavior is checked.
	 * * none The event is never send to the layer and goes on the the next
	 *   layer. This is used for tooltips who might cover a button but a click
	 *   on the tooltips should still click the button.
	 * * all The event is always send to this layer and stops the search for a
	 *   next layer.
	 * * hit If the mouse is inside the dispatcher area the event is send and
	 *   no longer searched further. If not inside tests the last layer.
	 *
	 * If after these tests no dispatcher is found the event is ignored.
	 */
	enum mouse_behavior {
		all,
		hit,
		none
	};

	/** Captures the mouse. */
	void capture_mouse()
	{
		gui2::event::capture_mouse(this);
	}

	/** Releases the mouse capture. */
	void release_mouse()
	{
		gui2::event::release_mouse(this);
	}

	/***** ***** ***** setters/getters ***** ***** *****/

	void set_mouse_behavior(const mouse_behavior mouse_behavior)
	{
		mouse_behavior_ = mouse_behavior;
	}

	mouse_behavior get_mouse_behavior() const
	{
		return mouse_behavior_;
	}

	void set_want_keyboard_input(const bool want_keyboard_input)
	{
		want_keyboard_input_ = want_keyboard_input;
	}

	bool get_want_keyboard_input() const
	{
		return want_keyboard_input_;
	}

	/** Helper struct to generate the various signal types. */
	template<class T>
	struct signal_type
	{
		signal_type() : pre_child(), child(), post_child()
		{
		}

		std::list<T> pre_child;
		std::list<T> child;
		std::list<T> post_child;

		/**
		 * Checks whether the queue of a given type is empty.
		 *
		 * @param queue_type    The queue to check. This may be one or more types
		 *                      OR'd together (event_queue_type is bit-unique).
		 *
		 * @returns             True if ALL the matching queues are empty, or false
		 *                      if any of the matching queues is NOT empty.
		 */
		bool empty(const dispatcher::event_queue_type queue_type) const
		{
			if((queue_type & dispatcher::pre) && !pre_child.empty()) {
				return false;
			}

			if((queue_type & dispatcher::child) && !child.empty()) {
				return false;
			}

			if((queue_type & dispatcher::post) && !post_child.empty()) {
				return false;
			}

			return true;
		}
	};

	/** Helper struct to generate the various event queues. */
	template<class T>
	struct signal_queue
	{
		signal_queue() : queue()
		{
		}

		std::map<ui_event, signal_type<T>> queue;

		void connect_signal(const ui_event event,
							const queue_position position,
							const T& signal)
		{
			switch(position) {
				case front_pre_child:
					queue[event].pre_child.push_front(signal);
					break;
				case back_pre_child:
					queue[event].pre_child.push_back(signal);
					break;

				case front_child:
					queue[event].child.push_front(signal);
					break;
				case back_child:
					queue[event].child.push_back(signal);
					break;

				case front_post_child:
					queue[event].post_child.push_front(signal);
					break;
				case back_post_child:
					queue[event].post_child.push_back(signal);
					break;
			}
		}

		void disconnect_signal(const ui_event event,
							   const queue_position position,
							   const T& signal)
		{
			signal_type<T>& signal_queue = queue[event];

			/* The function doesn't differentiate between front and back position so fall
			 * down from front to back.
			 *
			 * NOTE: This used to only remove the first signal of matching target type.
			 *       That behavior could be restored in the future if needed.
			 * - vultraz, 2017-05-02
			 */
			switch(position) {
				case front_pre_child:
				case back_pre_child: {
					signal_queue.pre_child.remove_if(
						[&signal](T& element) { return signal.target_type() == element.target_type(); }
					);
				} break;

				case front_child:
				case back_child: {
					signal_queue.child.remove_if(
						[&signal](T& element) { return signal.target_type() == element.target_type(); }
					);
				} break;

				case front_post_child:
				case back_post_child: {
					signal_queue.post_child.remove_if(
						[&signal](T& element) { return signal.target_type() == element.target_type(); }
					);
				} break;
			}
		}
	};

	/**
	 * Registers a hotkey.
	 *
	 * @todo add a static function register_global_hotkey.
	 *
	 * Once that's done execute_hotkey will first try to execute a global
	 * hotkey and if that fails tries the hotkeys in this dispatcher.
	 *
	 * @param id                  The hotkey to register.
	 * @param function            The callback function to call.
	 */
	void register_hotkey(const hotkey::HOTKEY_COMMAND id,
						 const hotkey_function& function);

	/**
	 * Executes a hotkey.
	 *
	 * @param id                  The hotkey to execute.
	 *
	 * @returns                   true if the hotkey is handled, false
	 *                            otherwise.
	 */
	bool execute_hotkey(const hotkey::HOTKEY_COMMAND id);

private:
	/** The mouse behavior for the dispatcher. */
	mouse_behavior mouse_behavior_;

	/**
	 * Does the dispatcher want to receive keyboard input.
	 *
	 * @todo The entire mouse and keyboard handling can use a code review to
	 * seen whether it might be combined in one flag field. At the moment the
	 * keyboard doesn't look whether a dialog has the mouse focus before
	 * sending the event, so maybe we should add an active dispatcher to keep
	 * track of it. But since at the moment there are only non-modal windows
	 * and tooltips it's not a problem.
	 */
	bool want_keyboard_input_;

	/** Signal queue for callbacks in set_event. */
	signal_queue<signal_function> signal_queue_;

	/** Signal queue for callbacks in set_event_mouse. */
	signal_queue<signal_mouse_function> signal_mouse_queue_;

	/** Signal queue for callbacks in set_event_keyboard. */
	signal_queue<signal_keyboard_function> signal_keyboard_queue_;

	/** Signal queue for callbacks in set_event_touch. */
	signal_queue<signal_touch_function> signal_touch_queue_;

	/** Signal queue for callbacks in set_event_notification. */
	signal_queue<signal_notification_function> signal_notification_queue_;

	/** Signal queue for callbacks in set_event_message. */
	signal_queue<signal_message_function> signal_message_queue_;

	/** Signal queue for callbacks in set_raw_event. */
	signal_queue<signal_raw_event_function> signal_raw_event_queue_;

	/** Signal queue for callbacks in set_event_text_input. */
	signal_queue<signal_text_input_function> signal_text_input_queue_;

	/** Are we connected to the event handler. */
	bool connected_;

	/** The registered hotkeys for this dispatcher. */
	std::map<hotkey::HOTKEY_COMMAND, hotkey_function> hotkeys_;
};

/***** ***** ***** ***** ***** Common helpers  ***** ***** ***** ***** *****/

/*
 * These helpers can be used to easily add callbacks to a dispatcher (widget).
 * This is just a list of common ones all others can be used as well.
 */

/**
 * Connects the signal for 'snooping' on the keypress.
 *
 * This callback is called before the widget itself allowing you to either
 * snoop on the input or filter it.
 */
void connect_signal_pre_key_press(dispatcher& dispatcher, const signal_keyboard_function& signal);

/** Connects a signal handler for a left mouse button click. */
void connect_signal_mouse_left_click(dispatcher& dispatcher, const signal_function& signal);

/** Disconnects a signal handler for a left mouse button click. */
void disconnect_signal_mouse_left_click(dispatcher& dispatcher, const signal_function& signal);

/**
 * Connects a signal handler for a left mouse button double click.
 *
 * I'm not exactly sure why this works in this queue position with toggle
 * panels, but it does. Will revisit if it becomes an issue later (ie, if
 * this is used with other widgets and doesn't work).
 *
 * - vultraz, 2017-08-23
 */
void connect_signal_mouse_left_double_click(dispatcher& dispatcher, const signal_function& signal);

/** Connects a signal handler for getting a notification upon modification. */
void connect_signal_notify_modified(dispatcher& dispatcher, const signal_notification_function& signal);

/** Connects a signal handler for a callback when the widget is drawn. */
void connect_signal_on_draw(dispatcher& dispatcher, const signal_function& signal);

} // namespace event

} // namespace gui2
