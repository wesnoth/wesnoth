/*
   Copyright (C) 2009 - 2015 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_AUXILIARY_EVENT_DISPATCHER_HPP_INCLUDED
#define GUI_WIDGETS_AUXILIARY_EVENT_DISPATCHER_HPP_INCLUDED

#include "gui/auxiliary/event/handler.hpp"
#include "hotkey/hotkey_command.hpp"
#include "sdl/compat.hpp"
#include "serialization/unicode_types.hpp"

#include "SDL_events.h"

#include "utils/boost_function_guarded.hpp"
#include <boost/mpl/int.hpp>
#include <boost/utility/enable_if.hpp>

#include <map>

namespace gui2
{

struct tpoint;
class twidget;

namespace event
{

struct tmessage;

/**
 * Callback function signature.
 *
 * There are several kinds of callback signature, this only has the parameters
 * shared by all callbacks.
 *
 * This function is used for the callbacks in tset_event.
 */
typedef boost::function<void(
		tdispatcher& dispatcher, const tevent event, bool& handled, bool& halt)>
tsignal_function;

/**
 * Callback function signature.
 *
 * This function is used for the callbacks in tset_event_mouse.
 */
typedef boost::function<void(tdispatcher& dispatcher,
							 const tevent event,
							 bool& handled,
							 bool& halt,
							 const tpoint& coordinate)> tsignal_mouse_function;

/**
 * Callback function signature.
 *
 * This function is used for the callbacks in tset_event_keyboard.
 */
typedef boost::function<void(tdispatcher& dispatcher,
							 const tevent event,
							 bool& handled,
							 bool& halt,
							 const SDLKey key,
							 const SDLMod modifier,
							 const utf8::string& unicode)>
tsignal_keyboard_function;

/**
 * Callback function signature.
 *
 * This function is used for the callbacks in tset_event_notification.
 * Added the dummy void* parameter which will be NULL to get a different
 * signature as tsignal_function's callback.
 */
typedef boost::function<void(tdispatcher& dispatcher,
							 const tevent event,
							 bool& handled,
							 bool& halt,
							 void*)> tsignal_notification_function;

/**
 * Callback function signature.
 *
 * This function is used for the callbacks in tset_message_notification.
 */
typedef boost::function<void(tdispatcher& dispatcher,
							 const tevent event,
							 bool& handled,
							 bool& halt,
							 tmessage& message)> tsignal_message_function;

/** Hotkey function handler signature. */
typedef boost::function<bool(tdispatcher& dispatcher,
							 hotkey::HOTKEY_COMMAND id)> thotkey_function;

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
 * [1] Not really sure whether it will be a base clase for a twidget or
 * tcontrol yet.
 */
class tdispatcher
{
	friend struct tdispatcher_implementation;

public:
	tdispatcher();
	virtual ~tdispatcher();

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
	virtual bool is_at(const tpoint& coordinate) const = 0;

	enum tevent_type {
		pre = 1,
		child = 2,
		post = 4
	};

	bool has_event(const tevent event, const tevent_type event_type);

	/** Fires an event which has no extra parameters. */
	bool fire(const tevent event, twidget& target);

	/**
	 * Fires an event which takes a coordinate parameter.
	 *
	 * @param event                  The event to fire.
	 * @param target                 The widget that should receive the event.
	 * @param coordinate             The mouse position for the event.
	 */
	bool fire(const tevent event, twidget& target, const tpoint& coordinate);

	/**
	 * Fires an event which takes keyboard parameters.
	 *
	 * @param event                  The event to fire.
	 * @param target                 The widget that should receive the event.
	 * @param key                    The SDL key code of the key pressed.
	 * @param modifier               The SDL key modifiers used.
	 * @param unicode                The unicode value for the key pressed.
	 */
	bool fire(const tevent event,
			  twidget& target,
			  const SDLKey key,
			  const SDLMod modifier,
			  const utf8::string& unicode);

