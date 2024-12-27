/*
	Copyright (C) 2009 - 2024
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

#include "gui/core/event/dispatcher.hpp"

#include "gui/widgets/widget.hpp"
#include "utils/ranges.hpp"

#include <SDL2/SDL_events.h>

#include <cassert>

namespace gui2::event
{
struct dispatcher_implementation
{
	/**
	 * Returns the appropriate signal queue for an event by category.
	 *
	 * @tparam C                  For example, general.
	 * @param dispatcher          The dispatcher whose signal queue is used.
	 * @param event               The event to get the signal for.
	 *
	 * @returns                   The signal of the type dispatcher::signal_type<T>
	 */
	template<event_category C>
	static auto& event_signal(dispatcher& dispatcher, const ui_event event)
	{
		return dispatcher.get_signal_queue<C>().queue[event];
	}

	/**
	 * A helper to test whether dispatcher has an handler for a certain event.
	 *
	 * @param dispatcher      The dispatcher whose signal queue is used.
	 * @param queue_type      The type of event to look for.
	 * @param event           The event to get the signal for.
	 *
	 * @returns               Whether or not the handler is found.
	 */
	static bool has_handler(dispatcher& dispatcher, const dispatcher::event_queue_type queue_type, ui_event event)
	{
		const auto queue_check = [&](auto& queue_set) {
			return !queue_set.queue[event].empty(queue_type);
		};

		// We can't just use get_signal_queue since there's no way to know the event at compile time.
		switch(get_event_category(event)) {
		case event_category::general:
			return queue_check(dispatcher.signal_queue_);
		case event_category::mouse:
			return queue_check(dispatcher.signal_mouse_queue_);
		case event_category::keyboard:
			return queue_check(dispatcher.signal_keyboard_queue_);
		case event_category::touch_motion:
			return queue_check(dispatcher.signal_touch_motion_queue_);
		case event_category::touch_gesture:
			return queue_check(dispatcher.signal_touch_gesture_queue_);
		case event_category::notification:
			return queue_check(dispatcher.signal_notification_queue_);;
		case event_category::message:
			return queue_check(dispatcher.signal_message_queue_);
		case event_category::raw_event:
			return queue_check(dispatcher.signal_raw_event_queue_);
		case event_category::text_input:
			return queue_check(dispatcher.signal_text_input_queue_);
		default:
			throw std::invalid_argument("Event is not categorized");
		}
	}
};

