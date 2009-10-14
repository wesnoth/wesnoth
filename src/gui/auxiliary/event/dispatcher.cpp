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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/auxiliary/event/dispatcher_private.hpp"

#include "foreach.hpp"
#include "gui/auxiliary/log.hpp"

namespace gui2 {

namespace event {

/***** tdispatcher class. *****/

tdispatcher::tdispatcher()
	: mouse_behaviour_(all)
	, signal_queue_()
	, signal_mouse_queue_()
	, signal_keyboard_queue_()
	, signal_notification_queue_()
	, connected_(false)
{
}

tdispatcher::~tdispatcher()
{
	if(connected_) {
		disconnect_dispatcher(this);
	}
}

void tdispatcher::connect()
{
	assert(!connected_);
	connected_ = true;
	connect_dispatcher(this);
}

bool tdispatcher::has_event(const tevent event
		, const tevent_type event_type
		)
{
#if 0
	// Debug code to test whether the event is in the right queue.
	std::cerr << "Event '" << event
			<< "' event "
			<< find<tset_event>(event, tdispatcher_implementation
				::thas_handler(event_type, *this))
			<< " mouse "
			<< find<tset_event_mouse>(event, tdispatcher_implementation
				::thas_handler(event_type, *this))
			<< " keyboard "
			<< find<tset_event_keyboard>(event, tdispatcher_implementation
				::thas_handler(event_type, *this))
			<< " notification "
			<< find<tset_event_notification>(event, tdispatcher_implementation
				::thas_handler(event_type, *this))
			<< ".\n";
#endif

	return find<tset_event>(event, tdispatcher_implementation
					::thas_handler(event_type, *this))
			|| find<tset_event_mouse>(event, tdispatcher_implementation
					::thas_handler(event_type, *this))
			|| find<tset_event_keyboard>(event, tdispatcher_implementation
					::thas_handler(event_type, *this))
			|| find<tset_event_notification>(event, tdispatcher_implementation
					::thas_handler(event_type, *this));
}

/**
 * Helper struct to wrap the functor call.
 *
 * The template function @ref fire_event needs to call a functor with extra
 * parameter. In order to facilitate this we send the parameter in the
 * constructor of the class and let operator() call the functor with the
 * default parameters and the stored parameters. This allows the core part of
 * @ref fire to be generic.
 */
class ttrigger
{
public:
	void operator()(tsignal_function functor
			, tdispatcher& dispatcher
			, const tevent event
			, bool& handled
			, bool& halt)
	{
		functor(dispatcher, event, handled, halt);
	}
};

bool tdispatcher::fire(const tevent event, twidget& target)
{

	switch(event) {
		case LEFT_BUTTON_DOUBLE_CLICK :
			return fire_event_double_click<
					  LEFT_BUTTON_CLICK
					, LEFT_BUTTON_DOUBLE_CLICK
					, &tevent_executor::wants_mouse_left_double_click
					, tsignal_function
					>(
					  dynamic_cast<twidget*>(this)
					, &target
					, ttrigger());

		case MIDDLE_BUTTON_DOUBLE_CLICK :
			return fire_event_double_click<
					  MIDDLE_BUTTON_CLICK
					, MIDDLE_BUTTON_DOUBLE_CLICK
					, &tevent_executor::wants_mouse_middle_double_click
					, tsignal_function
					>(
					  dynamic_cast<twidget*>(this)
					, &target
					, ttrigger());

		case RIGHT_BUTTON_DOUBLE_CLICK :
			return fire_event_double_click<
					  RIGHT_BUTTON_CLICK
					, RIGHT_BUTTON_DOUBLE_CLICK
					, &tevent_executor::wants_mouse_right_double_click
					, tsignal_function
					>(
					  dynamic_cast<twidget*>(this)
					, &target
					, ttrigger());

		default :
			return fire_event<tsignal_function>(event
				, dynamic_cast<twidget*>(this)
				, &target
				, ttrigger());
	}
}

/** Helper struct to wrap the functor call. */
class ttrigger_mouse
{
public:
	ttrigger_mouse(const tpoint& coordinate)
		: coordinate_(coordinate)
	{

	}

	void operator()(tsignal_mouse_function functor
			, tdispatcher& dispatcher
			, const tevent event
			, bool& handled
			, bool& halt)
	{
		functor(dispatcher, event, handled, halt, coordinate_);
	}

private:
	tpoint coordinate_;
};

bool tdispatcher::fire(const tevent event
		, twidget& target
		, const tpoint& coordinate)
{
	return fire_event<tsignal_mouse_function>(event
			, dynamic_cast<twidget*>(this)
			, &target
			, ttrigger_mouse(coordinate));
}

/** Helper struct to wrap the functor call. */
class ttrigger_keyboard
{
public:
	ttrigger_keyboard(const SDLKey key
			, const SDLMod modifier
			, const Uint16 unicode)
		: key_(key)
		, modifier_(modifier)
		, unicode_(unicode)
	{
	}

