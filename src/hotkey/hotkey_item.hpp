/*
   Copyright (C) 2003 - 2015 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef HOTKEY_ITEM_HPP_INCLUDED
#define HOTKEY_ITEM_HPP_INCLUDED

#include "config.hpp"
#include "SDL_events.h"

namespace hotkey {


class hotkey_item {

public:

	explicit hotkey_item(const std::string& command) :
	command_(command),
	shift_(false), 	ctrl_(false), cmd_(false), alt_(false),
	character_(-1), keycode_(-1),
	joystick_(-1), mouse_(-1),
	button_(-1), hat_(-1), value_(-1),
	is_default_(false)
	{}

	explicit hotkey_item(const config& cfg, bool is_default):
				command_("null"),
				shift_(false), 	ctrl_(false), cmd_(false), alt_(false),
				character_(-1), keycode_(-1),
				joystick_(-1), mouse_(-1),
				button_(-1), hat_(-1), value_(-1),
				is_default_(is_default)
	{
		load_from_config(cfg);
	}

	void load_from_config(const config& cfg);

	void set_command(const std::string& command);

	/** get the string name of the HOTKEY_COMMAND */
	const std::string get_command() const { return command_; }
	/** The translated description */
	const std::string get_description() const;
	/** @return if the item should appear in the hotkey preferences */
	bool hidden() const;

	bool is_default() const { return is_default_; }

	void unset_default(bool is_not_default = true) {
		is_default_ = !is_not_default;
	}

	/// Return "name" of hotkey. Example :"ctrl+alt+g"
	std::string get_name() const;

	void clear();

	bool null() const { return command_  == "null"; }

	/// returns weather there is a associated hotkey_command.
	// if the none of the hotkey_commands fits this hotkey_item then get_hotkey_command will return the hotkey_command::null_command().
	bool active() const;

	/// checks weather that hotkey "makes sense" meaning weather one of character, keycode, joystick, mouse, button, hat, value is set.
	/// i don't know what the axis_.. values are so i ignore them.
	bool valid() const;

	void save(config& cfg) const;

	/// Return the actual key code.
	int get_keycode() const { return keycode_; }
	/// Returns unicode value of the pressed key.
	int get_character() const { return character_; }

	/** for buttons on devices */
	int get_button() const { return button_; }
	int get_joystick() const { return joystick_; }
	int get_hat() const { return hat_; }
	int get_mouse() const { return mouse_; }
	int get_value() const { return value_; }

	/** modifiers */
	bool get_shift() const { return shift_; }
	bool get_ctrl() const { return ctrl_; }
	bool get_cmd() const { return cmd_; }
	bool get_alt() const { return alt_; }

	void set_jbutton(int button, int joystick, bool shift, bool ctrl, bool cmd, bool alt);
	void set_jhat(int joystick, int hat, int value, bool shift, bool ctrl, bool cmd, bool alt);
	void set_key(int character, int keycode, bool shift, bool ctrl, bool cmd, bool alt);
	void set_mbutton(int device, int button, bool shift, bool ctrl, bool cmd, bool alt);

protected:

	// The unique command associated with this item.
	// Used to bind to a hotkey_command struct.
	std::string command_;

	// modifier keys
	bool shift_, ctrl_, cmd_, alt_;

	// Actual unicode character
	int character_;

	// These used for function keys (which don't have a unicode value) or
	// space (which doesn't have a distinct unicode value when shifted).
	int keycode_;

	int joystick_, mouse_;
	int button_;
	int hat_, value_;

	bool is_default_;

};

/// Initiated weather there is at least one hotkey_item with the given command
bool has_hotkey_item(const std::string& command);

void add_hotkey(const hotkey_item& item);

const hotkey_item& get_hotkey(int mouse, int joystick,
		int button, int hat, int value,
		bool shift, bool ctrl, bool cmd, bool alt);
const hotkey_item& get_hotkey(int character, int keycode,
		bool shift, bool ctrl, bool cmd, bool alt);

const hotkey_item& get_hotkey(const SDL_JoyButtonEvent& event);
const hotkey_item& get_hotkey(const SDL_JoyHatEvent& event);
const hotkey_item& get_hotkey(const SDL_KeyboardEvent& event);
const hotkey_item& get_hotkey(const SDL_MouseButtonEvent& event);

void load_hotkeys(const config& cfg, bool set_as_default = false);
void reset_default_hotkeys();

const std::vector<hotkey_item>& get_hotkeys();

void clear_hotkeys(const std::string& command);
void clear_hotkeys();

/// returns all hotkey_item s that point to this, command in the form "A,Strg+M,F4"
/// used in the preferences menu
std::string get_names(std::string id);

void save_hotkeys(config& cfg);

/// Append a single hotkey item to @a cfg.
//void save_hotkey(config& cfg, const hotkey_item & item);


}

#endif
