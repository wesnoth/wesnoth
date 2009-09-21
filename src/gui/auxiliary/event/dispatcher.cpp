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

#include "gui/auxiliary/event/dispatcher.hpp"

#include "foreach.hpp"
#include "gui/auxiliary/log.hpp"

#include <string>

namespace gui2 {

namespace event {

/***** tdispatcher class. *****/

tdispatcher::tdispatcher()
	: mouse_behaviour_(all)
	, signal_queue_()
	, signal_mouse_queue_()
	, signal_keyboard_queue_()
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

template<class T>
static void dispatch_signal(const tevent event
		, tdispatcher::tsignal<T>& queue)
{
	std::vector<std::pair<
		  typename std::vector<T>::iterator
		, typename std::vector<T>::iterator> > queues;

	queues.push_back(
			std::make_pair(queue.pre_child.begin(), queue.pre_child.end()));
	queues.push_back(
			std::make_pair(queue.child.begin(), queue.child.end()));
	queues.push_back(
			std::make_pair(queue.post_child.begin(), queue.post_child.end()));

	bool handled = false;
	bool halt = false;

	for(typename
				std::vector<std::pair<
					  typename std::vector<T>::iterator
					, typename std::vector<T>::iterator> >::iterator
				queue_itor = queues.begin();
				queue_itor != queues.end();
				++queue_itor) {

		for(typename std::vector<T>::iterator itor = queue_itor->first;
				itor != queue_itor->second; ++itor) {

			(*itor)(event, handled, halt);
			if(halt) {
				assert(handled);
				break;
			}

		}

		if(handled) {
			return;
		}
	}
}

void tdispatcher::fire(const tevent event)
{
	dispatch_signal<tsignal_function>(event, signal_queue_.queue[event]);
}

// These templates look a lot alike one had extra parameters which are send
// to the functor, see how that can be "solved"
template<class T>
static void dispatch_signal(const tevent event
		, tdispatcher::tsignal<T>& queue
		, const tpoint& coordinate)
{
	std::vector<std::pair<
		  typename std::vector<T>::iterator
		, typename std::vector<T>::iterator> > queues;

	queues.push_back(
			std::make_pair(queue.pre_child.begin(), queue.pre_child.end()));
	queues.push_back(
			std::make_pair(queue.child.begin(), queue.child.end()));
	queues.push_back(
			std::make_pair(queue.post_child.begin(), queue.post_child.end()));

	bool handled = false;
	bool halt = false;

	for(typename
				std::vector<std::pair<
					  typename std::vector<T>::iterator
					, typename std::vector<T>::iterator> >::iterator
				queue_itor = queues.begin();
				queue_itor != queues.end();
				++queue_itor) {

		for(typename std::vector<T>::iterator itor = queue_itor->first;
				itor != queue_itor->second; ++itor) {

			(*itor)(event, handled, halt, coordinate);
			if(halt) {
				assert(handled);
				break;
			}

		}

		if(handled) {
			return;
		}
	}
}

void tdispatcher::fire(const tevent event, const tpoint& coordinate)
{
	dispatch_signal<tsignal_mouse_function>(event
			, signal_mouse_queue_.queue[event]
			, coordinate);
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
