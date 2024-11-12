/*
	Copyright (C) 2003 - 2024
	by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
	Copyright (C) 2003 by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/**
 * @file
 * controller_base framework:
 * controller_base is roughly analogous to a "dialog" class in a GUI toolkit
 * which is appropriate for deriving wesnoth game modes, e.g. single player
 * mode, multiplayer mode, replay mode, editor mode.
 *
 * It provides implementation details for:
 * - play_slice, which is essentially one pass of the "main loop" of
 *   the application, pumping and dispatching SDL events, raising draw
 *   events, handling scrolling, sound sources, and some joystick issues
 *   It also handles displaying menus (Menu, Action).
 *
 * - showing context menus (much is delegated to command executor though)
 *
 * Other than this it functions as an abstract interface, enforcing that
 * controllers derive from events::sdl_handler, hotkey_command_executor,
 * and provide some accessors needed for event handling.
 */

#pragma once

#include "events.hpp"
#include "hotkey/hotkey_command.hpp"
#include "key.hpp"
#include "quit_confirmation.hpp"

#include <chrono>

class game_config_view;
class display;
class plugins_context;

namespace events
{
class mouse_handler_base;
}

namespace hotkey
{
class command_executor;
struct ui_command;
}

namespace soundsource
{
class manager;
}

class controller_base : public events::sdl_handler, public events::pump_monitor
{
public:
	controller_base();
	virtual ~controller_base();

	virtual void play_slice();

	void apply_keyboard_scroll(int x, int y);

	void set_scroll_up(bool on)
	{
		scroll_up_ = on;
	}

	void set_scroll_down(bool on)
	{
		scroll_down_ = on;
	}

	void set_scroll_left(bool on)
	{
		scroll_left_ = on;
	}

	void set_scroll_right(bool on)
	{
		scroll_right_ = on;
	}

	/** Optionally get a command executor to handle context menu events. */
	virtual hotkey::command_executor* get_hotkey_command_executor()
	{
		return nullptr;
	}

protected:
	virtual bool is_browsing() const
	{
		return false;
	}

	/** Get a reference to a mouse handler member a derived class uses. */
	virtual events::mouse_handler_base& get_mouse_handler_base() = 0;

	/** Get a reference to a display member a derived class uses. */
	virtual display& get_display() = 0;

	/** Get (optionally) a soundsources manager a derived class uses. */
	virtual soundsource::manager* get_soundsource_man()
	{
		return nullptr;
	}

	/** Get (optionally) a plugins context a derived class uses. */
	virtual plugins_context* get_plugins_context()
	{
		return nullptr;
	}

	/**
	 * Derived classes should override this to return false when arrow keys
	 * should not scroll the map, hotkeys not processed etc, for example
	 * when a textbox is active
	 * @returns true when arrow keys should scroll the map, false otherwise
	 */
	virtual bool have_keyboard_focus();

	virtual std::vector<std::string> additional_actions_pressed()
	{
		return std::vector<std::string>();
	}

	/**
	 * Handle scrolling by keyboard, joystick and moving mouse near map edges
	 * @see scrolling_, which is set if the display is being scrolled
	 * @return true when there was any scrolling, false otherwise
	 */
	bool handle_scroll(int mousex, int mousey, int mouse_flags);

	/**
	 * Process mouse- and keypress-events from SDL.
	 * Calls various virtual function to allow specialized
	 * behavior of derived classes.
	 */
	void handle_event(const SDL_Event& event) override;

	/** Process keydown (only when the general map display does not have focus). */
	virtual void process_focus_keydown_event(const SDL_Event& /*event*/)
	{
		// No action by default
	}

	virtual void process() override;

	/** Process keydown (always). Overridden in derived classes */
	virtual void process_keydown_event(const SDL_Event& /*event*/)
	{
		// No action by default
	}

	/** Process keyup (always). * Overridden in derived classes */
	virtual void process_keyup_event(const SDL_Event& /*event*/)
	{
		// No action by default
	}

	virtual void show_menu(const std::vector<config>& items_arg, int xloc, int yloc, bool context_menu, display& disp);
	virtual void execute_action(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu);

	virtual bool in_context_menu(const hotkey::ui_command& cmd) const;

	void long_touch_callback(int x, int y);

	const game_config_view& game_config_;

	CKey key_;

	bool scrolling_;
	bool scroll_up_;
	bool scroll_down_;
	bool scroll_left_;
	bool scroll_right_;
	/* When the last scroll tick was processed */
	std::chrono::steady_clock::time_point last_scroll_tick_;
	/* Sub-pixel movement left over from a previous scroll tick.
	 * This is added to the next scroll tick, if scrolling continues. */
	double scroll_carry_x_;
	double scroll_carry_y_;

private:
	/* A separate class for listening key-up events.
	It's needed because otherwise such events might be consumed by a different event context
	and the input system would believe that the player is still holding a key (bug #2573) */
	class keyup_listener : public events::sdl_handler
	{
	public:
		keyup_listener(controller_base& controller)
			: events::sdl_handler(false)
			, controller_(controller)
		{
			join_global();
		}

		void handle_event(const SDL_Event& event) override;

	private:
		controller_base& controller_;
	};

	keyup_listener key_release_listener_;

	bool last_mouse_is_touch_;
	/** Context menu timer */
	size_t long_touch_timer_;
};
