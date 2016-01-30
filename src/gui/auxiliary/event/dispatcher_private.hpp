/*
   Copyright (C) 2009 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_AUXILIARY_EVENT_DISPATCHER_PRIVATE_HPP_INCLUDED
#define GUI_WIDGETS_AUXILIARY_EVENT_DISPATCHER_PRIVATE_HPP_INCLUDED

#include "gui/auxiliary/event/dispatcher.hpp"

#include "gui/widgets/widget.hpp"

#include "SDL_events.h"

#include <boost/mpl/for_each.hpp>

namespace gui2
{

namespace event
{

struct tdispatcher_implementation
{
/**
 * Helper macro to implement the various event_signal functions.
 *
 * Implements two helper functions as documented in the macro.
 *
 * @param SET                     The set in which the event type needs to be
 *                                eg the @ref gui2::event::tset_event or a
 *                                similar set defined in that header.
 * @param FUNCTION                The function signature to validate the
 *                                implementation function SFINAE against eg the
 *                                @ref gui2::event::tsignal_function or another
 *                                one in that header.
 * @param QUEUE                   The queue in which the @p event is slotted.
 */
#define IMPLEMENT_EVENT_SIGNAL(SET, FUNCTION, QUEUE)                           \
	/**                                                                        \
	 * Returns the signal structure for a FUNCTION.                            \
	 *                                                                         \
	 * There are several functions that only overload the return value, in     \
	 * order to do so they use SFINAE.                                         \
	 *                                                                         \
	 * @tparam F                  tsignal_function.                            \
	 * @param dispatcher          The dispatcher whose signal queue is used.   \
	 * @param event               The event to get the signal for.             \
	 *                                                                         \
	 * @returns                   The signal of the type                       \
	 *                            tdispatcher::tsignal<FUNCTION>               \
	 */                                                                        \
	template <class F>                                                         \
	static typename boost::enable_if<boost::is_same<F, FUNCTION>,              \
									 tdispatcher::tsignal<FUNCTION> >::type&   \
	event_signal(tdispatcher& dispatcher, const tevent event)                  \
	{                                                                          \
		return dispatcher.QUEUE.queue[event];                                  \
	}                                                                          \
                                                                               \
	/**                                                                        \
	 * Returns the signal structure for a key in SET.                          \
	 *                                                                         \
	 * There are several functions that only overload the return value, in     \
	 * order to do so they use SFINAE.                                         \
	 *                                                                         \
	 * @tparam K                  A key in tset_event.                         \
	 * @param dispatcher          The dispatcher whose signal queue is used.   \
	 * @param event               The event to get the signal for.             \
	 *                                                                         \
	 * @returns                   The signal of the type                       \
	 *                            tdispatcher::tsignal<FUNCTION>               \
	 */                                                                        \
	template <class K>                                                         \
	static typename boost::enable_if<boost::mpl::has_key<SET, K>,              \
									 tdispatcher::tsignal<FUNCTION> >::type&   \
	event_signal(tdispatcher& dispatcher, const tevent event)                  \
	{                                                                          \
		return dispatcher.QUEUE.queue[event];                                  \
	}


	IMPLEMENT_EVENT_SIGNAL(tset_event, tsignal_function, signal_queue_)

/**
 * Small helper macro to wrap @ref IMPLEMENT_EVENT_SIGNAL.
 *
 * Since the parameters to @ref IMPLEMENT_EVENT_SIGNAL use the same parameters
 * with a slight difference per type this macro wraps the function by its type.
 *
 * @param TYPE                    The type to wrap for @ref
 *                                IMPLEMENT_EVENT_SIGNAL.
 */
#define IMPLEMENT_EVENT_SIGNAL_WRAPPER(TYPE)                                   \
	IMPLEMENT_EVENT_SIGNAL(tset_event_##TYPE,                                  \
						   tsignal_##TYPE##_function,                          \
						   signal_##TYPE##_queue_)

	IMPLEMENT_EVENT_SIGNAL_WRAPPER(mouse)
	IMPLEMENT_EVENT_SIGNAL_WRAPPER(keyboard)
	IMPLEMENT_EVENT_SIGNAL_WRAPPER(notification)
	IMPLEMENT_EVENT_SIGNAL_WRAPPER(message)

#undef IMPLEMENT_EVENT_SIGNAL_WRAPPER
#undef IMPLEMENT_EVENT_SIGNAL

	/**
	 * A helper class to find out whether dispatcher has an handler for a
	 * certain event.
	 */
	class thas_handler
	{
	public:
		/**
		 * Constructor.
		 *
		 * @param event_type      The type of event to look for.
		* @param dispatcher      The dispatcher whose signal queue is used.
		 */
		thas_handler(const tdispatcher::tevent_type event_type,
					 tdispatcher& dispatcher)
			: event_type_(event_type), dispatcher_(dispatcher)
		{
		}

		/**
		 * Tests whether a handler for an event is available.
		 *
		 * It tests for both the event and the event_type send in the
		 * constructor.
		 *
		 * @tparam T              A key from an event set used to instantiate
		 *                        the proper @p event_signal function.
		* @param event           The event to get the signal for.
		 *
		 * @returns               Whether or not the handler is found.
		 */
		// not called operator() to work around a problem in MSVC 2008.
		template <class T>
		bool oper(tevent event)
		{
			if((event_type_ & tdispatcher::pre)
			   && !event_signal<T>(dispatcher_, event).pre_child.empty()) {
				return true;
			}
			if((event_type_ & tdispatcher::child)
			   && !event_signal<T>(dispatcher_, event).child.empty()) {
				return true;
			}
			if((event_type_ & tdispatcher::post)
			   && !event_signal<T>(dispatcher_, event).post_child.empty()) {
				return true;
			}
			return false;
		}

	private:
		tdispatcher::tevent_type event_type_;
		tdispatcher& dispatcher_;
	};
};

