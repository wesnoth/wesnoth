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
	: signal_queue_()
	, connected_(false)
{
}

tdispatcher::~tdispatcher()
{
	if(connected_) {
		connect_dispatcher(this);
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

