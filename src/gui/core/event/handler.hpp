/*
	Copyright (C) 2009 - 2025
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

#include <cstdint>
#include <iosfwd>
#include <vector>

namespace gui2
{
class window;

namespace event
{
class dispatcher;

class manager
{
public:
	manager();
	~manager();
};

// clang-format off

/**
 * Event category masks.
 *
 * These begin at 2^8 to allow for 8 bits for the event identifiers themselves.
 * This means ui_event can have up to 256 unique members. Since each mask needs
 * its own place value, we can have 24 categories since ui_event's underlying
 * type is 32 bits:
 *
 *                   USABLE CATEGORY BITS    NULL
 *                |------------------------|--------|
 *     MASK        000000000000000000000000 00000000
 *
 *                   ENCODED CATEGORY        EVENT
 *                |------------------------|--------|
 *     UI_EVENT    000000000000000000000000 00000000
 */
enum class event_category : uint32_t {
	/**
	 * Callbacks without extra parameters.
 	 * @note Some mouse events like MOUSE_ENTER don't send the mouse coordinates
 	 * to the callback function so they are also in this category.
 	 */
	general           = 1u << 8,

	/**
	 * Callbacks with a coordinate as extra parameter.
	 */
	mouse             = 1u << 9,

	/**
	 * Callbacks with the keyboard values (these haven't been determined yet).
	 */
	keyboard          = 1u << 10,

	touch_motion      = 1u << 11,
	touch_gesture     = 1u << 12,

	/**
	 * Callbacks with a sender aka notification messages. Like general events
	 * it has no extra parameters, but this version is only sent to the target
	 * and does not use the pre and post queue.
 	 */
	notification      = 1u << 13,

	/**
	 * Callbacks with a sender aka notification messages.
	 * Unlike the notifications this message is send through the chain. The event
	 * is sent from a widget all the way up to the window, who is always the
	 * receiver of the message (unless somebody grabbed it before).
	 */
	message           = 1u << 14,

	raw_event         = 1u << 15,
	text_input        = 1u << 16,
};
// clang-format on

constexpr uint32_t encode_category(const uint32_t input, const event_category mask)
{
	return input | static_cast<uint32_t>(mask);
}

// clang-format off

/**
 * The event sent to the dispatcher.
 *
 * Events prefixed by SDL are (semi)-real SDL events. The handler does some
 * minor decoding like splitting the button down event to the proper event but
 * nothing more. Events without an SDL prefix are generated by another signal
 * eg the windows signal handler for SDL_MOUSE_MOTION can generate a
 * MOUSE_ENTER, MOUSE_MOTION and MOUSE_LEAVE event and send that to it's
 * children.
 *
 * @note When adding a new entry to the enum also add a unit test.
 */
enum ui_event : uint32_t {
	DRAW                           = encode_category(1 , event_category::general),
	CLOSE_WINDOW                   = encode_category(2 , event_category::general),
	MOUSE_ENTER                    = encode_category(3 , event_category::general),
	MOUSE_LEAVE                    = encode_category(4 , event_category::general),
	LEFT_BUTTON_DOWN               = encode_category(5 , event_category::general),
	LEFT_BUTTON_UP                 = encode_category(6 , event_category::general),
	LEFT_BUTTON_CLICK              = encode_category(7 , event_category::general),
	LEFT_BUTTON_DOUBLE_CLICK       = encode_category(8 , event_category::general),
	MIDDLE_BUTTON_DOWN             = encode_category(9 , event_category::general),
	MIDDLE_BUTTON_UP               = encode_category(10, event_category::general),
	MIDDLE_BUTTON_CLICK            = encode_category(11, event_category::general),
	MIDDLE_BUTTON_DOUBLE_CLICK     = encode_category(12, event_category::general),
	RIGHT_BUTTON_DOWN              = encode_category(13, event_category::general),
	RIGHT_BUTTON_UP                = encode_category(14, event_category::general),
	RIGHT_BUTTON_CLICK             = encode_category(15, event_category::general),
	RIGHT_BUTTON_DOUBLE_CLICK      = encode_category(16, event_category::general),

