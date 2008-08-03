/* $Id$ */
/*
   Copyright (C) 2008 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef TESTS_UTILS_FAKE_EVENT_SOURCE_HPP_INCLUDED
#define TESTS_UTILS_FAKE_EVENT_SOURCE_HPP_INCLUDED

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <queue>
#include "SDL.h"

#include "events.hpp"

namespace test_utils {


	template<class T>
		struct less_ptr {
			bool operator()(const T& a, const T& b) const
			{
				return (*a) < (*b);
			}
		};

	/**
	 * Base class for all event nodes to be used to fire fake input events
	 **/
	class event_node :
		public boost::noncopyable
	{
		size_t time_;
		bool fired_;
		protected:
		SDL_Event event_;

		public:
		event_node(const size_t time, const SDL_Event& event);
		virtual ~event_node();

		virtual void fire_event();
		/**
		 * @return true if this should stop firing events
		 **/
		bool test_if_should_fire(const size_t frame_count ) const;

		bool is_fired() const;

		/**
		 * We want the smallestat the top
		 **/
		bool operator<(const event_node& o) const;
	};

	class event_node_keyboard : public event_node {
		public:
			event_node_keyboard(size_t time, SDL_Event& event);
			virtual void fire_event();
	};

	class event_node_mouse_motion : public event_node {
		public:
			event_node_mouse_motion(size_t time, SDL_Event& event);
			virtual void fire_event();
	};

	typedef boost::shared_ptr<event_node> event_node_ptr;

	/**
	 * fake_event_source is used to generate new events in
	 * events::pump()
	 * Timing used in function is loop counter incremented
	 * everytime events::pump() is called.
	 **/

	class fake_event_source 
		: public events::pump_monitor,
		public boost::noncopyable
	{
			size_t frame_count_;
			typedef std::priority_queue<event_node_ptr,std::vector<event_node_ptr>,less_ptr<event_node_ptr> > event_queue;

			event_queue queue_;

			SDL_Event make_key_event(Uint8 type, const SDLKey key, const SDLMod mod);
		public:
			fake_event_source();
			~fake_event_source();
			void add_event(const size_t time, const SDL_Event& event);
			void add_event(event_node_ptr new_node);
			void start();

			event_node_ptr press_key(const size_t time, const SDLKey key, const SDLMod mod = KMOD_NONE);
			event_node_ptr release_key(const size_t time, const SDLKey key, const SDLMod mod =KMOD_NONE);

			event_node_ptr move_mouse(const size_t time, const int x, const int y);
			event_node_ptr mouse_press(const size_t time, const Uint8 button);
			event_node_ptr mouse_release(const size_t time, const Uint8 button);

			void process(events::pump_info& /*info*/);
	};
}
#endif