	void operator()(tsignal_keyboard_function functor
			, tdispatcher& dispatcher
			, const tevent event
			, bool& handled
			, bool& halt)
	{
		functor(dispatcher, event, handled, halt, key_, modifier_, unicode_);
	}

private:
	SDLKey key_;
	SDLMod modifier_;
	Uint16 unicode_;
};

bool tdispatcher::fire(const tevent event
		, twidget& target
		, const SDLKey key
		, const SDLMod modifier
		, const Uint16 unicode)
{
	return fire_event<tsignal_keyboard_function>(event
			, dynamic_cast<twidget*>(this)
			, &target
			, ttrigger_keyboard(key, modifier, unicode));
}

bool tdispatcher::fire(const tevent event
		, twidget& target
		, void*)
{

	/**
	 * @todo The firing needs some polishing.
	 *
	 * Make sure the events can't be added to pre and post chain since they are
	 * not used.
	 */

	// Fire it here directly since we need special handling.
	bool handled = false;
	bool halt = false;

	if(target.has_event(event, child)) {

		tsignal<tsignal_notification_function>& signal =
				target.signal_notification_queue_.queue[event];

		for(std::vector<tsignal_notification_function>::iterator
					itor = signal.child.begin();
				itor != signal.child.end();
				++itor) {

			(*itor)(*this, event, handled, halt, NULL);

			if(halt) {
				assert(handled);
				break;
			}
		}
	}
	return handled;
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
 * @section handling_solution The implementation solutions.
 *
 * For the event handling we use a few use case scenarios and show the possible
 * solutions.
 *
 * @subsection sample The sample window
 *
 * In our samples we use this sample window with the following components; a
 * window W, a container C and a button B. These are arranged accordingly.
 *
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
 * * An event that is wanted by none.
 * * A mouse down event that should focus C and set the pressed state in B.
 * * A mouse wheel event, which first should be offered to B and if not handled
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
 * * W pre child
 * * C pre child
 * * B pre child
 * * W child
 * * C child
 * * B child
 * * W post child
 * * C post child
 * * B post child
 *
 * @subsubsection mouse_down Mouse down
 *
 * * W pre child
 * * C pre child -> set focus -> !handled
 * * B pre child -> set pressed state -> handled
 *
 * @subsubsection mouse_wheel Mouse wheel
 *
 * * W pre child
 * * C pre child
 * * B pre child -> We can't scroll so ignore
 * * W child
 * * C child
 * * B child
 * * W post child
 * * C post child -> Scroll -> handled
 *
 * @subsection chain Pass the events in a chain like fashion
 *
 * In this solution the events are send to the pre- and post queue of all but
 * the last possible widget and to the child of the last widget. The pre queue
 * will be send from top to bottom, the post queue from bottom to top.
 *
 * @subsubsection unhandled Unhandled event
 *
 * * W pre child
 * * C pre child
 * * B child
 * * C post child
 * * W post child
 *
 * @subsubsection mouse_down Mouse down
 *
 * * W pre child
 * * C pre child -> set focus -> !handled
 * * B child -> set pressed state -> handled
 *
 * @subsubsection mouse_wheel Mouse wheel
 *
 * * W pre child
 * * C pre child
 * * B child -> We can't scroll so ignore
 * * W post child
 * * C post child -> Scroll -> handled
 *
 * @section evaluation Evaluation
 *
 * When using the first solution it's possible to drop the child queue since
 * everything falls in pre or post. But there is a scenario that's a bit ugly
 * to solve with the first solution:
 *
 * Assume there is a listbox with toggle panels and on the panel there are a
 * few buttons, the wanted behaviour is:
 * * if clicked on the panel it should toggle, which may or may not be allowed.
 * * if clicked on a button in the panel, we want to make sure the panel is
 *   selected, which again may or may not be allowed.
 *
 * With solution 2 it's rather easy:
 *
 * Click on panel:
 * * W pre child
 * * C child -> Test whether we can toggle -> handled, halt = !toggled
 *
 * * W pre child
 * * C pre child -> Test whether we can select -> handled = halt = !selected
 * * B child -> do button stuff -> handled
 *
 * Since for the different clicks, different queues are triggered it's easy to
 * add a different handler there.
 *
 * With solution 1:
 *
 * Click on panel:
 * * W pre child
 * * C pre child -> handler 1 -> if last in queue -> solution 2 C child
 * * C pre child -> handler 2 -> if !last in queue -> solution 2 C pre child
 * * B pre child -> do button stuff -> handled
 *
 * Not that different from solution 2, the two handlers are installed in the C
 * pre event. But we need to manually check whether we're really the last,
 * which means the code to check whether there are more handlers at a lower
 * level is needed for both solutions. In solution 1 this test needs to be done
 * twice versus once in solution 2. Also the fact that the queues for the
 * events are processed in reverse order on the way back sounds more
 * initiative.
 */
