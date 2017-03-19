/*
   Copyright (C) 2009 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/core/event/dispatcher_private.hpp"

#include "gui/core/log.hpp"

namespace gui2
{

namespace event
{

/***** dispatcher class. *****/

dispatcher::dispatcher()
	: mouse_behavior_(all)
	, want_keyboard_input_(true)
	, signal_queue_()
	, signal_mouse_queue_()
	, signal_keyboard_queue_()
	, signal_notification_queue_()
	, signal_message_queue_()
	, connected_(false)
	, hotkeys_()
{
}

dispatcher::~dispatcher()
{
	if(connected_) {
		disconnect_dispatcher(this);
	}
}

void dispatcher::connect()
{
	assert(!connected_);
	connected_ = true;
	connect_dispatcher(this);
}

bool dispatcher::has_event(const ui_event event, const event_queue_type event_type)
{
#if 0
	// Debug code to test whether the event is in the right queue.
	std::cerr << "Event '" << event
			<< "' event "
			<< find<set_event>(event, dispatcher_implementation
				::has_handler(event_type, *this))
			<< " mouse "
			<< find<set_event_mouse>(event, dispatcher_implementation
				::has_handler(event_type, *this))
			<< " keyboard "
			<< find<set_event_keyboard>(event, dispatcher_implementation
				::has_handler(event_type, *this))
			<< " notification "
			<< find<set_event_notification>(event, dispatcher_implementation
				::has_handler(event_type, *this))
			<< " message "
			<< find<set_event_message>(event, dispatcher_implementation
				::has_handler(event_type, *this))
			<< ".\n";
#endif

	return find<set_event>(
				   event,
				   dispatcher_implementation::has_handler(event_type, *this))
		   || find<set_event_mouse>(event,
									 dispatcher_implementation::has_handler(
											 event_type, *this))
		   || find<set_event_keyboard>(
					  event,
					  dispatcher_implementation::has_handler(event_type,
															   *this))
		   || find<set_event_touch>(
					  event,
					  dispatcher_implementation::has_handler(event_type,
															   *this))
		   || find<set_event_notification>(
					  event,
					  dispatcher_implementation::has_handler(event_type,
															   *this))
		   || find<set_event_message>(event,
									   dispatcher_implementation::has_handler(
											   event_type, *this));
}

/**
 * Helper class to do a runtime test whether an event is in a set.
 *
 * The class is supposed to be used in combination with find function. This
 * function is used in the fire functions to make sure an event is send to the
 * proper handler. If not there will be a run-time assertion failure. This
 * makes developing and testing the code easier, a wrong handler terminates
 * Wesnoth instead of silently not working.
 */
class event_in_set
{
public:
	/**
	 * If found we get executed to set the result.
	 *
	 * Since we need to return true if found we always return true.
	 */
	template <class T>
	bool oper(ui_event)
	{
		return true;
	}
};

/**
 * Helper struct to wrap the functor call.
 *
 * The template function @ref fire_event needs to call a functor with extra
 * parameter. In order to facilitate this we send the parameter in the
 * constructor of the class and let operator() call the functor with the
 * default parameters and the stored parameters. This allows the core part of
 * @ref dispatcher::fire to be generic.
 */
class trigger
{
public:
	void operator()(signal_function functor,
					dispatcher& dispatcher,
					const ui_event event,
					bool& handled,
					bool& halt) const
	{
		functor(dispatcher, event, handled, halt);
	}
};

bool dispatcher::fire(const ui_event event, widget& target)
{
	assert(find<set_event>(event, event_in_set()));
	switch(event) {
		case LEFT_BUTTON_DOUBLE_CLICK:
			return fire_event_double_click<LEFT_BUTTON_CLICK,
										   LEFT_BUTTON_DOUBLE_CLICK,
										   &event_executor::
													wants_mouse_left_double_click,
										   signal_function>(
					dynamic_cast<widget*>(this), &target, trigger());

		case MIDDLE_BUTTON_DOUBLE_CLICK:
			return fire_event_double_click<MIDDLE_BUTTON_CLICK,
										   MIDDLE_BUTTON_DOUBLE_CLICK,
										   &event_executor::
													wants_mouse_middle_double_click,
										   signal_function>(
					dynamic_cast<widget*>(this), &target, trigger());

		case RIGHT_BUTTON_DOUBLE_CLICK:
			return fire_event_double_click<RIGHT_BUTTON_CLICK,
										   RIGHT_BUTTON_DOUBLE_CLICK,
										   &event_executor::
													wants_mouse_right_double_click,
										   signal_function>(
					dynamic_cast<widget*>(this), &target, trigger());

		default:
			return fire_event<signal_function>(
					event, dynamic_cast<widget*>(this), &target, trigger());
	}
}