/** Contains the implementation details of the find function. */
namespace implementation
{

/** Specialized class when itor == end */
template <bool done = true>
struct find
{
	template <typename itor, typename end, typename E, typename F>
	static bool execute(itor*, end*, E, F)
	{
		return false;
	}
};

/** Specialized class when itor != end */
template <>
struct find<false>
{
	template <typename itor, typename end, typename E, typename F>
	static bool execute(itor*, end*, E event, F functor)
	{
		typedef typename boost::mpl::deref<itor>::type item;
		typedef typename boost::mpl::apply1<boost::mpl::identity<>, item>::type
		arg;

		boost::value_initialized<arg> x;

		if(boost::get(x) == event) {
			// MSVC 2008 doesn't like operator() here so changed the name.
			return functor.template oper<item>(event);
		} else {
			typedef typename boost::mpl::next<itor>::type titor;
			return find<boost::is_same<titor, end>::value>::execute(
					static_cast<titor*>(NULL),
					static_cast<end*>(NULL),
					event,
					functor);
		}
	}
};

} // namespace implementation

/**
 * Tests whether an event handler is available.
 *
 * The code is based on boost::mpl_for_each, which doesn't allow to call a
 * template function with the dereferred iterator as template parameter.
 *
 * The function first tries to match whether the value in the sequence matches
 * event, once that matched it will execute the functor with the key found as
 * template parameter and the event as parameter.
 *
 * @tparam sequence               The sequence to test upon.
 * @tparam E                      The value type of the item in the sequence
 * @tparam F                      Type of the functor.
 *
 * @param event                   The event to look for.
 * @param functor                 The predicate which should is executed if the
 *                                event is matched.
 *
 * @returns                       Whether or not the function found a result.
 */
template <typename sequence, typename E, typename F>
inline bool find(E event, F functor)
{
	typedef typename boost::mpl::begin<sequence>::type begin;
	typedef typename boost::mpl::end<sequence>::type end;

	return implementation::find<boost::is_same<begin, end>::value>::execute(
			static_cast<begin*>(NULL), static_cast<end*>(NULL), event, functor);
}

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
 * @pre                           dispatcher != NULL
 * @pre                           widget != NULL
 *
 * @param event                   The event to test.
 * @param dispatcher              The final widget to test, this is also the
 *                                dispatcher the sends the event.
 * @param widget                  The widget should parent(s) to check.
 *
 * @returns                       The list of widgets with a handler.
 *                                The order will be (assuming all have a
 *                                handler):
 *                                * container 2
 *                                * container 1
 *                                * dispatcher
 */
template <class T>
inline std::vector<std::pair<twidget*, tevent> >
build_event_chain(const tevent event, twidget* dispatcher, twidget* widget)
{
	assert(dispatcher);
	assert(widget);

	std::vector<std::pair<twidget*, tevent> > result;

	while(widget != dispatcher) {
		widget = widget->parent();
		assert(widget);

		if(widget->has_event(event,
							 tdispatcher::tevent_type(tdispatcher::pre
													  | tdispatcher::post))) {

			result.push_back(std::make_pair(widget, event));
		}
	}

	return result;
}

/**
 * Build the event chain for tsignal_notification_function.
 *
 * The notification is only send to the receiver it returns an empty chain.
 * Since the pre and post queues are unused, it validates whether they are
 * empty (using asserts).
 *
 * @returns                       An empty vector.
 */
