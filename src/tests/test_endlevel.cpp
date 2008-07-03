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

#include <boost/test/auto_unit_test.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <queue>
#include "SDL.h"

#include "key.hpp"
#include "events.hpp"
//#include "dialogs.hpp"


// Linker workarounds start here
#define LD_WA

#ifdef LD_WA
#include "config.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "filesystem.hpp"
#include "sdl_utils.hpp"
#include "game_events.hpp"
#include "network.hpp"
// to load libwesnoth_extras
WML_HANDLER_FUNCTION(test_sources, , , )
{
	// To load libwesnoth_core
	network::get_pending_stats();
	// To load libwesnoth_sdl
	SDL_Color color = int_to_color(255);
	std::cerr << "Fooled you\n";
}
#endif
// Linker workarounds end here

/**
 * fake_sdl_event_source is used to generate new events in
 * events::pump()
 **/
namespace test {

//	bool event_fired = false;

	template<class T>
		struct test_less_ptr {
			bool operator()(const T& a, const T& b) const
			{
				return (*a) < (*b);
			}
		};

	class fake_sdl_event_source 
		: public events::pump_monitor,
		public boost::noncopyable
	{
		public:

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
			event_node(const size_t time, const SDL_Event& event) : time_(time), fired_(false), event_(event)
			{}
			virtual ~event_node()
			{ }

			virtual void fire_event()
			{
				const int number_of_events = 1;
				const Uint32 mask = 0;
				SDL_PeepEvents(&event_, number_of_events, SDL_ADDEVENT, mask);
				fired_ = true;
			}

			/**
			 * @return true if this should stop firing events
			 **/
			bool test_if_should_fire(const size_t time_now) const
			{
			   	return time_ <= time_now;
		   	}

			bool is_fired() const
			{
				return fired_;
			}

			/**
			 * We want the smallestat the top
			 **/
			bool operator<(const event_node& o) const
			{ return time_ > o.time_; }
		};

		class event_node_keyboard : public event_node {
			public:
			event_node_keyboard(size_t time, SDL_Event& event) : event_node(time,event)
			{}
			void fire_event()
			{
				event_node::fire_event();
				static int num_keys = 300;
				Uint8* key_list = SDL_GetKeyState( &num_keys );
				if (event_.type == SDL_KEYDOWN)
					key_list[SDLK_ESCAPE] = 5;
				else
					key_list[SDLK_ESCAPE] = 0;
			}
		};

		typedef boost::shared_ptr<event_node> event_node_ptr;

		private:
		typedef std::priority_queue<event_node_ptr,std::vector<event_node_ptr>,test_less_ptr<event_node_ptr> > event_queue;

		event_queue queue_;

		public:
		void add_event(const size_t time, const SDL_Event& event)
		{
			event_node_ptr new_node(new event_node(time,event));
			queue_.push(new_node);
		}
		
		void add_event(event_node_ptr new_node)
		{
			queue_.push(new_node);
		}

		void process(events::pump_info& /*info*/);
	};

	void fake_sdl_event_source::process(events::pump_info& /*info*/)
	{
		if (queue_.empty())
			return;
		size_t now = SDL_GetTicks();
		if (queue_.top()->test_if_should_fire(now))
		{
			queue_.top()->fire_event();
			queue_.pop();
		}
	}


	BOOST_AUTO_TEST_SUITE( endlevel )

		BOOST_AUTO_TEST_CASE( test_fake_input )
		{
#ifdef LD_WA
			{
				config cfg;
				scoped_istream stream = preprocess_file("data/hardwired/language.cfg");
				read(cfg, *stream);
			}
#endif

			fake_sdl_event_source source;
			SDL_Event event;

			event.type = SDL_KEYDOWN;
			event.key.state = SDL_PRESSED;
			event.key.keysym.sym = SDLK_ESCAPE;
			event.key.keysym.scancode = 65;
			event.key.keysym.mod = KMOD_NONE;
			event.key.keysym.unicode = 0;
			const size_t now = SDL_GetTicks();
		
			fake_sdl_event_source::event_node_ptr new_keypress(new fake_sdl_event_source::event_node_keyboard(now + 10, event));
			event.type = SDL_KEYUP;
			event.key.state = SDL_RELEASED;
			fake_sdl_event_source::event_node_ptr new_keyrelease(new fake_sdl_event_source::event_node_keyboard(now + 20, event));
			source.add_event(new_keypress);
			source.add_event(new_keyrelease);
			

			BOOST_MESSAGE("Starting endlevel test!");
			CKey key;
			//	dialogs::get_save_name();
			while(true)
			{
				events::pump();
				
				BOOST_CHECK_EQUAL(key[SDLK_ESCAPE], new_keypress->is_fired());
				if (key[SDLK_ESCAPE])
					break;
			}	
			while(true)
			{	
				events::pump();
				BOOST_CHECK_EQUAL(key[SDLK_ESCAPE], !new_keyrelease->is_fired());
				if (!key[SDLK_ESCAPE])
					break;
			}
		}

	BOOST_AUTO_TEST_SUITE_END()
}
