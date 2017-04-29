/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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

#include <SDL_events.h>
#include <SDL.h>
#include <memory>
#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>

class config;
class CVideo;
namespace hotkey {

/* forward declarations */
class hotkey_base;
class hotkey_mouse;
class hotkey_keyboard;
typedef std::shared_ptr<hotkey_base> hotkey_ptr;
typedef std::shared_ptr<hotkey_mouse> hotkey_mouse_ptr;
typedef std::shared_ptr<hotkey_keyboard> hotkey_keyboard_ptr;

typedef std::vector<hotkey::hotkey_ptr> hotkey_list;
typedef std::vector<hotkey::hotkey_ptr>::iterator hotkey_list_iter;

/**
 * This is the base class for hotkey event matching.
 */
class hotkey_base
{
public:
	/**
	 * Initialises a new empty hotkey that will be disabled
	 */
	hotkey_base() : command_("null"), is_default_(true), is_disabled_(false), mod_(0)
	{}

	void set_command(const std::string& command)
	{
		command_ = command;
	}

	/**
	 * Set keyboard modifiers.
	 * @param mods Bitmask of SDL_Keymod.
	 */
	void set_mods(unsigned int mods)
	{
		mod_ = mods;
	}

	/**
	 *  Returns the string name of the HOTKEY_COMMAND
	 *  @return The unique string for a hotkey command.
	 **/
	const std::string& get_command() const
	{
		return command_;
	}

	/**
	 * Returns the translated description
	 * @todo unused
	 * @return internationalised description of the command.
	 **/
	const std::string get_description() const;

	/**
	 * This controls whether the item should appear in the hotkey preferences.
	 * @return true if the item should be hidden
	 **/
	virtual bool hidden() const
	{
		return false;
	}

	/**
	 * This indicates whether a hotkey is from the default config or if it's
	 * from the user preferences.
	 * @return true if from the default configurations, false otherwise
	 */
	bool is_default() const
	{
		return is_default_;
	}

	/**
	 * Used to indicate that a hotkey is overriden and should be treated as
	 * a user-set hotkey.
	 */
	void unset_default()
	{
		is_default_ = false;
	}

	bool is_disabled() const
	{
		return is_disabled_;
	}
	void disable()
	{
		is_disabled_ = true;
	}
	void enable()
	{
		is_disabled_ = false;
	}

	/**
	 * Unbind this hotkey by linking it to the null-command
	 */
	void clear()
	{
		command_ = "null";
	}

	/**
	 * Returns whether this hotkey points to the null-command
	 * @return true if it points to the null-command, false otherwise.
	 */
	bool null() const
	{
		return command_ == "null";
	}

	/*
	 * Returns whether there is a associated hotkey_command.
	 * If the none of the hotkey_commands fits this hotkey_item then
	 * @param get_hotkey_command will return the hotkey_command::null_command().
	 * @return true if the hotkey is not bound to the null-command.
	 */
	bool active() const
	{
		return command_ != "null";
	}

	/**
	 * Evaluates whether the hotkey bindings are valid.
	 * @return true if they are valid, false otherwise.
	 */
	virtual bool valid() const = 0;

	/**
	 * Save the hotkey into the configuration object.
	 * @param cfg The configuration object to save into.
	 */
	void save(config& cfg) const;

	/**
	 * Return "name" of hotkey. Example :"ctrl+alt+g"
	 * @return The string representation of the keybindings
	 */
	const std::string get_name() const;

	/**
	 * Used to evaluate whether:
	 * 1. The hotkey is valid in the current scope.
	 * 2. The Keyboard modifiers and SDL_Event mathes this hotkey.
	 *
	 * @param event The SDL_Event that has triggered and is being evaluated.
	 */
	bool matches(const SDL_Event& event) const;

	/**
	 * Checks whether the hotkey bindings and scope are equal.
	 * @param other the hokey bindings to compare against.
	 * @return true if %other has same scope and bindings.
	 */
	virtual bool bindings_equal(hotkey_ptr other);

	virtual ~hotkey_base()
	{}

protected:
	/**
	 *  This is invoked by hotkey_base::get_name and must be implemented by subclasses.
	 *  Keyboard modifiers are handled in this class, other hotkeys in the respective classes
	 */
	virtual const std::string get_name_helper() const = 0;
	/**
	 * This is invoked by hotkey_base::matches as a helper for the concrete classes.
	 * Implementing classes should only check their parts of the hotkey.
	 * @param event The SDL_Event being generated.
	 * @returns true if they match, false otherwise.
	 */
	virtual bool matches_helper(const SDL_Event &event) const = 0;
	virtual void save_helper(config& cfg) const = 0;
	/**
	 * This is invoked by hotkey_base::bindings_equal as a helper for the concrete classes.
	 * Implementing classes should only check their parts of the hotkey.
	 * @param other The other hotkey the check against. Not guaranteed to be the same subclass.
	 * @returns true if they match, false otherwise.
	 */
	virtual bool bindings_equal_helper(hotkey_ptr other) const = 0;

	/**
	 * The command that should be executed, or "null".
	 */
	std::string command_;

	/**
	 * is_default_ is true if the hot-key is part of the default hot-key list defined in data/core/hotkeys.cfg.
	 * is_default_ is false if it is not, in which case it would be defined in the user's preferences file.
	 */
	bool is_default_;

