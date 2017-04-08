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
#include "hotkey/hotkey_item.hpp"
#include "hotkey/hotkey_command.hpp"
#include "config.hpp"

#define GETTEXT_DOMAIN "wesnoth-lib"

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string.hpp>
#include "utils/functional.hpp"

#include <SDL.h>
#include <key.hpp>


static lg::log_domain log_config("config");
#define ERR_G  LOG_STREAM(err,   lg::general())
#define LOG_G  LOG_STREAM(info,  lg::general())
#define DBG_G  LOG_STREAM(debug, lg::general())
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

	if (mods & KMOD_GUI)
	mods |= KMOD_GUI;

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
	if (mod_ & KMOD_GUI)
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
	if (!hotkey::is_scope_active(hotkey::get_hotkey_command(get_command()).scope) ||
			!active() || is_disabled()) {
		return false;
	}

	return matches_helper(event);
}

void hotkey_base::save(config& item) const
{
	item["command"] = get_command();
	item["disabled"] = is_disabled();

	item["shift"] = !!(mod_ & KMOD_SHIFT);
	item["ctrl"] = !!(mod_ & KMOD_CTRL);
	item["cmd"] = !!(mod_ & KMOD_GUI);
	item["alt"] = !!(mod_ & KMOD_ALT);

	save_helper(item);
}

hotkey_ptr create_hotkey(const std::string &id, SDL_Event &event)
{
	hotkey_ptr base = hotkey_ptr(new hotkey_void);
	unsigned mods = sdl_get_mods();

	switch (event.type) {
	case SDL_KEYDOWN:
	case SDL_KEYUP: {
		if (mods & KMOD_CTRL || mods & KMOD_ALT || mods & KMOD_GUI || CKey::is_uncomposable(event.key)) {
			hotkey_keyboard_ptr keyboard(new hotkey_keyboard());
			base = std::dynamic_pointer_cast<hotkey_base>(keyboard);
			SDL_Keycode code;
			code = event.key.keysym.sym;
			keyboard->set_keycode(code);
			keyboard->set_text(SDL_GetKeyName(event.key.keysym.sym));
		}
	}
		break;
	case SDL_TEXTINPUT: {
		hotkey_keyboard_ptr keyboard(new hotkey_keyboard());
		base = std::dynamic_pointer_cast<hotkey_base>(keyboard);
		keyboard->set_text(std::string(event.text.text));
	}
		break;
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP: {
		hotkey_mouse_ptr mouse(new hotkey_mouse());
		base = std::dynamic_pointer_cast<hotkey_base>(mouse);
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
		base = std::dynamic_pointer_cast<hotkey_base>(mouse);
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
		base = std::dynamic_pointer_cast<hotkey_base>(keyboard);

		SDL_Keycode keycode = SDL_GetKeyFromName(key_cfg.c_str());
		if (keycode == SDLK_UNKNOWN) {
			ERR_G<< "Unknown key: " << key_cfg << "\n";
		}
		keyboard->set_text(key_cfg);
		keyboard->set_keycode(keycode);
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
		mods |= KMOD_GUI;
	if (cfg["alt"].to_bool())
		mods |= KMOD_ALT;

	base->set_mods(mods);
	base->set_command(cfg["command"].str());

	cfg["disabled"].to_bool() ? base->disable() : base->enable();

	return base;
}

bool hotkey_mouse::matches_helper(const SDL_Event &event) const
{
	if (event.type != SDL_MOUSEBUTTONUP && event.type != SDL_MOUSEBUTTONDOWN) {
		return false;
	}

	unsigned int mods = sdl_get_mods();
	if ((mods != mod_)) {
		return false;
	}

	if (event.button.button != button_) {
		return false;
	}

	return true;
}

const std::string hotkey_mouse::get_name_helper() const
{
	return "mouse " + std::to_string(button_);
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
	return text_;
}

bool hotkey_keyboard::matches_helper(const SDL_Event &event) const
{
	unsigned int mods = sdl_get_mods();

	if ((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) &&
				(mods & KMOD_CTRL || mods & KMOD_ALT || mods & KMOD_GUI ||
			  CKey::is_uncomposable(event.key))) {
		return event.key.keysym.sym == keycode_ && mods == mod_;
	}

	if (event.type == SDL_TEXTINPUT) {
		std::string text = std::string(event.text.text);
		boost::algorithm::to_lower(text);
		if (text == ":") {
			mods = mods & ~KMOD_SHIFT;
		}
		return text_ == text && mods == mod_;
	}

	return false;
}

bool hotkey_mouse::bindings_equal_helper(hotkey_ptr other) const
{
	hotkey_mouse_ptr other_m = std::dynamic_pointer_cast<hotkey_mouse>(other);

	if (other_m == hotkey_mouse_ptr()) {
		return false;
	}

	return button_ == other_m->button_;
}

void hotkey_keyboard::save_helper(config &item) const
{
	if (keycode_ != SDLK_UNKNOWN) {
		item["key"] = SDL_GetKeyName(keycode_);
	}
}

bool has_hotkey_item(const std::string& command)
{
	for (hotkey_ptr item : hotkeys_) {
		if (item->get_command() == command) {
			return true;
		}

	}
	return false;
}

bool hotkey_keyboard::bindings_equal_helper(hotkey_ptr other) const
{
	hotkey_keyboard_ptr other_k = std::dynamic_pointer_cast<hotkey_keyboard>(
			other);
	if (other_k == hotkey_keyboard_ptr()) {
		return false;
	}

	return keycode_ == other_k->keycode_;
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
			std::remove_if(hotkeys_.begin(), hotkeys_.end(), [item](const hotkey::hotkey_ptr& hk) { return hk->bindings_equal(item); }),
			hotkeys_.end());
	}

	hotkeys_.push_back(item);

}

void clear_hotkeys(const std::string& command)
{
	for (hotkey::hotkey_ptr item : hotkeys_) {
		if (item->get_command() == command)
		{
			if (item->is_default())
				item->disable();
			else
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
	for (hotkey_ptr item : hotkeys_) {
		if (item->matches(event)) {
			return item;
		}
	}
	return hotkey_ptr(new hotkey_void());
}

void load_hotkeys(const config& cfg, bool set_as_default)
{
	for (const config &hk : cfg.child_range("hotkey")) {

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

	for (hotkey_ptr item : hotkeys_) {
		if ((!item->is_default() && item->active()) ||
			(item->is_default() && item->is_disabled())) {
			item->save(cfg.add_child("hotkey"));
		}
	}
}

std::string get_names(std::string id)
{
	// Names are used in places like the hot-key preferences menu
	std::vector<std::string> names;
	for (const hotkey::hotkey_ptr item : hotkeys_) {
		if (item->get_command() == id && !item->null() && !item->is_disabled()) {
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

bool is_hotkeyable_event(const SDL_Event &event) {

	if (event.type == SDL_JOYBUTTONUP ||
			event.type == SDL_JOYHATMOTION ||
			event.type == SDL_MOUSEBUTTONUP) {
		return true;
	}

	unsigned mods = sdl_get_mods();

	if (mods & KMOD_CTRL || mods & KMOD_ALT || mods & KMOD_GUI) {
		return event.type == SDL_KEYUP;
	} else {
		return event.type == SDL_TEXTINPUT ||
				(event.type  == SDL_KEYUP && CKey::is_uncomposable(event.key));
	}
}

}