	/**
	 * Fires an event which takes notification parameters.
	 *
	 * @note the void* parameter is a dummy needed for SFINAE.
	 *
	 * @param event                  The event to fire.
	 * @param target                 The widget that should receive the event.
	 */
	bool fire(const tevent event, twidget& target, void*);

	/**
	 * Fires an event which takes message parameters.
	 *
	 * @param event                  The event to fire.
	 * @param target                 The widget that should receive the event.
	 *                               Normally this is the window holding the
	 *                               widget.
	 * @param message                The extra information needed for a window
	 *                               (or another widget in the chain) to handle
	 *                               the message.
	 */
	bool fire(const tevent event, twidget& target, tmessage& message);

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
	enum tposition {
		front_pre_child,
		back_pre_child,
		front_child,
		back_child,
		front_post_child,
		back_post_child
	};

	/**
	 * Connect a signal for callback in tset_event.
	 *
	 * The function uses some boost magic to avoid registering the wrong
	 * function, but the common way to use this function is:
	 * widget->connect_signal<EVENT_ID>(
	 * boost::bind(&tmy_dialog::my_member, this));
	 * This allows simply adding a member of a dialog to be used as a callback
	 * for widget without a lot of magic. Note most widgets probably will get a
	 * callback like
	 * connect_signal_mouse_left_click(const tsignal_function& callback)
	 * which hides this function for the average use.
	 *
	 * @tparam E                     The event the callback needs to react to.
	 * @param signal                 The callback function.
	 * @param position               The position to place the callback.
	 */
	template <tevent E>
	typename boost::enable_if<boost::mpl::has_key<tset_event,
												  boost::mpl::int_<E> > >::type
	connect_signal(const tsignal_function& signal,
				   const tposition position = back_child)
	{
		signal_queue_.connect_signal(E, position, signal);
	}

	/**
	 * Disconnect a signal for callback in tset_event.
	 *
	 * @tparam E                     The event the callback was used for.
	 * @param signal                 The callback function.
	 * @param position               The place where the function was added.
	 *                               Needed remove the event from the right
	 *                               place. (The function doesn't care whether
	 *                               was added in front or back.)
	 */
	template <tevent E>
	typename boost::enable_if<boost::mpl::has_key<tset_event,
												  boost::mpl::int_<E> > >::type
	disconnect_signal(const tsignal_function& signal,
					  const tposition position = back_child)
	{
		signal_queue_.disconnect_signal(E, position, signal);
	}

	/**
	 * Connect a signal for callback in tset_event_mouse.
	 *
	 * @tparam E                     The event the callback needs to react to.
	 * @param signal                 The callback function.
	 * @param position               The position to place the callback.
	 */
	template <tevent E>
	typename boost::enable_if<boost::mpl::has_key<tset_event_mouse,
												  boost::mpl::int_<E> > >::type
	connect_signal(const tsignal_mouse_function& signal,
				   const tposition position = back_child)
	{
		signal_mouse_queue_.connect_signal(E, position, signal);
	}

	/**
	 * Disconnect a signal for callback in tset_event_mouse.
	 *
	 * @tparam E                     The event the callback was used for.
	 * @param signal                 The callback function.
	 * @param position               The place where the function was added.
	 *                               Needed remove the event from the right
	 *                               place. (The function doesn't care whether
	 *                               was added in front or back.)
	 */
	template <tevent E>
	typename boost::enable_if<boost::mpl::has_key<tset_event_mouse,
												  boost::mpl::int_<E> > >::type
	disconnect_signal(const tsignal_mouse_function& signal,
					  const tposition position = back_child)
	{
		signal_mouse_queue_.disconnect_signal(E, position, signal);
	}