namespace implementation
{

/*
 * Small sample to illustrate the effects of the various build_event_chain
 * functions. Assume the widgets are in an window with the following widgets:
 *
 *  -----------------------
 *  | dispatcher          |
 *  | ------------------- |
 *  | | container 1     | |
 *  | | --------------- | |
 *  | | | container 2 | | |
 *  | | | ----------- | | |
 *  | | | | widget  | | | |
 *  | | | ----------- | | |
 *  | | --------------- | |
 *  | ------------------- |
 *  -----------------------
 *
 * Note that the firing routine fires the events from:
 * - pre child for chain.end() - > chain.begin()
 * - child for widget
 * - post child for chain.begin() -> chain.end()
 */

/**
 * Build the event chain.
 *
 * The event chain is a chain of events starting from the first parent of the
 * widget until (and including) the wanted parent. For all these widgets it
 * will be tested whether they have either a pre or post handler for the event.
 * This ways there will be list of widgets to try to send the events to.
 * If there's no line from widget to parent the result is undefined.
 * (If widget == dispatcher the result will always be empty.)
 *
 * @pre                           dispatcher != nullptr
 * @pre                           widget != nullptr
 *
 * @param event                   The event to test.
 * @param dispatcher              The final widget to test, this is also the
 *                                dispatcher the sends the event.
 * @param w                       The widget should parent(s) to check.
 *
 * @returns                       The list of widgets with a handler.
 *                                The order will be (assuming all have a
 *                                handler):
 *                                * container 2
 *                                * container 1
 *                                * dispatcher
 */
template<event_category C>
std::vector<std::pair<widget*, ui_event>>
build_event_chain(const ui_event event, widget* dispatcher, widget* w)
{
	assert(dispatcher);
	assert(w);

	std::vector<std::pair<widget*, ui_event>> result;

	while(true) {
		if(w->has_event(event, dispatcher::event_queue_type(dispatcher::pre | dispatcher::post))) {
			result.emplace_back(w, event);
		}

		if(w == dispatcher) {
			break;
		}

		w = w->parent();
		assert(w);
	}

	return result;
}

/**
 * Build the event chain for signal_notification.
 *
 * The notification is only send to the receiver it returns an empty chain.
 * Since the pre and post queues are unused, it validates whether they are
 * empty (using asserts).
 *
 * @returns                       An empty vector.
 */
template<>
std::vector<std::pair<widget*, ui_event>>
build_event_chain<event_category::notification>(const ui_event event, widget* dispatcher, widget* w)
{
	assert(dispatcher);
	assert(w);

	assert(!w->has_event(event, dispatcher::event_queue_type(dispatcher::pre | dispatcher::post)));

	return {};
}

/**
 * Build the event chain for signal_message.
 *
 * This function expects that the widget sending it is also the receiver. This
 * assumption might change, but is valid for now. The function doesn't build an
 * event chain from @p dispatcher to @p widget but from @p widget to its
 * toplevel item (the first one without a parent) which we call @p window.
 *
 * @pre                           dispatcher == widget
 *
 * @returns                       The list of widgets with a handler.
 *                                The order will be (assuming all have a
 *                                handler):
 *                                * window
 *                                * container 1
 *                                * container 2
 */
template<>
std::vector<std::pair<widget*, ui_event>>
build_event_chain<event_category::message>(const ui_event event, widget* dispatcher, widget* w)
{
	assert(dispatcher);
	assert(w);
	assert(w == dispatcher);

	std::vector<std::pair<widget*, ui_event>> result;

	/* We only should add the parents of the widget to the chain. */
	while((w = w->parent())) {
		assert(w);

		if(w->has_event(event, dispatcher::event_queue_type(dispatcher::pre | dispatcher::post))) {
			result.emplace(result.begin(), w, event);
		}
	}

	return result;
}

/**
 * Helper function for fire_event.
 *
 * This is called with the same parameters as fire_event except for the
 * event_chain, which contains the widgets with the events to call for them.
 */
template<event_category C, typename... F>
bool fire_event(const ui_event event,
	const std::vector<std::pair<widget*, ui_event>>& event_chain,
	widget* dispatcher,
	widget* w,
	F&&... params)
{
	bool handled = false;
	bool halt = false;

	/***** ***** ***** Pre ***** ***** *****/
	for(const auto& [chain_target, chain_event] : event_chain | utils::views::reverse) {
		const auto& signal = dispatcher_implementation::event_signal<C>(*chain_target, chain_event);

		for(const auto& pre_func : signal.pre_child) {
			pre_func(*dispatcher, chain_event, handled, halt, std::forward<F>(params)...);

			if(halt) {
				assert(handled);
				break;
			}
		}

		if(handled) {
			return true;
		}
	}

	/***** ***** ***** Child ***** ***** *****/
	if(w->has_event(event, dispatcher::child)) {
		const auto& signal = dispatcher_implementation::event_signal<C>(*w, event);

		for(const auto& func : signal.child) {
			func(*dispatcher, event, handled, halt, std::forward<F>(params)...);

			if(halt) {
				assert(handled);
				break;
			}
		}

		if(handled) {
			return true;
		}
	}

	/***** ***** ***** Post ***** ***** *****/
	for(const auto& [chain_target, chain_event] : event_chain) {
		const auto& signal = dispatcher_implementation::event_signal<C>(*chain_target, chain_event);

		for(const auto& post_func : signal.post_child) {
			post_func(*dispatcher, chain_event, handled, halt, std::forward<F>(params)...);

			if(halt) {
				assert(handled);
				break;
			}
		}

		if(handled) {
			return true;
		}
	}

	/**** ***** ***** Unhandled ***** ***** *****/
	assert(handled == false);
	return false;
}

} // namespace implementation

/**
 * Fires an event.
 *
 * A helper to allow the common event firing code to be shared between the
 * different signal function types.
 *
 * @pre                           d != nullptr
 * @pre                           w != nullptr
 *
 * @tparam C                      The category of the event to handle.
 * @tparam F                      The parameter pack type.
 *
 *
 * @param event                   The event to fire.
 * @param d                       The dispatcher that handles the event.
 * @param w                       The widget that should receive the event.
 * @param params                  Zero or more additional arguments to pass
 *                                to the signal function when it's executed.
 *
 * @returns                       Whether or not the event was handled.
 */
template<event_category C, typename... F>
bool fire_event(const ui_event event, dispatcher* d, widget* w, F&&... params)
{
	assert(d);
	assert(w);

	widget* dispatcher_w = dynamic_cast<widget*>(d);

	std::vector<std::pair<widget*, ui_event>> event_chain =
		implementation::build_event_chain<C>(event, dispatcher_w, w);

	return implementation::fire_event<C>(event, event_chain, dispatcher_w, w, std::forward<F>(params)...);
}

template<ui_event click,
	ui_event double_click,
	bool (event_executor::*wants_double_click)() const,
	typename... F>
bool fire_event_double_click(dispatcher* dsp, widget* wgt, F&&... params)
{
	assert(dsp);
	assert(wgt);

	std::vector<std::pair<widget*, ui_event>> event_chain;
	widget* w = wgt;
	widget* d = dynamic_cast<widget*>(dsp);

	while(w != d) {
		w = w->parent();
		assert(w);

		if(std::invoke(wants_double_click, w)) {
			if(w->has_event(double_click, dispatcher::event_queue_type(dispatcher::pre | dispatcher::post))) {
				event_chain.emplace_back(w, double_click);
			}
		} else {
			if(w->has_event(click, dispatcher::event_queue_type(dispatcher::pre | dispatcher::post))) {
				event_chain.emplace_back(w, click);
			}
		}
	}

	if(std::invoke(wants_double_click, wgt)) {
		constexpr auto C = get_event_category(double_click);
		return implementation::fire_event<C>(double_click, event_chain, d, wgt, std::forward<F>(params)...);
	} else {
		constexpr auto C = get_event_category(click);
		return implementation::fire_event<C>(click, event_chain, d, wgt, std::forward<F>(params)...);
	}
}

} // namespace gui2::event
