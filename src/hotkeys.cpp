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
#include "events.hpp"
#include "hotkeys.hpp"
#include "language.hpp"
#include "playlevel.hpp"
#include "preferences.hpp"
#include "show_dialog.hpp"
#include "video.hpp"

#include "SDL.h"

#include <algorithm>
#include <cstdlib>
#include <map>

namespace hotkey {

static std::map<std::string,HOTKEY_COMMAND> m;
	
	
HOTKEY_COMMAND string_to_command(const std::string& str)
{
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
		m.insert(val("renameunit",HOTKEY_RENAME_UNIT));
		m.insert(val("save",HOTKEY_SAVE_GAME));
		m.insert(val("recruit",HOTKEY_RECRUIT));
		m.insert(val("repeatrecruit",HOTKEY_REPEAT_RECRUIT));
		m.insert(val("recall",HOTKEY_RECALL));
		m.insert(val("endturn",HOTKEY_ENDTURN));
		m.insert(val("togglegrid",HOTKEY_TOGGLE_GRID));
		m.insert(val("statustable",HOTKEY_STATUS_TABLE));
		m.insert(val("mute",HOTKEY_MUTE));
		m.insert(val("speak",HOTKEY_SPEAK));
		m.insert(val("createunit",HOTKEY_CREATE_UNIT));
		m.insert(val("preferences",HOTKEY_PREFERENCES));
		m.insert(val("objectives",HOTKEY_OBJECTIVES));
		m.insert(val("unitlist",HOTKEY_UNIT_LIST));
		m.insert(val("statistics",HOTKEY_STATISTICS));
		m.insert(val("quit",HOTKEY_QUIT_GAME));
		m.insert(val("labelterrain",HOTKEY_LABEL_TERRAIN));
		m.insert(val("showenemymoves",HOTKEY_SHOW_ENEMY_MOVES));
		m.insert(val("bestenemymoves",HOTKEY_BEST_ENEMY_MOVES));
		m.insert(val("editquit",HOTKEY_EDIT_QUIT));
		m.insert(val("editnewmap",HOTKEY_EDIT_NEW_MAP));
		m.insert(val("editloadmap",HOTKEY_EDIT_LOAD_MAP));
		m.insert(val("editsavemap",HOTKEY_EDIT_SAVE_MAP));
		m.insert(val("editsaveas",HOTKEY_EDIT_SAVE_AS));
		m.insert(val("editsetstartpos",HOTKEY_EDIT_SET_START_POS));
		m.insert(val("editfloodfill",HOTKEY_EDIT_FLOOD_FILL));
		m.insert(val("editfillselection",HOTKEY_EDIT_FILL_SELECTION));
		m.insert(val("editcut",HOTKEY_EDIT_CUT));
		m.insert(val("editcopy",HOTKEY_EDIT_COPY));
		m.insert(val("editpaste",HOTKEY_EDIT_PASTE));
		m.insert(val("editrevert",HOTKEY_EDIT_REVERT));
		m.insert(val("delayshroud",HOTKEY_DELAY_SHROUD));
		m.insert(val("updateshroud",HOTKEY_UPDATE_SHROUD));
		m.insert(val("continue",HOTKEY_CONTINUE_MOVE));
		m.insert(val("search",HOTKEY_SEARCH));
	}
	
	const std::map<std::string,HOTKEY_COMMAND>::const_iterator i = m.find(str);
	if(i == m.end())
		return HOTKEY_NULL;
	else
		return i->second;
}
	
std::string command_to_string(const HOTKEY_COMMAND &command)
{
	for(std::map<std::string,HOTKEY_COMMAND>::iterator i = m.begin(); i != m.end(); ++i) {
		if(i->second == command) {
			return i->first;
		}
	}

	std::cerr << "\n command_to_string: No matching command found...";
	return "";
}


hotkey_item::hotkey_item(const config& cfg) : lastres(false)
{
	action = string_to_command(cfg["command"]);

	keycode = cfg["key"].empty() ? 0 : cfg["key"][0];
	alt = (cfg["alt"] == "yes");
	ctrl = (cfg["ctrl"] == "yes");
	shift = (cfg["shift"] == "yes");
}

bool operator==(const hotkey_item& a, const hotkey_item& b)
{
	return a.keycode == b.keycode && a.alt == b.alt &&
	       a.ctrl == b.ctrl && a.shift == b.shift;
}

bool operator!=(const hotkey_item& a, const hotkey_item& b)
{
	return !(a == b);
}

}

namespace {
std::vector<hotkey::hotkey_item> hotkeys;

}

struct hotkey_pressed {
	hotkey_pressed(const SDL_KeyboardEvent& event);

