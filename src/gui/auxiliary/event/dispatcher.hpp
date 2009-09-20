/* $Id$ */
/*
   Copyright (C) 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_AUXILIARY_EVENT_DISPATCHER_HPP_INCLUDED
#define GUI_WIDGETS_AUXILIARY_EVENT_DISPATCHER_HPP_INCLUDED

#include "gui/auxiliary/event/handler.hpp"

#include <boost/function.hpp>
#include <boost/mpl/int.hpp>
#include <boost/utility/enable_if.hpp>

#include <vector>
#include <map>

namespace gui2 {

class tpoint;

namespace event {

/**
 * Callback function signature.
 *
 * There are several kinds of callback signature, this only has the parameters
 * shared by all callbacks.
 *
 * This function is used for the callbacks in tset_event.
 */
typedef
		boost::function<void(const tevent event
			, bool& handled
			, bool& halt)>
		tsignal_function;

/**
 * Callback function signature.
 *
 * This function is used for the callbacks in tset_event_mouse.
 */
typedef
		boost::function<void(const tevent event
			, bool& handled
			, bool& halt
			, const tpoint& coordinate)>
		tsignal_mouse_function;

/**
 * Callback function signature.
 *
 * This function is used for the callbacks in tset_event_keyboard.
 */
typedef
		boost::function<void(const tevent event
			, bool& handled
			, bool& halt) >
		tsignal_keyboard_function;


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

	/** Fires an event which has no extra parameters. */
	void fire(const tevent event);

	/**
	 * Fires an event which takes a coordinate parameter.
	 *
	 * @param coordinate             The mouse position for the event.
	 */
	void fire(const tevent event, const tpoint& coordinate);

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
	 * For every queue it's posible to add a new event in the front or in the
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
	 *   exectuted afterwards.
	 *
	 * A toggle button may or may not be toggled:
	 * * The user inserts a callback, that validates whether the action is
	 *   allowed, if not allowed it sets the halt flag (and handled), else
	 *   leaves the flags untouched.
	 * * The normal buttons toggle function then might get invoked and if so
	 *   sets the handled flag.
	 * * Optionally there is another user callback invoked at this point.
	 */
	enum tposition
	{
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
	 * for widget without a lot of magic. Note most widgets probaly will get a
	 * callback like
	 * set_callback_mouse_left_click(const tsignal_function& callback)
	 * which hides this function for the avarage use.
	 *
	 * @tparam E                     The event the callback needs to react to.
	 * @param signal                 The callback function.
	 * @param position               The position to place the callback.
	 */
	template<tevent E>
	typename boost::enable_if<boost::mpl::has_key<
			tset_event, boost::mpl::int_<E> > >::type
	connect_signal(const tsignal_function& signal
			, const tposition position = back_pre_child)
	{
		signal_queue_.connect_signal(E, position, signal);
	}

	/**
	 * Connect a signal for callback in tset_event_mouse.
	 *
	 * @tparam E                     The event the callback needs to react to.
	 * @param signal                 The callback function.
	 * @param position               The position to place the callback.
	 */
	template<tevent E>
	typename boost::enable_if<boost::mpl::has_key<
			tset_event_mouse, boost::mpl::int_<E> > >::type
	connect_signal(const tsignal_mouse_function& signal
			, const tposition position = back_pre_child)
	{
		signal_mouse_queue_.connect_signal(E, position, signal);
	}

	/**
	 * Connect a signal for callback in tset_event_keyboard.
	 *
	 * @tparam E                     The event the callback needs to react to.
	 * @param signal                 The callback function.
	 * @param position               The position to place the callback.
	 */
	template<tevent E>
	typename boost::enable_if<boost::mpl::has_key<
			tset_event_keyboard, boost::mpl::int_<E> > >::type
	connect_signal(const tsignal_keyboard_function& signal
			, const tposition position = back_pre_child)
	{
		signal_keyboard_queue_.connect_signal(E, position, signal);
	}

	/**
	 * The behaviour of the mouse events.
	 *
	 * Normally for mouse events there's first cheched whether a dispatcher has
	 * captured the mouse if so it gets the event.
	 * If not the dispatcher is searched from the back to the front in the
	 * layers and it's behavious is checked.
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
	enum tmouse_behaviour
	{
		  all
		, hit
		, none
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
	void set_mouse_behaviour(const tmouse_behaviour mouse_behaviour)
	{
		mouse_behaviour_ = mouse_behaviour;
	}

	tmouse_behaviour get_mouse_behaviour() const
	{
		return mouse_behaviour_;
	}
private:

	/** The mouse behaviour for the dispatcher. */
	tmouse_behaviour mouse_behaviour_;

	/** Helper struct to generate the various signal types. */
	template<class T>
	struct tsignal
	{
		std::vector<T> pre_child;
		std::vector<T> child;
		std::vector<T> post_child;
	};

	/** Helper struct to generate the various event queues. */
	template<class T>
	struct tsignal_queue
	{
		std::map<tevent, tsignal<T> > queue;

		void connect_signal(const tevent event
				, const tposition position
				, const T& signal)
		{
			switch(position) {
				case front_pre_child :
					queue[event].pre_child.insert(
							queue[event].pre_child.begin(), signal);
					break;
				case back_pre_child :
					queue[event].pre_child.push_back(signal);
					break;

				case front_child :
					queue[event].child.insert(
							queue[event].child.begin(), signal);
					break;
				case back_child :
					queue[event].child.push_back(signal);
					break;

				case front_post_child :
					queue[event].post_child.insert(
							queue[event].post_child.begin(), signal);
					break;
				case back_post_child :
					queue[event].post_child.push_back(signal);
					break;

				default :
					assert(false);
			}

		}
	};

	/** Signal queue for callbacks in tset_event. */
	tsignal_queue<tsignal_function> signal_queue_;

	/** Signal queue for callbacks in tset_event_mouse. */
	tsignal_queue<tsignal_mouse_function> signal_mouse_queue_;

	/** Signal queue for callbacks in tset_event_keyboard. */
	tsignal_queue<tsignal_keyboard_function> signal_keyboard_queue_;

	/** Are we connected to the event handler. */
	bool connected_;
};

} // namespace event

} // namespace gui2

#endif