	/**
	 * Connect a signal for callback in tset_event_keyboard.
	 *
	 * @tparam E                     The event the callback needs to react to.
	 * @param signal                 The callback function.
	 * @param position               The position to place the callback.
	 */
	template <tevent E>
	typename boost::enable_if<boost::mpl::has_key<tset_event_keyboard,
												  boost::mpl::int_<E> > >::type
	connect_signal(const tsignal_keyboard_function& signal,
				   const tposition position = back_child)
	{
		signal_keyboard_queue_.connect_signal(E, position, signal);
	}

	/**
	 * Disconnect a signal for callback in tset_event_keyboard.
	 *
	 * @tparam E                     The event the callback was used for.
	 * @param signal                 The callback function.
	 * @param position               The place where the function was added.
	 *                               Needed remove the event from the right
	 *                               place. (The function doesn't care whether
	 *                               was added in front or back.)
	 */
	template <tevent E>
	typename boost::enable_if<boost::mpl::has_key<tset_event_keyboard,
												  boost::mpl::int_<E> > >::type
	disconnect_signal(const tsignal_keyboard_function& signal,
					  const tposition position = back_child)
	{
		signal_keyboard_queue_.disconnect_signal(E, position, signal);
	}

	/**
	 * Connect a signal for callback in tset_event_notification.
	 *
	 * @tparam E                     The event the callback needs to react to.
	 * @param signal                 The callback function.
	 * @param position               The position to place the callback. Since
	 *                               the message is send to a widget directly
	 *                               the pre and post positions make no sense
	 *                               and shouldn't be used.
	 */
	template <tevent E>
	typename boost::enable_if<boost::mpl::has_key<tset_event_notification,
												  boost::mpl::int_<E> > >::type
	connect_signal(const tsignal_notification_function& signal,
				   const tposition position = back_child)
	{
		signal_notification_queue_.connect_signal(E, position, signal);
	}

	/**
	 * Disconnect a signal for callback in tset_event_notification.
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
	template <tevent E>
	typename boost::enable_if<boost::mpl::has_key<tset_event_notification,
												  boost::mpl::int_<E> > >::type
	disconnect_signal(const tsignal_notification_function& signal,
					  const tposition position = back_child)
	{
		signal_notification_queue_.disconnect_signal(E, position, signal);
	}

	/**
	 * Connect a signal for callback in tset_event_message.
	 *
	 * @tparam E                     The event the callback needs to react to.
	 * @param signal                 The callback function.
	 * @param position               The position to place the callback. Since
	 *                               the message is send to a widget directly
	 *                               the pre and post positions make no sense
	 *                               and shouldn't be used.
	 */
	template <tevent E>
	typename boost::enable_if<boost::mpl::has_key<tset_event_message,
												  boost::mpl::int_<E> > >::type
	connect_signal(const tsignal_message_function& signal,
				   const tposition position = back_child)
	{
		signal_message_queue_.connect_signal(E, position, signal);
	}

	/**
	 * Disconnect a signal for callback in tset_event_message.
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
	template <tevent E>
	typename boost::enable_if<boost::mpl::has_key<tset_event_message,
												  boost::mpl::int_<E> > >::type
	disconnect_signal(const tsignal_message_function& signal,
					  const tposition position = back_child)
	{
		signal_message_queue_.disconnect_signal(E, position, signal);
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
	enum tmouse_behavior {
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

	void set_mouse_behavior(const tmouse_behavior mouse_behavior)
	{
		mouse_behavior_ = mouse_behavior;
	}

	tmouse_behavior get_mouse_behavior() const
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
	template <class T>
	struct tsignal
	{
		tsignal() : pre_child(), child(), post_child()
		{
		}

		std::vector<T> pre_child;
		std::vector<T> child;
		std::vector<T> post_child;
	};

	/** Helper struct to generate the various event queues. */
	template <class T>
	struct tsignal_queue
	{
		tsignal_queue() : queue()
		{
		}

		std::map<tevent, tsignal<T> > queue;

