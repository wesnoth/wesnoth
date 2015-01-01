/*
   Copyright (C) 2008 - 2015 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
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

		/**
		 * Used to fire sdl event stored in this object.
		 * Child class may extend or override functionality
		 **/
		virtual void fire_event();
		/**
		 * Test if this event should fire now
		 **/
		bool test_if_should_fire(const size_t frame_count ) const;

		/**
		 * @return true if this event has fired
		 **/
		bool is_fired() const;

		/**
		 * We want the smallestat the top
		 **/
		bool operator<(const event_node& o) const;
	};

	/**
	 * modifies SDL_GetKeyState table to have
	 * correct state.
	 **/
	class event_node_keyboard : public event_node {
		public:
			event_node_keyboard(size_t time, SDL_Event& event);
			virtual void fire_event();
	};

	/**
	 * Uses special SDL_WarpMouse function to
	 * generate mouse move events
	 **/
	class event_node_mouse_motion : public event_node {
		public:
			event_node_mouse_motion(size_t time, SDL_Event& event);
			virtual void fire_event();
	};

	/**
	 * Used to create SDL_MOUSEBUTTONDOWN/UP events
	 * with correct x,y values
	 **/
	class event_node_mouse_click : public event_node {
		public:
			event_node_mouse_click(size_t time, SDL_Event& event);
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
			SDL_Event make_mouse_click_event(const Uint8 type, const Uint8 button);
		public:
			fake_event_source();
			~fake_event_source();

			/**
			 * adds a generic event to queue
			 **/
			void add_event(const size_t time, const SDL_Event& event);
			/**
			 * adds any type of event to queue
			 **/
			void add_event(event_node_ptr new_node);

			/**
			 * Sets event time source back to zero
			 **/
			void start();

			/**
			 * adds keyboard press event to queue
			 **/
			event_node_ptr press_key(const size_t time, const SDLKey key, const SDLMod mod = KMOD_NONE);
			/**
			 * adds keyboard release event to queue
			 **/
			event_node_ptr release_key(const size_t time, const SDLKey key, const SDLMod mod =KMOD_NONE);
			/**
			 * Just push and release a key
			 * release is done in time+1
			 * @return release event only
			 **/
			event_node_ptr type_key(const size_t time, const SDLKey key, const SDLMod mod =KMOD_NONE);

			/**
			 * Adds mouse motion event to queue
			 **/
			event_node_ptr move_mouse(const size_t time, const int x, const int y);
			/**
			 * adds mouse button click event to queue
			 **/
			event_node_ptr mouse_press(const size_t time, const Uint8 button);
			/**
			 * adds mouse button realease event to queue
			 **/
			event_node_ptr mouse_release(const size_t time, const Uint8 button);
			/**
			 * Make mouse click that equals to press and release
			 * relase is done in time+1
			 * @return release event only
			 **/
			event_node_ptr mouse_click(const size_t time, const Uint8 button);

			/**
			 * Called by events::pump() to fire events
			 **/
			void process(events::pump_info& /*info*/);
	};
}
#endif