template <>
inline std::vector<std::pair<twidget*, tevent> >
build_event_chain<tsignal_notification_function>(const tevent event,
												 twidget* dispatcher,
												 twidget* widget)
{
	assert(dispatcher);
	assert(widget);

	assert(!widget->has_event(event,
							  tdispatcher::tevent_type(tdispatcher::pre
													   | tdispatcher::post)));

	return std::vector<std::pair<twidget*, tevent> >();
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4706)
#endif
/**
 * Build the event chain for tsignal_message_function.
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
template <>
inline std::vector<std::pair<twidget*, tevent> >
build_event_chain<tsignal_message_function>(const tevent event,
											twidget* dispatcher,
											twidget* widget)
{
	assert(dispatcher);
	assert(widget);
	assert(widget == dispatcher);

	std::vector<std::pair<twidget*, tevent> > result;

	/* We only should add the parents of the widget to the chain. */
	while((widget = widget->parent())) {
		assert(widget);

		if(widget->has_event(event,
							 tdispatcher::tevent_type(tdispatcher::pre
													  | tdispatcher::post))) {

			result.insert(result.begin(), std::make_pair(widget, event));
		}
	}

	return result;
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

/**
 * Helper function for fire_event.
 *
 * This is called with the same parameters as fire_event except for the
 * event_chain, which contains the widgets with the events to call for them.
 */
template <class T, class F>
inline bool fire_event(const tevent event,
					   std::vector<std::pair<twidget*, tevent> >& event_chain,
					   twidget* dispatcher,
					   twidget* widget,
					   F functor)
{
	bool handled = false;
	bool halt = false;

	/***** ***** ***** Pre ***** ***** *****/
	for(std::vector<std::pair<twidget*, tevent> >::reverse_iterator ritor_widget
		= event_chain.rbegin();
		ritor_widget != event_chain.rend();
		++ritor_widget) {

		tdispatcher::tsignal<T>& signal
				= tdispatcher_implementation::event_signal<T>(
						*ritor_widget->first, ritor_widget->second);

		for(typename std::vector<T>::iterator itor = signal.pre_child.begin();
			itor != signal.pre_child.end();
			++itor) {

			functor(*itor, *dispatcher, ritor_widget->second, handled, halt);
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
	if(widget->has_event(event, tdispatcher::child)) {

		tdispatcher::tsignal<T>& signal
				= tdispatcher_implementation::event_signal<T>(*widget, event);

		for(typename std::vector<T>::iterator itor = signal.child.begin();
			itor != signal.child.end();
			++itor) {

			functor(*itor, *dispatcher, event, handled, halt);

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
	for(std::vector<std::pair<twidget*, tevent> >::iterator itor_widget
		= event_chain.begin();
		itor_widget != event_chain.end();
		++itor_widget) {

		tdispatcher::tsignal<T>& signal
				= tdispatcher_implementation::event_signal<T>(
						*itor_widget->first, itor_widget->second);

		for(typename std::vector<T>::iterator itor = signal.post_child.begin();
			itor != signal.post_child.end();
			++itor) {

			functor(*itor, *dispatcher, itor_widget->second, handled, halt);
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
 * @pre                           dispatcher != NULL
 * @pre                           widget != NULL
 *
 * @tparam T                      The signal type of the event to handle.
 * @tparam F                      The type of the functor.
 *
 *
 * @param event                   The event to fire.
 * @param dispatcher              The dispatcher that handles the event.
 * @param widget                  The widget that should receive the event.
 * @param functor                 The functor to execute the actual event.
 *                                Since some functions need different
 *                                parameters this functor stores them before
 *                                firing the event.
 *
 * @returns                       Whether or not the event was handled.
 */
template <class T, class F>
inline bool
fire_event(const tevent event, twidget* dispatcher, twidget* widget, F functor)
{
	assert(dispatcher);
	assert(widget);

	std::vector<std::pair<twidget*, tevent> > event_chain
			= implementation::build_event_chain<T>(event, dispatcher, widget);

	return implementation::fire_event<T>(
			event, event_chain, dispatcher, widget, functor);
}

template <tevent click,
		  tevent double_click,
		  bool (tevent_executor::*wants_double_click)() const,
		  class T,
		  class F>
inline bool
fire_event_double_click(twidget* dispatcher, twidget* widget, F functor)
{
	assert(dispatcher);
	assert(widget);

	std::vector<std::pair<twidget*, tevent> > event_chain;
	twidget* w = widget;
	while(w != dispatcher) {
		w = w->parent();
		assert(w);

		if((w->*wants_double_click)()) {

			if(w->has_event(double_click,
							tdispatcher::tevent_type(tdispatcher::pre
													 | tdispatcher::post))) {

				event_chain.push_back(std::make_pair(w, double_click));
			}
		} else {
			if(w->has_event(click,
							tdispatcher::tevent_type(tdispatcher::pre
													 | tdispatcher::post))) {

				event_chain.push_back(std::make_pair(w, click));
			}
		}
	}

	if((widget->*wants_double_click)()) {
		return implementation::fire_event<T>(
				double_click, event_chain, dispatcher, widget, functor);
	} else {
		return implementation::fire_event<T>(
				click, event_chain, dispatcher, widget, functor);
	}
}

} // namespace event

} // namespace gui2

#endif