		void connect_signal(const tevent event,
							const tposition position,
							const T& signal)
		{
			switch(position) {
				case front_pre_child:
					queue[event].pre_child.insert(
							queue[event].pre_child.begin(), signal);
					break;
				case back_pre_child:
					queue[event].pre_child.push_back(signal);
					break;

				case front_child:
					queue[event]
							.child.insert(queue[event].child.begin(), signal);
					break;
				case back_child:
					queue[event].child.push_back(signal);
					break;

				case front_post_child:
					queue[event].post_child.insert(
							queue[event].post_child.begin(), signal);
					break;
				case back_post_child:
					queue[event].post_child.push_back(signal);
					break;
			}
		}

		void disconnect_signal(const tevent event,
							   const tposition position,
							   const T& signal)
		{
			/*
			 * The function doesn't differentiate between front and back
			 * position so fall down from front to back.
			 */
			switch(position) {
				case front_pre_child:
				case back_pre_child: {
					tsignal<T>& signal_queue = queue[event];
					for(typename std::vector<T>::iterator itor
						= signal_queue.child.begin();
						itor != signal_queue.child.end();
						++itor) {

						if(signal.target_type() == itor->target_type()) {
							signal_queue.child.erase(itor);
							return;
						}
					}
				} break;

				case front_child:
				case back_child: {
					tsignal<T>& signal_queue = queue[event];
					for(typename std::vector<T>::iterator itor
						= signal_queue.child.begin();
						itor != signal_queue.child.end();
						++itor) {

						if(signal.target_type() == itor->target_type()) {
							signal_queue.child.erase(itor);
							return;
						}
					}
				} break;

				case front_post_child:
				case back_post_child: {
					tsignal<T>& signal_queue = queue[event];
					for(typename std::vector<T>::iterator itor
						= signal_queue.child.begin();
						itor != signal_queue.child.end();
						++itor) {

						if(signal.target_type() == itor->target_type()) {
							signal_queue.child.erase(itor);
							return;
						}
					}
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
						 const thotkey_function& function);

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
	tmouse_behavior mouse_behavior_;

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

	/** Signal queue for callbacks in tset_event. */
	tsignal_queue<tsignal_function> signal_queue_;

	/** Signal queue for callbacks in tset_event_mouse. */
	tsignal_queue<tsignal_mouse_function> signal_mouse_queue_;

	/** Signal queue for callbacks in tset_event_keyboard. */
	tsignal_queue<tsignal_keyboard_function> signal_keyboard_queue_;

	/** Signal queue for callbacks in tset_event_notification. */
	tsignal_queue<tsignal_notification_function> signal_notification_queue_;

	/** Signal queue for callbacks in tset_event_message. */
	tsignal_queue<tsignal_message_function> signal_message_queue_;

	/** Are we connected to the event handler. */
	bool connected_;

	/** The registered hotkeys for this dispatcher. */
	std::map<hotkey::HOTKEY_COMMAND, thotkey_function> hotkeys_;
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
inline void
connect_signal_pre_key_press(tdispatcher& dispatcher,
							 const tsignal_keyboard_function& signal)
{
	dispatcher.connect_signal<SDL_KEY_DOWN>(signal, tdispatcher::front_child);
}

/** Connects a signal handler for a left mouse button click. */
inline void connect_signal_mouse_left_click(tdispatcher& dispatcher,
											const tsignal_function& signal)
{
	dispatcher.connect_signal<LEFT_BUTTON_CLICK>(signal);
}

/** Disconnects a signal handler for a left mouse button click. */
inline void disconnect_signal_mouse_left_click(tdispatcher& dispatcher,
											   const tsignal_function& signal)
{
	dispatcher.disconnect_signal<LEFT_BUTTON_CLICK>(signal);
}

/** Connects a signal handler for getting a notification upon modification. */
inline void
connect_signal_notify_modified(tdispatcher& dispatcher,
							   const tsignal_notification_function& signal)
{
	dispatcher.connect_signal<event::NOTIFY_MODIFIED>(signal);
}

} // namespace event

} // namespace gui2

#endif
