/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
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
#include "config.hpp"

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gettext.hpp"
#include "serialization/unicode.hpp"
#include "sdl/utils.hpp"

#include <boost/foreach.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/bind.hpp>

#include "key.hpp"

#include "SDL.h"

#if !SDL_VERSION_ATLEAST(2,0,0)
#include "sdl/keyboard.hpp"
#endif

static lg::log_domain log_config("config");
#define ERR_G  LOG_STREAM(err,   lg::general)
#define LOG_G  LOG_STREAM(info,  lg::general)
#define DBG_G  LOG_STREAM(debug, lg::general)
#define ERR_CF LOG_STREAM(err,   log_config)

namespace hotkey {

hotkey_list hotkeys_;
config default_hotkey_cfg_;

static unsigned int sdl_get_mods()
{
	unsigned int mods;
	mods = SDL_GetModState();

	mods &= ~KMOD_NUM;
	mods &= ~KMOD_CAPS;
	mods &= ~KMOD_MODE;

	// save the matching for checking right vs left keys
	if (mods & KMOD_SHIFT)
		mods |= KMOD_SHIFT;

	if (mods & KMOD_CTRL)
		mods |= KMOD_CTRL;

	if (mods & KMOD_ALT)
		mods |= KMOD_ALT;

#if SDL_VERSION_ATLEAST(2,0,0)
	if (mods & KMOD_GUI)
	mods |= KMOD_GUI;
#else
	if (mods & KMOD_META)
		mods |= KMOD_META;
#endif

	return mods;
}

const std::string hotkey_base::get_name() const
{
	std::string ret = "";

	if (mod_ & KMOD_CTRL)
		ret += "ctrl";

	ret +=
			(!ret.empty() && !boost::algorithm::ends_with(ret, "+") ?
					"+" : "");
	if (mod_ & KMOD_ALT)
		ret += "alt";

	ret +=
			(!ret.empty() && !boost::algorithm::ends_with(ret, "+") ?
					"+" : "");
	if (mod_ & KMOD_SHIFT)
		ret += "shift";

	ret +=
			(!ret.empty() && !boost::algorithm::ends_with(ret, "+") ?
					"+" : "");
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (mod_ & KMOD_GUI)
#else
	if (mod_ & KMOD_META)
#endif
#ifdef __APPLE__
		ret += "cmd";
#else
		ret += "win";
#endif

	ret +=
			(!ret.empty() && !boost::algorithm::ends_with(ret, "+") ?
					"+" : "");
	return ret += get_name_helper();
}

bool hotkey_base::bindings_equal(hotkey_ptr other)
{

	bool ret;

	if (other == hotkey_ptr()) {
		return false;
	}

	hk_scopes scopematch = hotkey::get_hotkey_command(get_command()).scope
			& hotkey::get_hotkey_command(other->get_command()).scope;

	if (scopematch.none()) {
		return false;
	}

	ret = mod_ == other->mod_ && bindings_equal_helper(other);

	return ret;
}

bool hotkey_base::matches(const SDL_Event &event) const
{
	unsigned int mods = sdl_get_mods();

	if (!hotkey::is_scope_active(hotkey::get_hotkey_command(get_command()).scope) ||
			!active()) {
		return false;
	}

	if ((mods != mod_)) {
		return false;
	}

	return matches_helper(event);
}

void hotkey_base::save(config& item) const
{
	item["command"] = get_command();

	item["shift"] = !!(mod_ & KMOD_SHIFT);
	item["ctrl"] = !!(mod_ & KMOD_CTRL);
#if SDL_VERSION_ATLEAST(2, 0, 0)
	item["cmd"] = !!(mod_ & KMOD_GUI);
#else
	item["cmd"] = !!(mod_ & KMOD_META);
#endif
	item["alt"] = !!(mod_ & KMOD_ALT);

	save_helper(item);
}

hotkey_ptr create_hotkey(const std::string &id, SDL_Event &event)
{
	hotkey_ptr base = hotkey_ptr(new hotkey_void);

	switch (event.type) {
	case SDL_KEYDOWN:
	case SDL_KEYUP: {
		hotkey_keyboard_ptr keyboard(new hotkey_keyboard());
		base = boost::dynamic_pointer_cast<hotkey_base>(keyboard);
		SDL_Scancode code;
#if  SDL_VERSION_ATLEAST(2, 0, 0)
		code = event.key.keysym.scancode;
#else
		code = SDL_GetScancodeFromKey(event.key.keysym.sym);
#endif
		keyboard->set_scancode(code);
		break;
	}
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP: {
		hotkey_mouse_ptr mouse(new hotkey_mouse());
		base = boost::dynamic_pointer_cast<hotkey_base>(mouse);
		mouse->set_button(event.button.button);
		break;
	}
	default:
		ERR_G<< "Trying to bind an unknown event type:" << event.type << "\n";
		break;
	}

	base->set_mods(sdl_get_mods());
	base->set_command(id);
	base->unset_default();

	return base;
}

hotkey_ptr load_from_config(const config& cfg)
{
	hotkey_ptr base = hotkey_ptr(new hotkey_void());

	const std::string& mouse_cfg = cfg["mouse"];
	if (!mouse_cfg.empty()) {
		hotkey_mouse_ptr mouse(new hotkey_mouse());
		base = boost::dynamic_pointer_cast<hotkey_base>(mouse);
		mouse->set_button(cfg["button"].to_int());
	}
	// TODO: add joystick support back
#if 0
	const std::string& joystick_cfg = cfg["joystick"];
	if (!joystick_cfg.empty()) {
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
#endif

	const std::string& key_cfg = cfg["key"];
	if (!key_cfg.empty()) {
		hotkey_keyboard_ptr keyboard(new hotkey_keyboard());
		base = boost::dynamic_pointer_cast<hotkey_base>(keyboard);

		SDL_Scancode scancode = SDL_GetScancodeFromName(key_cfg.c_str());
		if (scancode == SDL_SCANCODE_UNKNOWN) {
			ERR_G<< "Unknown key: " << key_cfg << "\n";
		}
		keyboard->set_scancode(scancode);
	}

	if (base == hotkey_ptr()) {
		return base;
	}

	unsigned int mods = 0;

	if (cfg["shift"].to_bool())
		mods |= KMOD_SHIFT;
	if (cfg["ctrl"].to_bool())
		mods |= KMOD_CTRL;
	if (cfg["cmd"].to_bool())
#if SDL_VERSION_ATLEAST(2, 0, 0)
		mods |= KMOD_GUI;
#else
		mods |= KMOD_META;
#endif
	if (cfg["alt"].to_bool())
		mods |= KMOD_ALT;

	base->set_mods(mods);
	base->set_command(cfg["command"].str());

	return base;
}

bool hotkey_mouse::matches_helper(const SDL_Event &event) const
{
	if (event.type != SDL_MOUSEBUTTONUP && event.type != SDL_MOUSEBUTTONDOWN) {
		return false;
	}

	if (event.button.button != button_) {
		return false;
	}

	return true;
}

const std::string hotkey_mouse::get_name_helper() const
{
	return "mouse " + lexical_cast<std::string>(button_);
}

void hotkey_mouse::save_helper(config &item) const
{
	item["mouse"] = 0;
	if (button_ != 0) {
		item["button"] = button_;
	}
}

const std::string hotkey_keyboard::get_name_helper() const
{
	std::string ret = std::string(SDL_GetKeyName(SDL_GetKeyFromScancode(scancode_)));

	if (ret.size() == 1) {
		boost::algorithm::to_lower(ret);
	}

	return ret;
}

bool hotkey_keyboard::matches_helper(const SDL_Event &event) const
{
	if (event.type != SDL_KEYDOWN && event.type != SDL_KEYUP) {
		return false;
	}

	SDL_Scancode code;
#if  SDL_VERSION_ATLEAST(2, 0, 0)
	code = event.key.keysym.scancode;
#else
	code = SDL_GetScancodeFromKey(event.key.keysym.sym);
#endif

	if (code != scancode_) {
		return false;
	}

	return true;
}

bool hotkey_mouse::bindings_equal_helper(hotkey_ptr other) const
{
	hotkey_mouse_ptr other_m = boost::dynamic_pointer_cast<hotkey_mouse>(other);

	if (other_m == hotkey_mouse_ptr()) {
		return false;
	}

	return button_ == other_m->button_;
}

void hotkey_keyboard::save_helper(config &item) const
{
	if (scancode_ != SDL_SCANCODE_UNKNOWN) {
		item["key"] = SDL_GetScancodeName(scancode_);
	}
}

bool has_hotkey_item(const std::string& command)
{
	BOOST_FOREACH(hotkey_ptr item, hotkeys_) {
		if (item->get_command() == command) {
			return true;
		}

	}
	return false;
}

bool hotkey_keyboard::bindings_equal_helper(hotkey_ptr other) const
{
	hotkey_keyboard_ptr other_k = boost::dynamic_pointer_cast<hotkey_keyboard>(
			other);
	if (other_k == hotkey_keyboard_ptr()) {
		return false;
	}

	return scancode_ == other_k->scancode_;
}

void del_hotkey(hotkey_ptr item)
{
	if (!hotkeys_.empty()) {
		hotkeys_.erase(std::remove(hotkeys_.begin(), hotkeys_.end(), item));
	}
}

void add_hotkey(const hotkey_ptr item)
{

	if (item == hotkey_ptr()) {
		return;
	}

	scope_changer scope_ch;
	set_active_scopes(hotkey::get_hotkey_command(item->get_command()).scope);

	if (!hotkeys_.empty()) {
		hotkeys_.erase(
				std::remove_if(hotkeys_.begin(), hotkeys_.end(),
						boost::bind(&hotkey_base::bindings_equal, _1, (item))),
				hotkeys_.end());
	}

	hotkeys_.push_back(item);

}

void clear_hotkeys(const std::string& command)
{
	BOOST_FOREACH(hotkey::hotkey_ptr item, hotkeys_) {
		if (item->get_command() == command) {
			item->clear();
		}
	}
}

void clear_hotkeys()
{
	hotkeys_.clear();
}

const hotkey_ptr get_hotkey(const SDL_Event &event)
{
	BOOST_FOREACH(hotkey_ptr item, hotkeys_) {
		if (item->matches(event)) {
			return item;
		}
	}
	return hotkey_ptr(new hotkey_void());
}

void load_hotkeys(const config& cfg, bool set_as_default)
{
	BOOST_FOREACH(const config &hk, cfg.child_range("hotkey")) {

		hotkey_ptr item = load_from_config(hk);
		if (!set_as_default) {
			item->unset_default();
		}

		if (!item->null()) {
			add_hotkey(item);
		}
	}

	if (set_as_default) {
		default_hotkey_cfg_ = cfg;
	}
}

void reset_default_hotkeys()
{
	hotkeys_.clear();

	if (!default_hotkey_cfg_.empty()) {
		load_hotkeys(default_hotkey_cfg_, true);
	} else {
		ERR_G<< "no default hotkeys set yet; all hotkeys are now unassigned!" << std::endl;
	}
}

const hotkey_list& get_hotkeys()
{
	return hotkeys_;
}

void save_hotkeys(config& cfg)
{
	cfg.clear_children("hotkey");

	BOOST_FOREACH(hotkey_ptr item, hotkeys_) {
		if (!item->is_default() && item->active()) {
			item->save(cfg.add_child("hotkey"));
		}
	}
}

std::string get_names(std::string id)
{

	std::vector<std::string> names;
	BOOST_FOREACH(const hotkey::hotkey_ptr item, hotkeys_) {
		if (item->get_command() == id && (!item->null())) {
			names.push_back(item->get_name());
		}
	}

	// These are hard-coded, non-rebindable hotkeys
	if (id == "quit") {
		names.push_back("escape");
	}
	else if (id == "quit-to-desktop") {
#ifdef __APPLE__
		names.push_back("cmd+q");
#else
		names.push_back("alt+F4");
#endif
	}

	return boost::algorithm::join(names, ", ");
}

}
