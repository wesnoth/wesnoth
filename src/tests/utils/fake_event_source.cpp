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

#define GETTEXT_DOMAIN "wesnoth-test"

#include "tests/utils/fake_event_source.hpp"

#include "mouse_handler_base.hpp"

namespace test_utils {
	/**
	 * Base class for all event nodes to be used to fire fake input events
	 **/
	event_node::event_node(const size_t time, const SDL_Event& event) : time_(time), fired_(false), event_(event)
	{}

	event_node::~event_node()
	{ }

	void event_node::fire_event()
	{
		const int number_of_events = 1;
		const Uint32 mask = 0;
		SDL_PeepEvents(&event_, number_of_events, SDL_ADDEVENT, mask);
		fired_ = true;
	}

	/**
	 * @return true if this should stop firing events
	 **/
	bool event_node::test_if_should_fire(const size_t frame_count) const
	{
		return frame_count >= time_;
	}

	bool event_node::is_fired() const
	{
		return fired_;
	}

	/**
	 * We want the smallestat the top
	 **/
	bool event_node::operator<(const event_node& o) const
	{ return time_ > o.time_; }

	event_node_keyboard::event_node_keyboard(size_t time, SDL_Event& event) : event_node(time,event)
	{}
	void event_node_keyboard::fire_event()
	{
		event_node::fire_event();
		static int num_keys = 300;
		Uint8* key_list = SDL_GetKeyState( &num_keys );
		if (event_.type == SDL_KEYDOWN)
		{
			key_list[event_.key.keysym.sym] = 1;
		}
		else
			key_list[event_.key.keysym.sym] = 0;
	}

	event_node_mouse_motion::event_node_mouse_motion(size_t time, SDL_Event& event) : event_node(time,event)
	{}
	void event_node_mouse_motion::fire_event()
	{
		SDL_WarpMouse(event_.motion.x,event_.motion.y);
	}

	event_node_mouse_click::event_node_mouse_click(size_t time, SDL_Event& event) : event_node(time,event)
	{}
	void event_node_mouse_click::fire_event()
	{
		// We have to use temporaries because of difference
		// in types for mouse position.
		int x, y;
		SDL_GetMouseState(&x, &y);
		event_.button.x = static_cast<Uint16>(x);
		event_.button.y = static_cast<Uint16>(y);
		event_node::fire_event();
	}


	fake_event_source::fake_event_source()
		: frame_count_(0)
		, queue_()
	{
	}

	fake_event_source::~fake_event_source()
	{
		// send all still queued events
		// so keyboard/mouse state is restored
		while(!queue_.empty())
		{
			queue_.top()->fire_event();
			queue_.pop();
		}
		events::pump();
	}


	void fake_event_source::add_event(const size_t time, const SDL_Event& event)
	{
		event_node_ptr new_node(new event_node(time,event));
		queue_.push(new_node);
	}

	void fake_event_source::add_event(event_node_ptr new_node)
	{
		queue_.push(new_node);
	}

	void fake_event_source::start()
	{
		frame_count_ = 0;
	}

	SDL_Event fake_event_source::make_key_event(Uint8 type, const SDLKey key, const SDLMod mod)
	{
		SDL_Event event;
		event.type = type;
		if (type == SDL_KEYDOWN)
			event.key.state = SDL_PRESSED;
		else
			event.key.state = SDL_RELEASED;
		event.key.keysym.sym = key;
		event.key.keysym.scancode = static_cast<Uint8>(key); //
		event.key.keysym.mod = mod;
		event.key.keysym.unicode = static_cast<Uint16>(key); //
		return event;
	}

	event_node_ptr fake_event_source::move_mouse(const size_t time, const int x, const int y)
	{
		SDL_Event event;
		event.type = SDL_MOUSEMOTION;
		event.motion.x = static_cast<Uint16>(x);
		event.motion.y = static_cast<Uint16>(y);
		event_node_ptr new_move(new event_node_mouse_motion(time, event));
		add_event(new_move);
		return new_move;
	}

	SDL_Event fake_event_source::make_mouse_click_event(const Uint8 type, const Uint8 button)
	{
		SDL_Event event;
		event.type = type;
		if (type == SDL_MOUSEBUTTONDOWN)
			event.button.state = SDL_PRESSED;
		else
			event.button.state = SDL_RELEASED;
		event.button.button = button;
		return event;
	}

	event_node_ptr fake_event_source::mouse_press(const size_t time, const Uint8 button)
	{
		SDL_Event event = make_mouse_click_event(SDL_MOUSEBUTTONDOWN, button);
		event_node_ptr new_click(new event_node_mouse_click(time, event));
		add_event(new_click);
		return new_click;
	}

	event_node_ptr fake_event_source::mouse_release(const size_t time, const Uint8 button)
	{
		SDL_Event event = make_mouse_click_event(SDL_MOUSEBUTTONDOWN, button);
		event_node_ptr new_click(new event_node_mouse_click(time, event));
		add_event(new_click);
		return new_click;
	}

	event_node_ptr fake_event_source::mouse_click(const size_t time, const Uint8 button)
	{
		mouse_press(time, button);
		return mouse_release(time+1,button);
	}

	event_node_ptr fake_event_source::press_key(const size_t time, const SDLKey key, const SDLMod mod)
	{
		SDL_Event event = make_key_event(SDL_KEYDOWN, key, mod);
		event_node_ptr new_key(new event_node_keyboard(time, event));
		add_event(new_key);
		return new_key;
	}

	event_node_ptr fake_event_source::release_key(const size_t time, const SDLKey key, const SDLMod mod)
	{
		SDL_Event event = make_key_event(SDL_KEYUP, key, mod);
		event_node_ptr new_key(new event_node_keyboard(time, event));
		add_event(new_key);
		return new_key;
	}

	event_node_ptr fake_event_source::type_key(const size_t time, const SDLKey key, const SDLMod mod)
	{
		press_key(time,key,mod);
		return release_key(time+1,key,mod);
	}

	void fake_event_source::process(events::pump_info& /*info*/)
	{
		if (events::commands_disabled > 0)
			return;
		++frame_count_;
		if (queue_.empty())
			return;
		while (!queue_.empty()
				&& queue_.top()->test_if_should_fire(frame_count_))
		{
			queue_.top()->fire_event();
			queue_.pop();
		}
	}
}