	bool operator()(const hotkey::hotkey_item& hk) const;

private:
	int keycode_;
	bool shift_, ctrl_, alt_;
};

hotkey_pressed::hotkey_pressed(const SDL_KeyboardEvent& event)
       : keycode_(event.keysym.sym), shift_(event.keysym.mod&KMOD_SHIFT),
         ctrl_(event.keysym.mod&KMOD_CTRL), alt_(event.keysym.mod&KMOD_ALT)
{}

bool hotkey_pressed::operator()(const hotkey::hotkey_item& hk) const
{
	return hk.keycode == keycode_ && shift_ == hk.shift &&
	       ctrl_ == hk.ctrl && alt_ == hk.alt;
}

namespace {

void add_hotkey(const config& cfg,bool overwrite)
{
	const hotkey::hotkey_item new_hotkey(cfg);
	for(std::vector<hotkey::hotkey_item>::iterator i = hotkeys.begin();
	    i != hotkeys.end(); ++i) {
		if(i->action == new_hotkey.action) {
		  if(overwrite)
			*i = new_hotkey;	
		  return;
		}
	}
	hotkeys.push_back(new_hotkey);
}

}

namespace hotkey {

void change_hotkey(hotkey_item& item)
{
	for(std::vector<hotkey::hotkey_item>::iterator i =hotkeys.begin();
		i!=hotkeys.end();i++)
	{
		if(item.action == i->action)
			*i = item;	
	}
}
	
basic_handler::basic_handler(display* disp) : disp_(disp) {}

void basic_handler::handle_event(const SDL_Event& event)
{
	if(event.type == SDL_KEYDOWN && !gui::in_dialog() && disp_ != NULL) {
		key_event(*disp_,event.key,NULL);
	}
}

void add_hotkeys(config& cfg,bool overwrite)
{
	const config::child_list& children = cfg.get_children("hotkey");
	for(config::child_list::const_iterator i = children.begin(); i != children.end(); ++i) {
	  add_hotkey(**i,overwrite);
	}
}

void save_hotkeys(config& cfg)
{
	const config::child_list children = cfg.get_children("hotkey");
	for(std::vector<hotkey_item>::iterator i = hotkeys.begin(); i != hotkeys.end(); ++i) {
		std::string action_name = command_to_string(i->action);

		config* item = cfg.find_child("hotkey","command",action_name);
		if(item == NULL)
			item = &cfg.add_child("hotkey");

		(*item)["command"] = action_name;
		(*item)["key"] = i->keycode;
		(*item)["alt"] = (i->alt) ? "yes" : "no";
		(*item)["ctrl"] = (i->ctrl) ? "yes" : "no";
		(*item)["shift"] = (i->shift) ? "yes" : "no";
	}
}

std::vector<hotkey_item>& get_hotkeys()
{
	return hotkeys;
}

std::string get_hotkey_name(hotkey_item i)
{
 	std::stringstream str;			
	if (i.alt)
 	str << "alt+";
	if (i.ctrl)
	str << "ctrl+";
 	if (i.shift)
	str << "shift+";
	str << SDL_GetKeyName(SDLKey(i.keycode));
	return str.str();
}

void key_event(display& disp, const SDL_KeyboardEvent& event, command_executor* executor)
{
	if(event.keysym.sym == SDLK_ESCAPE && disp.in_game()) {
		std::cerr << "escape pressed..showing quit\n";
		const int res = gui::show_dialog(disp,NULL,"",string_table["quit_message"],gui::YES_NO);
		if(res == 0) {
			throw end_level_exception(QUIT);
		} else {
			return;
		}
	}

	const std::vector<hotkey_item>::iterator i = std::find_if(hotkeys.begin(),hotkeys.end(),hotkey_pressed(event));

	if(i == hotkeys.end())
		return;

	execute_command(disp,i->action,executor);
}

void execute_command(display& disp, HOTKEY_COMMAND command, command_executor* executor)
{
	const int zoom_amount = 4;

	if(executor != NULL && executor->can_execute_command(command) == false)
		return;

	switch(command) {
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
		case HOTKEY_MUTE:
			preferences::mute(!preferences::is_muted());
			break;
		case HOTKEY_CYCLE_UNITS:
			if(executor)
				executor->cycle_units();
			break;
		case HOTKEY_ENDTURN:
			if(executor)
				executor->end_turn();
			break;
		case HOTKEY_END_UNIT_TURN:
			if(executor)
				executor->end_unit_turn();
			break;
		case HOTKEY_LEADER:
			if(executor)
				executor->goto_leader();
			break;
		case HOTKEY_UNDO:
			if(executor)
				executor->undo();
			break;
		case HOTKEY_REDO:
			if(executor)
				executor->redo();
			break;
		case HOTKEY_TERRAIN_TABLE:
			if(executor)
				executor->terrain_table();
			break;
		case HOTKEY_ATTACK_RESISTANCE:
			if(executor)
				executor->attack_resistance();
			break;
		case HOTKEY_UNIT_DESCRIPTION:
			if(executor)
				executor->unit_description();
			break;
		case HOTKEY_RENAME_UNIT:
			if(executor)
				executor->rename_unit();
			break;
		case HOTKEY_SAVE_GAME:
			if(executor)
				executor->save_game();
			break;
		case HOTKEY_TOGGLE_GRID:
			if(executor)
				executor->toggle_grid();
			break;
		case HOTKEY_STATUS_TABLE:
			if(executor)
				executor->status_table();
			break;
		case HOTKEY_RECALL:
			if(executor)
				executor->recall();
			break;
		case HOTKEY_RECRUIT:
			if(executor)
				executor->recruit();
			break;
		case hotkey::HOTKEY_REPEAT_RECRUIT:
			if(executor)
				executor->repeat_recruit();
			break;
		case HOTKEY_SPEAK:
			if(executor)
				executor->speak();
			break;
		case HOTKEY_CREATE_UNIT:
			if(executor)
				executor->create_unit();
			break;
		case HOTKEY_PREFERENCES:
			if(executor)
				executor->preferences();
			break;
		case HOTKEY_OBJECTIVES:
			if(executor)
				executor->objectives();
			break;
		case HOTKEY_UNIT_LIST:
			if(executor)
				executor->unit_list();
			break;
		case HOTKEY_STATISTICS:
			if(executor)
				executor->show_statistics();
			break;
		case HOTKEY_LABEL_TERRAIN:
			if(executor)
				executor->label_terrain();
			break;
		case HOTKEY_SHOW_ENEMY_MOVES:
			if(executor)
				executor->show_enemy_moves(false);
			break;
		case HOTKEY_BEST_ENEMY_MOVES:
			if(executor)
				executor->show_enemy_moves(true);
			break;
		case HOTKEY_DELAY_SHROUD:
			if(executor)
				executor->toggle_shroud_updates();
			break;
		case HOTKEY_UPDATE_SHROUD:
			if(executor)
				executor->update_shroud_now();
			break;
		case HOTKEY_CONTINUE_MOVE:
			if(executor)
				executor->continue_move();
			break;
		case HOTKEY_SEARCH:
			if(executor)
				executor->search();
			break;
		case HOTKEY_QUIT_GAME: {
			if(disp.in_game()) {
				std::cerr << "is in game -- showing quit message\n";
				const int res = gui::show_dialog(disp,NULL,"",string_table["quit_message"],gui::YES_NO);
				if(res == 0) {
					throw end_level_exception(QUIT);
				}
			}

			break;
		}

		case HOTKEY_EDIT_SET_TERRAIN:
			if(executor)
				executor->edit_set_terrain();
			break;
		case HOTKEY_EDIT_QUIT:
			if(executor)
				executor->edit_quit();
			break;
		 case HOTKEY_EDIT_NEW_MAP:
			if(executor)
				executor->edit_new_map();
			break;
		 case HOTKEY_EDIT_LOAD_MAP:
			if(executor)
				executor->edit_load_map();
			break;
		 case HOTKEY_EDIT_SAVE_MAP:
			if(executor)
				executor->edit_save_map();
			break;
		 case HOTKEY_EDIT_SAVE_AS:
			if(executor)
				executor->edit_save_as();
			break;
		 case HOTKEY_EDIT_SET_START_POS:
			if(executor)
				executor->edit_set_start_pos();
			break;
		 case HOTKEY_EDIT_FLOOD_FILL:
			if(executor)
				executor->edit_flood_fill();
			break;

		 case HOTKEY_EDIT_FILL_SELECTION:
			if(executor)
				executor->edit_fill_selection();
			break;
		 case HOTKEY_EDIT_CUT:
			if(executor)
				executor->edit_cut();
			break;
		 case HOTKEY_EDIT_PASTE:
			if(executor)
				executor->edit_paste();
			break;
		 case HOTKEY_EDIT_COPY:
			if(executor)
				executor->edit_copy();
			break;
		 case HOTKEY_EDIT_REVERT:
			if(executor)
				executor->edit_revert();
			break;
		default:
			std::cerr << "command_executor: unknown command number " << command << ", ignoring.\n";
			break;
	}
}

std::string command_executor::get_menu_image(hotkey::HOTKEY_COMMAND command) const {
	switch(get_action_state(command)) {
		case ACTION_ON: return game_config::checked_menu_image;
		case ACTION_OFF: return game_config::unchecked_menu_image;
		default: return get_action_image(command);
	}
}

}