/** Helper struct to wrap the functor call. */
class trigger_mouse
{
public:
	explicit trigger_mouse(const point& coordinate) : coordinate_(coordinate)
	{
	}

	void operator()(signal_mouse_function functor,
					dispatcher& dispatcher,
					const ui_event event,
					bool& handled,
					bool& halt)
	{
		functor(dispatcher, event, handled, halt, coordinate_);
	}

private:
	point coordinate_;
};

bool
dispatcher::fire(const ui_event event, widget& target, const point& coordinate)
{
	assert(find<set_event_mouse>(event, event_in_set()));
	return fire_event<signal_mouse_function>(event,
											  dynamic_cast<widget*>(this),
											  &target,
											  trigger_mouse(coordinate));
}

/** Helper struct to wrap the functor call. */
class trigger_keyboard
{
public:
	trigger_keyboard(const SDL_Keycode key,
					  const SDL_Keymod modifier,
					  const utf8::string& unicode)
		: key_(key), modifier_(modifier), unicode_(unicode)
	{
	}

	void operator()(signal_keyboard_function functor,
					dispatcher& dispatcher,
					const ui_event event,
					bool& handled,
					bool& halt)
	{
		functor(dispatcher, event, handled, halt, key_, modifier_, unicode_);
	}

private:
	SDL_Keycode key_;
	SDL_Keymod modifier_;
	utf8::string unicode_;
};

bool dispatcher::fire(const ui_event event,
					   widget& target,
					   const SDL_Keycode key,
					   const SDL_Keymod modifier,
					   const utf8::string& unicode)
{
	assert(find<set_event_keyboard>(event, event_in_set()));
	return fire_event<signal_keyboard_function>(
			event,
			dynamic_cast<widget*>(this),
			&target,
			trigger_keyboard(key, modifier, unicode));
}

/** Helper struct to wrap the functor call. */
class trigger_touch
{
public:
	trigger_touch(const point& pos, const point& distance)
		: pos_(pos)
		, distance_(distance)
	{
	}

	void operator()(signal_touch_function functor,
					dispatcher& dispatcher,
					const ui_event event,
					bool& handled,
					bool& halt)
	{
		functor(dispatcher, event, handled, halt, pos_, distance_);
	}

private:
	point pos_;
	point distance_;
};

bool dispatcher::fire(const ui_event event,
					   widget& target,
					   const point& pos,
					   const point& distance)
{
	assert(find<set_event_touch>(event, event_in_set()));
	return fire_event<signal_touch_function>(
			event,
			dynamic_cast<widget*>(this),
			&target,
			trigger_touch(pos, distance));
}

/** Helper struct to wrap the functor call. */
class trigger_notification
{
public:
	void operator()(signal_notification_function functor,
					dispatcher& dispatcher,
					const ui_event event,
					bool& handled,
					bool& halt) const
	{
		functor(dispatcher, event, handled, halt, nullptr);
	}
};

bool dispatcher::fire(const ui_event event, widget& target, void*)
{
	assert(find<set_event_notification>(event, event_in_set()));
	return fire_event<signal_notification_function>(
			event,
			dynamic_cast<widget*>(this),
			&target,
			trigger_notification());
}

/** Helper struct to wrap the functor call. */
class trigger_message
{
public:
	explicit trigger_message(message& msg) : message_(msg)
	{
	}

	void operator()(signal_message_function functor,
					dispatcher& dispatcher,
					const ui_event event,
					bool& handled,
					bool& halt)
	{
		functor(dispatcher, event, handled, halt, message_);
	}

private:
	message& message_;
};

