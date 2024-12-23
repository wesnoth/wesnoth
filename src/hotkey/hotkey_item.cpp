/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "hotkey/hotkey_item.hpp"

#include "config.hpp"
#include "game_config_view.hpp"
#include "hotkey/hotkey_command.hpp"
#include "key.hpp"
#include "log.hpp"
#include "sdl/input.hpp" // for sdl::get_mods
#include "serialization/unicode.hpp"
#include "utils/general.hpp"

#include <boost/algorithm/string.hpp>

#include <functional>

static lg::log_domain log_config("config");
#define ERR_G LOG_STREAM(err, lg::general())
#define LOG_G LOG_STREAM(info, lg::general())
#define DBG_G LOG_STREAM(debug, lg::general())
#define ERR_CF LOG_STREAM(err, log_config)

namespace hotkey
{
namespace
{
hotkey_list hotkeys_;
game_config_view default_hotkey_cfg_;

const int TOUCH_MOUSE_INDEX = 255;
}; // namespace

const std::string hotkey_base::get_name() const
{
	std::string ret = "";

	if(mod_ & KMOD_CTRL) {
		ret += "ctrl";
	}

	ret += (!ret.empty() && !boost::algorithm::ends_with(ret, "+") ? "+" : "");
	if(mod_ & KMOD_ALT) {
#ifdef __APPLE__
		ret += "opt";
#else
		ret += "alt";
#endif
	}

	ret += (!ret.empty() && !boost::algorithm::ends_with(ret, "+") ? "+" : "");
	if(mod_ & KMOD_SHIFT) {
		ret += "shift";
	}

	ret += (!ret.empty() && !boost::algorithm::ends_with(ret, "+") ? "+" : "");
	if(mod_ & KMOD_GUI) {
#ifdef __APPLE__
		ret += "cmd";
#else
		ret += "win";
#endif
	}

	ret += (!ret.empty() && !boost::algorithm::ends_with(ret, "+") ? "+" : "");
	return ret += get_name_helper();
}

bool hotkey_base::bindings_equal(const hotkey_ptr& other)
{
	if(other == nullptr) {
		return false;
	}

	const hk_scopes scopematch =
		hotkey::get_hotkey_command(get_command()).scope &
		hotkey::get_hotkey_command(other->get_command()).scope;

	if(scopematch.none()) {
		return false;
	}

	return mod_ == other->mod_ && bindings_equal_helper(other);
}

bool hotkey_base::matches(const SDL_Event& event) const
{
	if(!hotkey::is_scope_active(hotkey::get_hotkey_command(get_command()).scope) || !active() || is_disabled()) {
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

hotkey_ptr create_hotkey(const std::string& id, const SDL_Event& event)
{
	hotkey_ptr base = std::make_shared<hotkey_void>();
	const hotkey_command& command = get_hotkey_command(id);
	unsigned mods = sdl::get_mods();

	switch(event.type) {
	case SDL_KEYUP: {
		if(mods & KMOD_CTRL || mods & KMOD_ALT || mods & KMOD_GUI || CKey::is_uncomposable(event.key)
			|| command.toggle) {
			auto keyboard = std::make_shared<hotkey_keyboard>();
			base = std::dynamic_pointer_cast<hotkey_base>(keyboard);
			SDL_Keycode code;
			code = event.key.keysym.sym;
			keyboard->set_keycode(code);
			keyboard->set_text(SDL_GetKeyName(event.key.keysym.sym));
		}
	} break;

	case SDL_TEXTINPUT: {
		if(command.toggle) {
			return nullptr;
		}
		auto keyboard = std::make_shared<hotkey_keyboard>();
		base = std::dynamic_pointer_cast<hotkey_base>(keyboard);
		std::string text = std::string(event.text.text);
		keyboard->set_text(text);
		if(text == ":" || text == "`") {
			mods = mods & ~KMOD_SHIFT;
		}
	} break;

	case SDL_MOUSEBUTTONUP: {
		auto mouse = std::make_shared<hotkey_mouse>();
		base = std::dynamic_pointer_cast<hotkey_base>(mouse);
		mouse->set_button(event.button.button);
		break;
	}

	default:
		ERR_G << "Trying to bind an unknown event type:" << event.type;
		break;
	}

	base->set_mods(mods);
	base->set_command(id);
	base->unset_default();

	return base;
}

hotkey_ptr load_from_config(const config& cfg)
{
	hotkey_ptr base = std::make_shared<hotkey_void>();

	const config::attribute_value& mouse_cfg = cfg["mouse"];
	if(!mouse_cfg.empty()) {
		auto mouse = std::make_shared<hotkey_mouse>();
		base = std::dynamic_pointer_cast<hotkey_base>(mouse);
		if(mouse_cfg.to_int() == TOUCH_MOUSE_INDEX) {
			mouse->set_button(TOUCH_MOUSE_INDEX);
		} else {
			mouse->set_button(cfg["button"].to_int());
		}
	}

	const std::string& key_cfg = cfg["key"];
	if(!key_cfg.empty()) {
		auto keyboard = std::make_shared<hotkey_keyboard>();
		base = std::dynamic_pointer_cast<hotkey_base>(keyboard);

		SDL_Keycode keycode = SDL_GetKeyFromName(key_cfg.c_str());
		if(keycode == SDLK_UNKNOWN) {
			ERR_G << "Unknown key: " << key_cfg;
		}
		keyboard->set_text(key_cfg);
		keyboard->set_keycode(keycode);
	}

	if(base == hotkey_ptr()) {
		return base;
	}

	unsigned int mods = 0;

	if(cfg["shift"].to_bool())
		mods |= KMOD_SHIFT;
	if(cfg["ctrl"].to_bool())
		mods |= KMOD_CTRL;
	if(cfg["cmd"].to_bool())
		mods |= KMOD_GUI;
	if(cfg["alt"].to_bool())
		mods |= KMOD_ALT;

	base->set_mods(mods);
	base->set_command(cfg["command"].str());

	cfg["disabled"].to_bool() ? base->disable() : base->enable();

	return base;
}

bool hotkey_mouse::matches_helper(const SDL_Event& event) const
{
	if(event.type != SDL_MOUSEBUTTONUP && event.type != SDL_MOUSEBUTTONDOWN && event.type != SDL_FINGERDOWN
		&& event.type != SDL_FINGERUP) {
		return false;
	}

	unsigned mods = sdl::get_mods();
	if((mods != mod_)) {
		return false;
	}

	if(event.button.which == SDL_TOUCH_MOUSEID) {
		return button_ == TOUCH_MOUSE_INDEX;
	}

	return event.button.button == button_;
}

const std::string hotkey_mouse::get_name_helper() const
{
	return "mouse " + std::to_string(button_);
}

void hotkey_mouse::save_helper(config& item) const
{
	item["mouse"] = 0;
	if(button_ != 0) {
		item["button"] = button_;
	}
}

void hotkey_keyboard::set_text(const std::string& text)
{
	text_ = text;
	boost::algorithm::to_lower(text_);
}

const std::string hotkey_keyboard::get_name_helper() const
{
	return text_;
}

bool hotkey_keyboard::matches_helper(const SDL_Event& event) const
{
	unsigned mods = sdl::get_mods();
	const hotkey_command& command = get_hotkey_command(get_command());

	if((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
		&& (mods & KMOD_CTRL || mods & KMOD_ALT || mods & KMOD_GUI || command.toggle || CKey::is_uncomposable(event.key))
	) {
		return event.key.keysym.sym == keycode_ && mods == mod_;
	}

	if(event.type == SDL_TEXTINPUT && !command.toggle) {
		std::string text = std::string(event.text.text);
		boost::algorithm::to_lower(text);
		if(text == ":" || text == "`") {
			mods = mods & ~KMOD_SHIFT;
		}
		return text_ == text && utf8::size(std::string(event.text.text)) == 1 && mods == mod_;
	}

	return false;
}

bool hotkey_mouse::bindings_equal_helper(hotkey_ptr other) const
{
	auto other_m = std::dynamic_pointer_cast<hotkey_mouse>(other);
	if(other_m == nullptr) {
		return false;
	}

	return button_ == other_m->button_;
}

void hotkey_keyboard::save_helper(config& item) const
{
	if(!text_.empty()) {
		item["key"] = text_;
	}
}

bool has_hotkey_item(const std::string& command)
{
	for(const hotkey_ptr& item : hotkeys_) {
		if(item->get_command() == command) {
			return true;
		}
	}
	return false;
}

bool hotkey_keyboard::bindings_equal_helper(hotkey_ptr other) const
{
	auto other_k = std::dynamic_pointer_cast<hotkey_keyboard>(other);
	if(other_k == nullptr) {
		return false;
	}

	return text_ == other_k->text_;
}

void del_hotkey(const hotkey_ptr& item)
{
	if(!hotkeys_.empty()) {
		utils::erase(hotkeys_, item);
	}
}

void add_hotkey(hotkey_ptr item)
{
	if(item) {
		auto iter = std::find_if(hotkeys_.begin(), hotkeys_.end(),
			[&item](const hotkey::hotkey_ptr& hk) { return hk->bindings_equal(item); });

		if(iter != hotkeys_.end()) {
			iter->swap(item);
		} else {
			hotkeys_.push_back(std::move(item));
		}
	}
}

void clear_hotkeys(const std::string& command)
{
	for(hotkey::hotkey_ptr& item : hotkeys_) {
		if(item->get_command() == command) {
			if(item->is_default()) {
				item->disable();
			} else {
				item->clear();
			}
		}
	}
}

void clear_hotkeys()
{
	hotkeys_.clear();
}

const hotkey_ptr get_hotkey(const SDL_Event& event)
{
	for(const hotkey_ptr& item : hotkeys_) {
		if(item->matches(event)) {
			return item;
		}
	}
	return std::make_shared<hotkey_void>();
}

void load_default_hotkeys(const game_config_view& cfg)
{
	hotkey_list new_hotkeys;
	for(const config& hk : cfg.child_range("hotkey")) {
		if(hotkey_ptr item = load_from_config(hk); !item->null()) {
			new_hotkeys.push_back(std::move(item));
		}
	}

	default_hotkey_cfg_ = cfg;
	hotkeys_.swap(new_hotkeys);
}

void load_custom_hotkeys(const game_config_view& cfg)
{
	for(const config& hk : cfg.child_range("hotkey")) {
		if(hotkey_ptr item = load_from_config(hk); !item->null()) {
			item->unset_default();
			add_hotkey(item);
		}
	}
}

void reset_default_hotkeys()
{
	hotkeys_.clear();

	if(!default_hotkey_cfg_.child_range("hotkey").empty()) {
		load_default_hotkeys(default_hotkey_cfg_);
	} else {
		ERR_G << "no default hotkeys set yet; all hotkeys are now unassigned!";
	}
}

const hotkey_list& get_hotkeys()
{
	return hotkeys_;
}

void save_hotkeys(config& cfg)
{
	cfg.clear_children("hotkey");

	for(hotkey_ptr& item : hotkeys_) {
		if((!item->is_default() && item->active()) || (item->is_default() && item->is_disabled())) {
			item->save(cfg.add_child("hotkey"));
		}
	}
}

std::string get_names(const std::string& id)
{
	// Names are used in places like the hot-key preferences menu
	std::vector<std::string> names;
	for(const hotkey::hotkey_ptr& item : hotkeys_) {
		if(item->get_command() == id && !item->null() && !item->is_disabled()) {
			names.push_back(item->get_name());
		}
	}

	// These are hard-coded, non-rebindable hotkeys
	if(id == "quit") {
		names.push_back("escape");
	} else if(id == "quit-to-desktop") {
#ifdef __APPLE__
		names.push_back("cmd+q");
#else
		names.push_back("alt+F4");
#endif
	}

	return boost::algorithm::join(names, ", ");
}

bool is_hotkeyable_event(const SDL_Event& event)
{
	if(event.type == SDL_JOYBUTTONUP || event.type == SDL_JOYHATMOTION || event.type == SDL_MOUSEBUTTONUP) {
		return true;
	}

	unsigned mods = sdl::get_mods();

	if(mods & KMOD_CTRL || mods & KMOD_ALT || mods & KMOD_GUI) {
		return event.type == SDL_KEYUP;
	} else {
		return event.type == SDL_TEXTINPUT || event.type == SDL_KEYUP;
	}
}

} // namespace hotkey