	/*
	 * The original design of using a "null" command to indicate a disabled hot-key is ambiguous with regards
	 * to when to save a user hot-key to preferences as well as when a default hot-key should be flagged as
	 * disabled. So introduce a separate disabled flag to resolve the ambiguity.
	 * Where the flag is true, the hot-key should not be written to preferences unless it is a default hot-key.
	 */
	bool is_disabled_;

	/*
	 * Keyboard modifiers. Treat as opaque, only do comparisons.
	 */
	unsigned int mod_;
};

/**
 * This class is responsible for handling keys, not modifiers.
 */
class hotkey_keyboard: public hotkey_base
{
public:
	/**
	 * Initialise new instance of this class that has no key associated with is.
	 */
	hotkey_keyboard() : hotkey_base(), keycode_(SDLK_UNKNOWN), text_("")
	{}

	/**
	 * Set the keycode associated with this class.
	 * @param keycode_ The SDL_Keycode that this hotkey should be associated with
	 */
	void set_keycode(SDL_Keycode keycode)
	{
		keycode_ = keycode;
	}

	void set_text(std::string text)
	{
		text_ = text;
		boost::algorithm::to_lower(text_);
	}

	/**
	 * Checks whether this hotkey has been set to a sensible value.
	 * @ return true if it is a known key
	 */
	virtual bool valid() const
	{
		return keycode_ != SDLK_UNKNOWN && text_ != "";
	}

protected:
	SDL_Keycode keycode_;
	std::string text_;

	virtual void save_helper(config& cfg) const;
	virtual const std::string get_name_helper() const;
	virtual bool matches_helper(const SDL_Event &event) const;
	virtual bool bindings_equal_helper (hotkey_ptr other) const;
};

/**
 * This class is used to return non-valid results in order to save
 * other people from null checks.
 */
class hotkey_void: public hotkey_base
{
public:
	hotkey_void() : hotkey_base()
	{}
	virtual bool valid() const
	{
		return false;
	}
protected:
	virtual void save_helper(config&) const
	{}
	virtual const std::string get_name_helper() const
	{
		return "";
	}
	virtual bool matches_helper(const SDL_Event&) const
	{
		return false;
	}
	virtual bool bindings_equal_helper(hotkey_ptr) const
	{
		return false;
	}
};

/**
 * This class is responsible for handling mouse button presses.
 */
class hotkey_mouse: public hotkey_base
{
public:
	/**
	 * Initialise new instance of this class that has no button associated with is.
	 */
	hotkey_mouse() : hotkey_base(), button_ (0)
	{}

	/**
	 * Returns true if the hotkey has a valid mouse button associated with it.
	 * @return true if a mouse button is set, false otherwise.
	 */
	virtual bool valid() const
	{
		return button_ != 0;
	}

	/* new functionality for this class */
	void set_button(int button)
	{
		button_ = button;
	}
protected:
	int button_;

	virtual void save_helper(config& cfg) const;
	virtual const std::string get_name_helper() const;
	virtual bool matches_helper(const SDL_Event &event) const;
	virtual bool bindings_equal_helper (hotkey_ptr other) const;
};

/**
 * @todo not implemented
 */
class hotkey_joystick: public hotkey_base
{
protected:
	int button_;
};

/**
 * Create and instantiate a hotkey from a config element.
 * @param cfg The config element to read for data.
 * @return The new instance of the hotkey item.
 */
hotkey_ptr load_from_config(const config& cfg);

/*
 * Scans the list of hotkeys to see if one has been bound to the command.
 * @param command The command that is searched for
 * @return true if there is a hotkey item that has the command bound.
 */
bool has_hotkey_item(const std::string& command);

/**
 * Add a hotkey to the list of hotkeys.
 * @param item The item to add.
 */
void add_hotkey(const hotkey_ptr item);

/**
 * Remove a hotkey from the list of hotkeys
 * @todo unusued?
 */
void del_hotkey(const hotkey_ptr item);

/**
 * Create a new hotkey item for a command from an SDL_Event.
 * @param id The command to bind to.
 * @param event The SDL_Event to base the creation on.
 */
hotkey_ptr create_hotkey(const std::string &id, const SDL_Event &event);

/**
 * Iterate through the list of hotkeys and return a hotkey that matches
 * the SDL_Event and the current keyboard modifier state.
 * @param event The SDL_Event to use as a template.
 * @return The newly created hotkey item.
 */
const hotkey_ptr get_hotkey(const SDL_Event &event);

/**
 * Iterates through all hotkeys present in the config struct and creates and adds
 * them to the hotkey list.
 * @param cfg The config struct to load from.
 * @param set_as_default Indicates whether the config struct should be treated as the
 * default game settings.
 */
void load_hotkeys(const config& cfg, bool set_as_default = false);

/**
 * Reset all hotkeys to the defaults.
 */
void reset_default_hotkeys();

/**
 * Returns the list of hotkeys.
 */
const hotkey_list& get_hotkeys();

/**
 * Unset the command bindings for all hotkeys matching the command.
 *
 * @param command The binding to be unset
 */
void clear_hotkeys(const std::string& command);

/**
 * Unset the bindings for all hotkeys.
 */
void clear_hotkeys();

/**
 * Returns a comma-separated string of hotkey names. A hotkey name is in the form of
 * "ctrl+l" or "n" or "mouse 1". The comman separated string is of the form "ctrl+l,n,mouse 1".
 * @return The comma separated string of hotkey names.
 */
std::string get_names(const std::string& id);

/**
 * Save the non-default hotkeys to the config.
 * @param cfg The config to save to.
 */
void save_hotkeys(config& cfg);

bool is_hotkeyable_event(const SDL_Event &event);

}

#endif
