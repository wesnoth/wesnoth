/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "log.hpp"
#include "hotkey_item.hpp"
#include "hotkey_command.hpp"

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gettext.hpp"
#include "serialization/unicode.hpp"
#include "sdl_utils.hpp"

#include "boost/foreach.hpp"
#include <boost/algorithm/string/join.hpp>

#include "key.hpp"

static lg::log_domain log_config("config");
#define ERR_G  LOG_STREAM(err,   lg::general)
#define LOG_G  LOG_STREAM(info,  lg::general)
#define DBG_G  LOG_STREAM(debug, lg::general)
#define ERR_CF LOG_STREAM(err,   log_config)

namespace {

std::vector<hotkey::hotkey_item> hotkeys_;
config default_hotkey_cfg_;

hotkey::hotkey_item null_hotkey_("null");

hotkey::hotkey_item& get_hotkey(int mouse, int joystick, int button, int hat, int value,
		bool shift, bool ctrl, bool cmd, bool alt)
{
	std::vector<hotkey::hotkey_item>::iterator itor;

	for (itor = hotkeys_.begin(); itor != hotkeys_.end(); ++itor) {

		if ( !( hotkey::is_scope_active(hotkey::get_hotkey_command(itor->get_command()).scope) && itor->active() ) ) {
			continue;
		}

		if ( itor->get_shift() != shift || itor->get_ctrl() != ctrl
				|| itor->get_cmd() != cmd || itor->get_alt() != alt ) {
			continue;
		}

		if ( itor->get_joystick() == joystick && itor->get_button() == button
				&& itor->get_hat() == hat && itor->get_value() == value
				&& itor->get_mouse() == mouse ) {
			return *itor;
		}
	}

	return null_hotkey_;
}

hotkey::hotkey_item& get_hotkey(int character, int keycode,
		bool shift, bool ctrl,	bool cmd, bool alt)
{
	std::vector<hotkey::hotkey_item>::iterator itor;

	DBG_G << "getting hotkey: char=" << lexical_cast<std::string>(character)
		<< " keycode="  << lexical_cast<std::string>(keycode) << " "
		<< (shift ? "shift," : "")
		<< (ctrl  ? "ctrl,"  : "")
		<< (cmd   ? "cmd,"   : "")
		<< (alt   ? "alt,"   : "")
		<< "\n";

	// Sometimes control modifies by -64, ie ^A == 1.
	if (0 < character && character < 64 && ctrl) {
		if (shift) {
			character += 64;
		} else {
			character += 96;
		}
		/// @todo
		DBG_G << "Mapped to character " << lexical_cast<std::string>(character) << "\n";
	}

	// For some reason on Mac OS, if cmd and shift are down, the character doesn't get upper-cased
	if (cmd && character > 96 && character < 123 && shift) {
		character -= 32; }

	bool found = false;

	for (itor = hotkeys_.begin(); itor != hotkeys_.end(); ++itor) {
		if (itor->get_character() != -1) {
			if (character == itor->get_character()) {
				if (ctrl == itor->get_ctrl()
						&& cmd == itor->get_cmd()
						&& alt == itor->get_alt()) {
					if (hotkey::is_scope_active(hotkey::get_hotkey_command(itor->get_command()).scope) && itor->active()) {
						DBG_G << "Could match by character..." << "yes\n";
						found = true;
						break;
					} else {
						DBG_G << "Could match by character..." << "yes, but scope is inactive\n";
					}
				}
				DBG_G << "Could match by character..." << "but modifiers different\n";
			}
		} else if (itor->get_keycode() != -1) {
			if (keycode == itor->get_keycode()) {
				if (shift == itor->get_shift()
						&& ctrl == itor->get_ctrl()
						&& cmd  == itor->get_cmd()
						&& alt  == itor->get_alt()) {
					if (hotkey::is_scope_active(hotkey::get_hotkey_command(itor->get_command()).scope) && itor->active()) {
						DBG_G << "Could match by keycode..." << "yes\n";
						found = true;
						break;
					} else {
						DBG_G << "Could match by keycode..." << "yes, but scope is inactive\n";
					}
				}
				DBG_G << "Could match by keycode..." << "but modifiers different\n";
			}
		}
		if (found) { break; }
	}

	if (!found) {
		return null_hotkey_; }

	return *itor;
}


}

