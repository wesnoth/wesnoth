/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "config.hpp"
#include "hotkeys.hpp"
#include "language.hpp"
#include "playlevel.hpp"
#include "preferences.hpp"
#include "show_dialog.hpp"
#include "video.hpp"

#include <algorithm>
#include <cstdlib>
#include <map>

namespace {

HOTKEY_COMMAND string_to_command(const std::string& str)
{
	static std::map<std::string,HOTKEY_COMMAND> m;
	if(m.empty()) {
		typedef std::pair<std::string,HOTKEY_COMMAND> val;
		m.insert(val("cycle",HOTKEY_CYCLE_UNITS));
		m.insert(val("endunitturn",HOTKEY_END_UNIT_TURN));
		m.insert(val("leader",HOTKEY_LEADER));
		m.insert(val("undo",HOTKEY_UNDO));
		m.insert(val("redo",HOTKEY_REDO));
		m.insert(val("zoomin",HOTKEY_ZOOM_IN));
		m.insert(val("zoomout",HOTKEY_ZOOM_OUT));
		m.insert(val("zoomdefault",HOTKEY_ZOOM_DEFAULT));
		m.insert(val("fullscreen",HOTKEY_FULLSCREEN));
		m.insert(val("accelerated",HOTKEY_ACCELERATED));
		m.insert(val("resistance",HOTKEY_ATTACK_RESISTANCE));
		m.insert(val("terraintable",HOTKEY_TERRAIN_TABLE));
		m.insert(val("describeunit",HOTKEY_UNIT_DESCRIPTION));
		m.insert(val("save",HOTKEY_SAVE_GAME));
		m.insert(val("recruit",HOTKEY_RECRUIT));
		m.insert(val("recall",HOTKEY_RECALL));
		m.insert(val("endturn",HOTKEY_ENDTURN));
	}

	const std::map<std::string,HOTKEY_COMMAND>::const_iterator i = m.find(str);
	if(i == m.end())
		return HOTKEY_NULL;
	else
		return i->second;
}

struct hotkey {
	explicit hotkey(config& cfg);

	HOTKEY_COMMAND action;
	int keycode;
	bool alt, ctrl, shift;
	mutable bool lastres;
};

hotkey::hotkey(config& cfg) : lastres(false)
{
	std::map<std::string,std::string>& m = cfg.values;
	action = string_to_command(m["command"]);

	keycode = m["key"].empty() ? 0 : m["key"][0];
	alt = (m["alt"] == "yes");
	ctrl = (m["ctrl"] == "yes");
	shift = (m["shift"] == "yes");
}

bool operator==(const hotkey& a, const hotkey& b)
{
	return a.keycode == b.keycode && a.alt == b.alt &&
	       a.ctrl == b.ctrl && a.shift == b.shift;
}

bool operator!=(const hotkey& a, const hotkey& b)
{
	return !(a == b);
}

std::vector<hotkey> hotkeys;

}

struct hotkey_pressed {
	hotkey_pressed(CKey& key);

	bool operator()(const hotkey& hk) const;

private:
	const bool shift_, ctrl_, alt_;
	CKey& key_;
};

hotkey_pressed::hotkey_pressed(CKey& key) :
     shift_(key[SDLK_LSHIFT] || key[SDLK_RSHIFT]),
     ctrl_(key[SDLK_LCTRL] || key[SDLK_RCTRL]),
     alt_(key[SDLK_LALT] || key[SDLK_RALT]), key_(key) {}

bool hotkey_pressed::operator()(const hotkey& hk) const
{
	const bool res = shift_ == hk.shift && ctrl_ == hk.ctrl &&
                     alt_ == hk.alt && key_[hk.keycode];

	//for zoom in and zoom out, allow it to happen multiple consecutive times
	if(hk.action == HOTKEY_ZOOM_IN || hk.action == HOTKEY_ZOOM_OUT) {
		hk.lastres = false;
		return res;
	}

	//don't let it return true on multiple consecutive occurrences
	if(hk.lastres) {
		hk.lastres = res;
		return false;
	} else {
		hk.lastres = res;
		return res;
	}
}

void add_hotkey(config& cfg)
{
	const hotkey new_hotkey(cfg);
	const std::vector<hotkey>::iterator i =
	               std::find(hotkeys.begin(),hotkeys.end(),new_hotkey);
	if(i != hotkeys.end()) {
		*i = new_hotkey;
	} else {
		hotkeys.push_back(new_hotkey);
	}
}

void add_hotkeys(config& cfg)
{
	std::vector<config*>& children = cfg.children["hotkey"];
	for(std::vector<config*>::iterator i = children.begin();
	    i != children.end(); ++i) {
		add_hotkey(**i);
	}
}

HOTKEY_COMMAND check_keys(display& disp)
{
	const double zoom_amount = 5.0;

	::pump_events();

	CKey key;
	if(key[KEY_ESCAPE]) {
		const int res = gui::show_dialog(disp,NULL,"",
		                   string_table["quit_message"],gui::YES_NO);
		if(res == 0) {
			throw end_level_exception(QUIT);
		}
	}

	const std::vector<hotkey>::iterator i =
	        std::find_if(hotkeys.begin(),hotkeys.end(),hotkey_pressed(key));
	if(i == hotkeys.end())
		return HOTKEY_NULL;

	switch(i->action) {
		case HOTKEY_ZOOM_IN:
			disp.zoom(zoom_amount);
			break;
		case HOTKEY_ZOOM_OUT:
			disp.zoom(-zoom_amount);
			break;
		case HOTKEY_ZOOM_DEFAULT:
			disp.default_zoom();
			break;
		case HOTKEY_FULLSCREEN:
			preferences::set_fullscreen(!preferences::fullscreen());
			break;
		case HOTKEY_ACCELERATED:
			preferences::set_turbo(!preferences::turbo());
			break;
		default:
			return i->action;
	}

	return HOTKEY_NULL;
}