bool dispatcher::fire(const ui_event event, widget& target, message& msg)
{
	assert(find<set_event_message>(event, event_in_set()));
	return fire_event<signal_message_function>(event,
												dynamic_cast<widget*>(this),
												&target,
												trigger_message(msg));
}

void dispatcher::register_hotkey(const hotkey::HOTKEY_COMMAND id,
								  const thotkey_function& function)
{
	hotkeys_[id] = function;
}

bool dispatcher::execute_hotkey(const hotkey::HOTKEY_COMMAND id)
{
	std::map<hotkey::HOTKEY_COMMAND, thotkey_function>::iterator itor
			= hotkeys_.find(id);

	if(itor == hotkeys_.end()) {
		return false;
	}

	return itor->second(*this, id);
}

} // namespace event

} // namespace gui2

/**
 * @page event_dispatching Event dispatching.
 *
 * @section introduction Introduction
 *
 * This page describes how the new event handling system works, since the
 * system is still work in progress it might be out of date with the actual
 * code. It also contains some ideas that might change later on. Some parts are
 * explained in the interface and will be integrated in this document later.
 *
 * Since the event handling code hasn't been cast in stone yet some scenarios
 * for solving the problem are discussed first and then the solution that is
 * chosen in more detail.
 *
 * After SDL has generated and event it needs to be turned into an event which
 * the widgets can use.
 *
 * @section handling_solution The implementation solutions.
 *
 * For the event handling we use a few use case scenarios and show the possible
 * solutions.
 *
 * @subsection sample The sample window
 *
 * In our samples we use this sample window with the following components:
 * - a window W
 * - a container C
 * - a button B
 *
 * These are arranged accordingly:
 * @code
 *
 *   ---------------------
 *  |W                     |
 *  |                      |
 *  |  -----------------   |
 *  | |C              |^|  |
 *  | |               |-|  |
 *  | |  ----------   |#|  |
 *  | | |B         |  | |  |
 *  | |  ----------   | |  |
 *  | |               |-|  |
 *  | |               |v|  |
 *  |  -----------------   |
 *  |                      |
 *   ---------------------
 *
 * @endcode
 *
 * @subsection scenarios Possible scenarios
 *
 * The scenarios are:
 * - An event that is wanted by none.
 * - A mouse down event that should focus C and set the pressed state in B.
 * - A mouse wheel event, which first should be offered to B and if not handled
 *   by B should be handled by C.
 *
 * @subsection all_queues Pass the event through all queues
 *
 * In this solution the event will be passed through all possible queues and
 * tries sees where the event sticks. This following sections describe how the
 * events are tried for this usage scenario.
 *
 * @subsubsection unhandled Unhandled event
 *
 * - W pre child
 * - C pre child
 * - B pre child
 * - W child
 * - C child
 * - B child
 * - W post child
 * - C post child
 * - B post child
 *
 * @subsubsection mouse_down Mouse down
 *
 * - W pre child
 * - C pre child -> set focus -> !handled
 * - B pre child -> set pressed state -> handled
 *
 * @subsubsection mouse_wheel Mouse wheel
 *
 * - W pre child
 * - C pre child
 * - B pre child -> We can't scroll so ignore
 * - W child
 * - C child
 * - B child
 * - W post child
 * - C post child -> Scroll -> handled
 *
 * @subsection chain Pass the events in a chain like fashion
 *
 * In this solution the events are send to the pre- and post queue of all but
 * the last possible widget and to the child of the last widget. The pre queue
 * will be send from top to bottom, the post queue from bottom to top.
 *
 * @subsubsection unhandled Unhandled event
 *
 * - W pre child
 * - C pre child
 * - B child
 * - C post child
 * - W post child
 *
 * @subsubsection mouse_down Mouse down
 *
 * - W pre child
 * - C pre child -> set focus -> !handled
 * - B child -> set pressed state -> handled
 *
 * @subsubsection mouse_wheel Mouse wheel
 *
 * - W pre child
 * - C pre child
 * - B child -> We can't scroll so ignore
 * - C post child -> Scroll -> handled
 *
 * @subsection evaluation Evaluation
 *
 * When using the first solution it's possible to drop the child queue since
 * everything falls in pre or post. But there is a scenario that's a bit ugly
 * to solve with the first solution:
 *
 * Assume there is a listbox with toggle panels and on the panel there are a
 * few buttons, the wanted behavior is:
 * - if clicked on the panel it should toggle, which may or may not be allowed.
 * - if clicked on a button in the panel, we want to make sure the panel is
 *   selected, which again may or may not be allowed.
 *
 * With solution 2 it's rather easy:
 *
 * Click on panel:
 * - W pre child
 * - C child -> Test whether we can toggle -> handled, halt = !toggled
 *
 * Click on button in panel:
 * - W pre child
 * - C pre child -> Test whether we can select -> handled = halt = !selected
 * - B child -> do button stuff -> handled
 *
 * Since for the different clicks, different queues are triggered it's easy to
 * add a different handler there.
 *
 * With solution 1:
 *
 * Click on panel:
 * - W pre child
 * - C pre child -> handler 1 -> if last in queue -> solution 2 C child
 *
 * Click on button in panel:
 * - W pre child
 * - C pre child -> handler 2 -> if !last in queue -> solution 2 C pre child
 * - B pre child -> do button stuff -> handled
 *
 * Not that different from solution 2, the two handlers are installed in the C
 * pre event. But we need to manually check whether we're really the last,
 * which means the code to check whether there are more handlers at a lower
 * level is needed for both solutions. In solution 1 this test needs to be done
 * twice versus once in solution 2. Also the fact that the queues for the
 * events are processed in reverse order on the way back sounds more
 * initiative.
 *
 * @section processing_raw_events Processing the raw events.
 *
 * This section describes how the events generated by SDL are send as our own
 * events to the various widgets. The first step in sending an event is to
 * decode it and send it to a registered dispatcher.
 *
 * - gui2::event::sdl_event_handler handles the SDL events.
 * - gui2::event::dispatcher has the registered dispatchers.
 *
 * In general a dispatcher is a window which then needs to send this event to
 * the widgets. The dispatcher is just a simple part which fires events and
 * finds a handler for the event. This is not to the liking of most widgets,
 * they don't want to handle raw events but get a polished and clean event. No
 * button up and down and then try to figure out whether it needs to act as if
 * it was clicked upon, no simply op and down to change the appearance and a
 * click event to do the clicking actions. And don't even try to convince a
 * widget to determine whether this up event was a single or double click.
 * Widgets like to sleep with nice dreams and not having nightmares where SDL
 * events haunt them.
 *
 * In order to remedy that problem there's the gui2::event::distributor
 * class, it's the class to do the dirty job of converting the raw event into
 * these nice polished events. The distributor is in general linked to a window,
 * but a widget can install it's own distributor if it needs to know more of the
 * raw events as still left in the polished events. At the time of this writing
 * no widget needs this feature, but the toggle panel might need it.
 *
 * After the distributor has polished the event and send it on its way to the
 * widget the dispatcher needs to make sure the event is properly dispatched to
 * the widget in question and also notify its parents by means of the previously
 * described event chain.
 *
 * @subsection sdl_event Get the SDL events
 *
 * The first step in event handling is getting the events in the first place.
 * Events are generated by SDL and placed in a queue. The Wesnoth code processes
 * this queue and thus handles the events. The part which does the first
 * handling isn't described here since it's (secretly) intended to be replaced
 * by the @ref gui2::event::sdl_event_handler class. Instead we directly jump to this
 * class and explain what it does.
 *
 * The main handling function is @ref gui2::event::sdl_event_handler::handle_event which
 * as no real surprise handles the events. The function is a simple multiplexer
 * which lets other subfunctions to the handling of specific events.
 *
 * @todo Describe drawing and resizing once the code is stable and working as
 * wanted in these areas.
 *
 * @subsubsection handler_mouse Mouse motion events
 *
 * If a dispatcher has captured the mouse it gets the event, no questions asked.
 * If not it goes through all dispatchers and finds the first one willing to
 * accept the mouse event.
 *
 * This means a mouse event is send to one dispatcher.
 *
 * @subsubsection handler_mouse_button_down Mouse button down events
 *
 * Turning the mouse wheel on a mouse generates both an down and up event. It
 * has been decided to handle the wheel event in the button up code so wheel
 * events are here directly dropped on the floor and forgotten.
 *
 * The other buttons are handled as if they're normal mouse events but are
 * decoded per button so instead of a button_down(id) you get button_down_id.
 *
 * @subsubsection handler_mouse_button_up Mouse button up events
 *
 * The mouse wheel event is handled as if it's a keyboard event and like the
 * button_down they are send as wheel_id events.
 *
 * The other mouse buttons are handled the same as the down buttons.
 *
 * @subsubsection handler_keyboard Keyboard events
 *
 * There are three types of keyboard events, the already mentioned mouse wheel
 * events, the key down and key up event. When a key is pressed for a longer
 * time several key down events are generated and only one key up, this means
 * the key up is rather useless. Guess what, the multiplexer already drops that
 * event so we never get it.
 *
 * If the keyboard event is a mouse wheel event it's directly send to the
 * dispachting queue; either the dispatcher that captured the keyboard or the
 * last dispatcher in the queue.
 *
 * If the event is a real keyboard action it's first tried as hotkey. In order
 * to do so the target dispatcher is first determined, either the dispatcher
 * that captured the keyboard or the last dispatcher in the queue. Then it's
 * tried whether a hotkey and whether the hotkey can be processed. If the
 * hotkey isn't processed the keyboard event is send to the dispatcher as
 * normal keyboard event.
 *
 * The hotkey processing will have several queues (to be implemented in 1.9):
 * - global hotkeys that always work eg toggling fullscreen mode.
 * - main screen hotkeys, these work when one of the dialogs is shown without
 *   other dialogs on top of them. These hotkeys are for example
 *   preferences. The main screens are:
 *   - title screen
 *   - game
 *   - editor
 *   - mp lobby
 * - map screen hotkeys, these work when a map is shown eg toggle grid. The
 *   screens are:
 *   - game
 *   - editor
 * - local hotkeys, these are hotkeys that only work in a specific dialog eg
 *   recruit unit only works in the game screen.
 *
 * The queues are processed in from bottom to top in the list above, this
 * allows an item to use a hotkey but have another handler function. Eg
 * preferences in the editor might open another preferences dialog.
 *
 * @todo The hotkeys need to be implemented like above in 1.9.
 *
 * @todo This might change in the near future.
 *
 * @subsection distributor Event polishing and distribution
 *
 * The event distributor has the job to find the widget that should receive the
 * event and which event(s) to send from a single event. In general an event is
 * first send to the widget as-is, sending the raw events allows other
 * distributors to be nested between this distributor and the intended target
 * widget. Or the intended widget might not really be the intended widget but
 * another distributor that wants to dispatch the event internally.
 *
 * However in the common cases this raw event isn't handled and the distributor
 * needs to send the polished events. In the following sections the details of
 * the conversion from raw to polished is described, it intentionally lacks the
 * part of sending the raw events as well since it adds no value.
 *
 * A widget can capture the mouse, which means all mouse events are send to this
 * widget, regardless where the mouse is. This is normally done in a mouse down
 * event (for a button) so all events following it are send to that widget.
 *
 * @subsection mouse_motion Mouse motion
 *
 * This section describes the conversion from a raw mouse motion to the polished
 * events it can generate:
 * - @ref gui2::event::MOUSE_ENTER "MOUSE_ENTER"
 * - @ref gui2::event::MOUSE_LEAVE "MOUSE_LEAVE"
 * - @ref gui2::event::MOUSE_MOTION "MOUSE_MOTION"
 *
 * When the mouse is captured that widget will only receive motion events.
 *
 * If not captured the code checks whether the widget underneath the mouse is
 * the same widget as at the last motion if event. If so that widget gets a
 * motion event.
 * If not the widget that before was underneath the mouse pointer (if any) gets
 * a leave event and the widget newly underneath the mouse pointer (if any) gets
 * an enter event.
 *
 * @subsection mouse_button Mouse buttons
 *
 * The mouse button code is a bit more complex and is separated in the various
 * events to be send.
 *
 * @subsubsection mouse_button_down Mouse button down
 *
 * Some things start simple, so does the event of pressing down a mouse button.
 * All it does is send the event to the widget as one of the following events:
 * - @ref gui2::event::LEFT_BUTTON_DOWN "LEFT_BUTTON_DOWN"
 * - @ref gui2::event::MIDDLE_BUTTON_DOWN "MIDDLE_BUTTON_DOWN"
 * - @ref gui2::event::RIGHT_BUTTON_DOWN "RIGHT_BUTTON_DOWN"
 *
 * @todo Validate the code it seems a down event with a captured mouse doesn't
 * really work as wanted. (Rare case but should work properly.) In general the
 * mouse event handling needs testing to see whether the proper events are send
 * all the time.
 *
 * @subsubsection mouse_button_up Mouse button up
 *
 * Simplicity ends here.
 *
 * @todo Document further.
 *
 * @subsubsection mouse_click Mouse click
 *
 * So the button up event has asked for mouse click, now we need to test whether
 * the click will be a click or a double click. A double click is generated when
 * the same widget is clicked twice in a short time and causes the following
 * events:
 * - @ref gui2::event::LEFT_BUTTON_DOUBLE_CLICK "LEFT_BUTTON_DOUBLE_CLICK"
 * - @ref gui2::event::MIDDLE_BUTTON_DOUBLE_CLICK "MIDDLE_BUTTON_DOUBLE_CLICK"
 * - @ref gui2::event::RIGHT_BUTTON_DOUBLE_CLICK "RIGHT_BUTTON_DOUBLE_CLICK"
 *
 * Otherwise one of the following single clicks is generated:
 * - @ref gui2::event::LEFT_BUTTON_CLICK "LEFT_BUTTON_CLICK"
 * - @ref gui2::event::MIDDLE_BUTTON_CLICK "MIDDLE_BUTTON_CLICK"
 * - @ref gui2::event::RIGHT_BUTTON_CLICK "RIGHT_BUTTON_CLICK"
 *
 * @subsubsection double_click To double click or not to double click
 *
 * Wait a second, a widget has a field whether or not it wants a double click
 * for a certain mouse button and now I see that it's bluntly ignored by the
 * distributor. Indeed the distributor doesn't care about what the widget wants,
 * it does what it wants and leaves the sorting out what's wanted to the
 * dispatcher.
 *
 * The problem is that in the chain events are send to one widget that may not
 * be interested in a double click, but another widget in the chain is. There
 * are several solutions to this problem:
 * -# Sending a click followed by a double click.
 * -# Sending a click with a tag field that it actually is a double click.
 * -# Send a double click and turn it into a click if the double click is
 *    unwanted.
 *
 * The first solution has the disadvantage that a toggle panel likes a click and
 * double click, the first click selects the second deselects and now the
 * deselected panel gets a double click. When the panel now checks whether it's
 * selected it's not and might take the wrong action upon it.
 *
 * The second option is possible but would be rather intrusive in the code,
 * since it would generate another event signature. Adding a signature just for
 * this special case seemed a bit too much effort vs. gain. Also the widget
 * needs to check whether a click is a click or a double click and choose a
 * different code path for it. This in turn would mean a signal handler
 * secretly might handle two events and lowers the transparency of the code.
 *
 * The third option also adds some special case handling but the scope is
 * limited and only one part knows about the tricks done.
 *
 * The last option has been chosen and the dispatcher build the event chain and
 * while building the chain it looks whether the widget wants the double click
 * or not. It does this test by looking at the wants double click function and
 * not test for a handler. The double click test function is made for this
 * purpose and depending on the handler might again do the wrong thing.
 * (A certain toggle panel might not want to do something on a double click but
 * also not being deselected upon a double click. The latter to keep the UI
 * consistent, a double click on a toggle panel might execute a special function
 * or not, but always keep the panel selected. (That is if the panel can be
 * selected.))
 */