namespace hotkey {


const hotkey_item& get_hotkey(const SDL_JoyButtonEvent& event)
{
	CKey keystate;
	bool shift = keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT];
	bool ctrl  = keystate[SDLK_RCTRL]  || keystate[SDLK_LCTRL];
	bool cmd   = keystate[SDLK_RMETA]  || keystate[SDLK_LMETA];
	bool alt   = keystate[SDLK_RALT]   || keystate[SDLK_LALT];

	return get_hotkey(-1, event.which, event.button, -1, -1, shift, ctrl, cmd, alt);
}

const hotkey_item& get_hotkey(const SDL_JoyHatEvent& event)
{
	CKey keystate;
	bool shift = keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT];
	bool ctrl  = keystate[SDLK_RCTRL]  || keystate[SDLK_LCTRL];
	bool cmd   = keystate[SDLK_RMETA]  || keystate[SDLK_LMETA];
	bool alt   = keystate[SDLK_RALT]   || keystate[SDLK_LALT];

	return get_hotkey(-1, event.which, -1, event.hat, event.value, shift, ctrl, cmd, alt);
}


const hotkey_item& get_hotkey(const SDL_MouseButtonEvent& event)
{
	CKey keystate;
	bool shift = keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT];
	bool ctrl  = keystate[SDLK_RCTRL]  || keystate[SDLK_LCTRL];
	bool cmd   = keystate[SDLK_RMETA]  || keystate[SDLK_LMETA];
	bool alt   = keystate[SDLK_RALT]   || keystate[SDLK_LALT];

	return get_hotkey(event.which, -1, event.button, -1, -1, shift, ctrl, cmd, alt);
}


const hotkey_item& get_hotkey(const SDL_KeyboardEvent& event)
{
	return get_hotkey(
#if SDL_VERSION_ATLEAST(2, 0, 0)
			event.keysym.scancode,
#else
			event.keysym.unicode,
#endif
			event.keysym.sym,
			(event.keysym.mod & KMOD_SHIFT) != 0,
			(event.keysym.mod & KMOD_CTRL)  != 0,
			(event.keysym.mod & KMOD_META)  != 0,
			(event.keysym.mod & KMOD_ALT)   != 0
			);
}



std::string get_names(std::string id) {

	std::vector<std::string> names;
	BOOST_FOREACH(const hotkey::hotkey_item& item, hotkeys_) {
		if (item.get_command() == id && (!item.null()) ) {
			names.push_back(item.get_name());
		}
	}

	return boost::algorithm::join(names, ", ");
}

const hotkey_item& get_hotkey(int mouse, int joystick, int button, int hat, int value,
		bool shift, bool ctrl, bool cmd, bool alt)
{
	return ::get_hotkey(mouse, joystick, button, hat, value,
			shift, ctrl, cmd, alt);
}

const hotkey::hotkey_item& get_hotkey(int character, int keycode,
		bool shift, bool ctrl,	bool cmd, bool alt)
{
	return ::get_hotkey(character, keycode,
			shift, ctrl, cmd, alt);
}

bool has_hotkey_item(const std::string& command)
{
	BOOST_FOREACH(hotkey_item& item, hotkeys_)
	{
		if(item.get_command() == command)
			return true;
	}
	return false;
}

void add_hotkey(const hotkey_item& item) {

	scope new_scope = hotkey::get_hotkey_command(item.get_command()).scope;
	scope_changer scope_ch;
	deactivate_all_scopes();
	hotkey::set_scope_active(new_scope);
	hotkey::set_scope_active(hotkey::SCOPE_GENERAL);

	hotkey_item& old_hk = (item.get_mouse() != -1 || item.get_joystick() != -1) ?
			::get_hotkey(item.get_mouse(), item.get_joystick(), item.get_button(), item.get_hat()
					, item.get_value(), item.get_shift(), item.get_ctrl(), item.get_cmd(), item.get_alt()) :
					::get_hotkey(item.get_character(), item.get_keycode(),
							item.get_shift(), item.get_ctrl(), item.get_cmd(), item.get_alt());

	if (old_hk.active()) {
		old_hk.set_command(item.get_command());
		old_hk.unset_default();
	}
	else
		hotkeys_.push_back(item);
}

void clear_hotkeys(const std::string& command)
{
	BOOST_FOREACH(hotkey::hotkey_item& item, hotkeys_) {
		if (item.get_command() == command) {
			item.clear(); }
	}
}

void clear_hotkeys()
{
	hotkeys_.clear();
}