	SDL_VIDEO_RESIZE               = encode_category(17, event_category::mouse),
	SDL_MOUSE_MOTION               = encode_category(18, event_category::mouse),
	MOUSE_MOTION                   = encode_category(19, event_category::mouse),
	SDL_LEFT_BUTTON_DOWN           = encode_category(20, event_category::mouse),
	SDL_LEFT_BUTTON_UP             = encode_category(21, event_category::mouse),
	SDL_MIDDLE_BUTTON_DOWN         = encode_category(22, event_category::mouse),
	SDL_MIDDLE_BUTTON_UP           = encode_category(23, event_category::mouse),
	SDL_RIGHT_BUTTON_DOWN          = encode_category(24, event_category::mouse),
	SDL_RIGHT_BUTTON_UP            = encode_category(25, event_category::mouse),
	SDL_WHEEL_LEFT                 = encode_category(26, event_category::mouse),
	SDL_WHEEL_RIGHT                = encode_category(27, event_category::mouse),
	SDL_WHEEL_UP                   = encode_category(28, event_category::mouse),
	SDL_WHEEL_DOWN                 = encode_category(29, event_category::mouse),
	SHOW_TOOLTIP                   = encode_category(30, event_category::mouse),
	SHOW_HELPTIP                   = encode_category(31, event_category::mouse),
	SDL_TOUCH_UP                   = encode_category(32, event_category::mouse),
	SDL_TOUCH_DOWN                 = encode_category(33, event_category::mouse),

	SDL_KEY_DOWN                   = encode_category(34, event_category::keyboard),

	SDL_TEXT_INPUT                 = encode_category(35, event_category::text_input), /**< An SDL text input (commit) event. */
	SDL_TEXT_EDITING               = encode_category(36, event_category::text_input), /**< An SDL text editing (IME) event. */

	SDL_ACTIVATE                   = encode_category(37, event_category::notification),
	NOTIFY_REMOVAL                 = encode_category(38, event_category::notification),
	NOTIFY_MODIFIED                = encode_category(39, event_category::notification),
	NOTIFY_REMOVE_TOOLTIP          = encode_category(40, event_category::notification),
	RECEIVE_KEYBOARD_FOCUS         = encode_category(41, event_category::notification),
	LOSE_KEYBOARD_FOCUS            = encode_category(42, event_category::notification),

	REQUEST_PLACEMENT              = encode_category(43, event_category::message),
	MESSAGE_SHOW_TOOLTIP           = encode_category(44, event_category::message),
	MESSAGE_SHOW_HELPTIP           = encode_category(45, event_category::message),

	SDL_TOUCH_MOTION               = encode_category(46, event_category::touch_motion),
	SDL_TOUCH_MULTI_GESTURE        = encode_category(47, event_category::touch_gesture),

	SDL_RAW_EVENT                  = encode_category(48, event_category::raw_event)
};
// clang-format on

/**
 * Checks if a given event is in a given category.
 *
 * @note Even though all events currently have only one category bitflag set, this function
 * works correctly if they ever have multiple flags set, unlike @ref get_event_category.
 */
constexpr bool is_in_category(const ui_event event, const event_category mask)
{
	const uint32_t asu32 = static_cast<uint32_t>(mask);
	return (event & asu32) == asu32;
}

/**
 * Returns the category of a given event.
 *
 * @note Since each event has only *one* category flag set, it is safe to simply do an
 * equality check with this result, which would return the same as @ref is_in_category.
 */
constexpr event_category get_event_category(const ui_event event)
{
	// Zero-out the first 8 bits since those encode the ui_event value, which we don't want.
	return static_cast<event_category>((event >> 8u) << 8u);
}

/**
 * Connects a dispatcher to the event handler.
 *
 * @param dispatcher              The dispatcher to connect.
 */
void connect_dispatcher(dispatcher* dispatcher);

/**
 * Disconnects a dispatcher to the event handler.
 *
 * @param dispatcher              The dispatcher to disconnect.
 */
void disconnect_dispatcher(dispatcher* dispatcher);

/**
 * Gets all event dispatchers in the Z order.
 */
std::vector<dispatcher*>& get_all_dispatchers();

/**
 * Initializes the location of the mouse.
 *
 * After a layout of the window the mouse location needs to be updated to
 * test whether it entered or left a widget. Also after closing a window it's
 * needed to send a dummy mouse move.
 */
void init_mouse_location();

/**
 * Captures the mouse.
 *
 * A dispatcher can capture the mouse, when for example it's pressed on a
 * button, this means all mouse events after that are send to that widget.
 *
 * @param dispatcher              The dispatcher which should get the mouse
 *                                focus.
 */
void capture_mouse(dispatcher* dispatcher);

/**
 * Releases a captured mouse.
 *
 * @param dispatcher              The dispatcher which should release the mouse
 *                                capture.
 */
void release_mouse(dispatcher* dispatcher);

/**
 * Captures the keyboard.
 *
 * A dispatcher can capture the keyboard, when for example it's pressed on a
 * button, this means all keyboard events after that are send to that widget.
 *
 * @param dispatcher              The dispatcher which should get the keyboard
 *                                focus.
 */
void capture_keyboard(dispatcher* dispatcher);

std::ostream& operator<<(std::ostream& stream, const ui_event event);

} // namespace event

/**
 * Is a dialog open?
 *
 * @note added as backwards compatibility for gui::is_in_dialog.
 */
bool is_in_dialog();

} // namespace gui2