void load_hotkeys(const config& cfg, bool set_as_default)
{
	BOOST_FOREACH(const config &hk, cfg.child_range("hotkey")) {
		add_hotkey(hotkey_item(hk, set_as_default));
	}

	if (set_as_default) {
		default_hotkey_cfg_ = cfg;
	}
}

void reset_default_hotkeys()
{
	hotkeys_.clear();

	if(!default_hotkey_cfg_.empty()) {
		load_hotkeys(default_hotkey_cfg_, true);
	} else {
		ERR_G << "no default hotkeys set yet; all hotkeys are now unassigned!\n";
	}
}

const std::vector<hotkey_item>& get_hotkeys()
{
	return hotkeys_;
}

void save_hotkeys(config& cfg)
{
	cfg.clear_children("hotkey");

	for(std::vector<hotkey_item>::iterator i = hotkeys_.begin();
			i != hotkeys_.end(); ++i)
	{
		if (!i->is_default())
			i->save(cfg.add_child("hotkey"));
	}
}

bool hotkey_item::active() const
{
	return !(command_  == "null");
}

bool hotkey_item::valid() const
{
	return (character_ | keycode_ | joystick_ | mouse_ | button_ | hat_ | value_) != 0;
}

// There are two kinds of "key" values.
// One refers to actual keys, like F1 or SPACE.
// The other refers to characters produced, eg 'M' or ':'.
// For the latter, specifying shift+; doesn't make sense,
// because ; is already shifted on French keyboards, for example.
// You really want to say ':', however that is typed.
// However, when you say shift+SPACE,
// you're really referring to the space bar,
// as shift+SPACE usually just produces a SPACE character.
void hotkey_item::load_from_config(const config& cfg)
{
	command_ = cfg["command"].str();

	const std::string& mouse = cfg["mouse"];
	if (!mouse.empty()) {
			mouse_ = cfg["mouse"].to_int();
			button_ = cfg["button"].to_int();
	}
	const std::string& joystick = cfg["joystick"];
	if (!joystick.empty()) {
		joystick_ = cfg["joystick"].to_int();
	}
	const std::string& hat = cfg["hat"];
	if (!hat.empty()) {
		hat_ = cfg["hat"].to_int();
		value_ = cfg["value"].to_int();
	}

	const std::string& button = cfg["button"];
	if (!button.empty()) {
		button_ = cfg["button"].to_int();
	}

	shift_ = cfg["shift"].to_bool();
	ctrl_  = cfg["ctrl"].to_bool();
	cmd_   = cfg["cmd"].to_bool();
	alt_   = cfg["alt"].to_bool();

	const std::string& key = cfg["key"];
	if (key.empty()) {
		return;
	}

	ucs4::string wkey = unicode_cast<ucs4::string>(key);

	// They may really want a specific key on the keyboard:
	// we assume that any single character keyname is a character.
	if (wkey.size() > 1) {

		keycode_ = sdl_keysym_from_name(key);
		if (keycode_ == SDLK_UNKNOWN) {
			if (tolower(key[0]) != 'f') {
				ERR_CF << "hotkey key '" << key << "' invalid\n";
			} else {
				int num = lexical_cast_default<int>(key.c_str() + 1);
				keycode_ = num + SDLK_F1 - 1;
			}
		}
	} else if (key == " " || shift_
#ifdef __APPLE__
		|| alt_
#endif
		) {
		// Space must be treated as a key because shift-space
		// isn't a different character from space,
		// and control key makes it go weird.
		// shift=yes should never be specified on single characters
		// (eg. key=m, shift=yes would be key=M),
		// but we don't want to break old preferences files.
		keycode_ = wkey[0];
	} else {
		character_ = wkey[0];
	}
}

void hotkey_item::set_command(const std::string& command) {
	command_ = command;
}

std::string hotkey_item::get_name() const
{
	std::stringstream str;

	if (shift_) { str << "shift+"; }
	if (ctrl_)  { str << "ctrl+";  }
	if (cmd_)   { str << "cmd+";   }
	if (alt_)   { str << "alt+";   }

	if (mouse_     >=  0) { str << _("Mouse") << mouse_ << _("Button") << button_; }
	if (character_ != -1) { str << static_cast<char>(character_); }
	if (keycode_   != -1) { str << SDL_GetKeyName(SDLKey(keycode_)); }
	if (joystick_  >=  0) { str << _("Joystick") << joystick_ << _("Button") << button_; }

	if (value_ >= 0) {
		std::string direction;
		switch (value_) {
			case SDL_HAT_CENTERED:
				direction = _("Centered");
				break;
			case SDL_HAT_UP:
				direction = _("Up");
				break;
			case SDL_HAT_RIGHT:
				direction = _("Right");
				break;
			case SDL_HAT_DOWN:
				direction = _("Down");
				break;
			case SDL_HAT_LEFT:
				direction = _("Left");
				break;
			case SDL_HAT_RIGHTUP:
				direction = _("RightUp");
				break;
			case SDL_HAT_RIGHTDOWN:
				direction = _("RightDown");
				break;
			case SDL_HAT_LEFTUP:
				direction = _("LeftUp");
				break;
			case SDL_HAT_LEFTDOWN:
				direction = _("LeftDown");
				break;
			default:
				direction = _("Unknown");
				break;
		}
		str << _("Joystick") << joystick_ << _("Hat") << button_ << direction;
	}

	return str.str();
}

void hotkey_item::clear()
{
	command_ = "null";
}

void hotkey_item::save(config& item) const
{
	if (get_button()    >= 0) item["button"]   = get_button();
	if (get_joystick()  >= 0) item["joystick"] = get_joystick();
	if (get_hat()       >= 0) item["hat"]      = get_hat();
	if (get_value()     >= 0) item["value"]    = get_value();
	if (get_keycode()   >= 0) item["key"]      = SDL_GetKeyName(SDLKey(get_keycode()));
	if (get_character() >= 0) item["key"]      = unicode_cast<utf8::string, ucs4::char_t>(get_character()); // Second template argument because get_character returns a signed int
	if (get_mouse()     >= 0) item["mouse"]    = get_mouse();
	if (get_button()    >= 0) item["button"]   = get_button();

	item["command"] = get_command();
	if (get_shift()) item["shift"] = get_shift();
	if (get_ctrl() ) item["ctrl"]  = get_ctrl();
	if (get_cmd()  ) item["cmd"]   = get_cmd();
	if (get_alt()  ) item["alt"]   = get_alt();
}

void hotkey_item::set_jbutton(int joystick, int button,
		bool shift, bool ctrl, bool cmd, bool alt)
{
	joystick_ = joystick;
	button_   = button;
	shift_    = shift;
	ctrl_     = ctrl;
	cmd_      = cmd;
	alt_      = alt;
}

void hotkey_item::set_jhat(int joystick, int hat, int value,
		bool shift, bool ctrl, bool cmd, bool alt)
{
	joystick_ = joystick;
	hat_      = hat;
	value_    = value;
	shift_    = shift;
	ctrl_     = ctrl;
	cmd_      = cmd;
	alt_      = alt;
}

void hotkey_item::set_mbutton(int mouse, int button,
		bool shift, bool ctrl, bool cmd, bool alt)
{
	mouse_  = mouse;
	button_ = button;
	shift_  = shift;
	ctrl_   = ctrl;
	cmd_    = cmd;
	alt_    = alt;
}

void hotkey_item::set_key(int character, int keycode,
		bool shift, bool ctrl, bool cmd, bool alt)
{
	LOG_G << "setting hotkey: char=" << lexical_cast<std::string>(character)
		   << " keycode="  << lexical_cast<std::string>(keycode) << " "
		   << (shift ? "shift," : "")
		   << (ctrl  ? "ctrl,"  : "")
		   << (cmd   ? "cmd,"   : "")
		   << (alt   ? "alt,"   : "")
		   << "\n";

	// Sometimes control modifies by -64, ie ^A == 1.
	if (character < 64 && ctrl) {
		if (shift) {
			character += 64; }
		else {
			character += 96; }
		LOG_G << "Mapped to character " << lexical_cast<std::string>(character) << "\n";
	}

	// For some reason on Mac OS, if cmd and shift are down, the character doesn't get upper-cased
	if (cmd && character > 96 && character < 123 && shift) {
		character -= 32; }

	// We handle simple cases by character, others by the actual key.
	if (isprint(character) && !isspace(character)) {
		character_ = character;
		ctrl_      = ctrl;
		cmd_       = cmd;
		alt_       = alt;
		LOG_G << "type = BY_CHARACTER\n";
	} else {
		keycode_ = keycode;
		shift_   = shift;
		ctrl_    = ctrl;
		cmd_     = cmd;
		alt_     = alt;
		LOG_G << "type = BY_KEYCODE\n";
	}
}

}
