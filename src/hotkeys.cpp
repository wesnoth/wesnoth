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
#include "util.hpp"
#include "video.hpp"

#include "SDL.h"

#include <algorithm>
#include <cstdlib>
#include <map>

namespace {
	std::vector<hotkey::hotkey_item> hotkeys_;
	hotkey::hotkey_item null_hotkey_;
}

namespace hotkey {

hotkey_item::hotkey_item(HOTKEY_COMMAND id, const std::string& command, const std::string& description, bool hidden) 
	: id_(id), command_(command), description_(description), keycode_(0),
	alt_(false), ctrl_(false), shift_(false), cmd_(false), hidden_(hidden)
{
}

void hotkey_item::load_from_config(const config& cfg)
{
	const std::string& code = cfg["key"];
	if(code.empty()) {
		keycode_ = 0;
	} else if(code.size() >= 2 && tolower(code[0]) == 'f') {
		const int num = lexical_cast_default<int>(std::string(code.begin()+1,code.end()),1);
		keycode_ = num + SDLK_F1 - 1;
		std::cerr << "set key to F" << num << " = " << keycode_ << "\n";
	} else {
		keycode_ = code[0];
	}
	
	alt_ = (cfg["alt"] == "yes");
	ctrl_ = (cfg["ctrl"] == "yes");
	shift_ = (cfg["shift"] == "yes");
	cmd_ = (cfg["cmd"] == "yes");
}

std::string hotkey_item::get_name() const 
{
	if (keycode_ != 0) {
		std::stringstream str;			
		if (alt_)
			str << "alt+";
		if (ctrl_)
			str << "ctrl+";
		if (shift_)
			str << "shift+";
		if (cmd_)
			str << "command+";

		str << SDL_GetKeyName(SDLKey(keycode_));
		return str.str();
	} else {
		return "";
	}
}

void hotkey_item::set_key(int keycode, bool shift, bool ctrl, bool alt, bool cmd)
{
	keycode_ = keycode;
	shift_ = shift;
	ctrl_ = ctrl;
	alt_ = alt;
	cmd_ = cmd;
}

manager::manager() 
{
	hotkeys_.push_back(hotkey_item(HOTKEY_CYCLE_UNITS, "cycle", _("Next unit")));
	hotkeys_.push_back(hotkey_item(HOTKEY_END_UNIT_TURN, "endunitturn", _("End Unit Turn")));
	hotkeys_.push_back(hotkey_item(HOTKEY_LEADER, "leader", _("Leader")));
	hotkeys_.push_back(hotkey_item(HOTKEY_UNDO, "undo", _("Undo")));
	hotkeys_.push_back(hotkey_item(HOTKEY_REDO, "redo", _("Redo")));
	hotkeys_.push_back(hotkey_item(HOTKEY_ZOOM_IN, "zoomin", _("Zoom In")));
	hotkeys_.push_back(hotkey_item(HOTKEY_ZOOM_OUT, "zoomout", _("Zoom Out")));
	hotkeys_.push_back(hotkey_item(HOTKEY_ZOOM_DEFAULT, "zoomdefault", _("Default Zoom")));
	hotkeys_.push_back(hotkey_item(HOTKEY_FULLSCREEN, "fullscreen", _("Fullscreen")));
	hotkeys_.push_back(hotkey_item(HOTKEY_ACCELERATED, "accelerated", _("Accelerated")));
	hotkeys_.push_back(hotkey_item(HOTKEY_UNIT_DESCRIPTION, "describeunit", _("Unit Description")));
	hotkeys_.push_back(hotkey_item(HOTKEY_RENAME_UNIT, "renameunit", _("Rename Unit")));
	hotkeys_.push_back(hotkey_item(HOTKEY_SAVE_GAME, "save", _("Save Game")));
	hotkeys_.push_back(hotkey_item(HOTKEY_LOAD_GAME, "load", _("Load Game")));
	hotkeys_.push_back(hotkey_item(HOTKEY_RECRUIT, "recruit", _("Recruit")));
	hotkeys_.push_back(hotkey_item(HOTKEY_REPEAT_RECRUIT, "repeatrecruit", _("Repeat Recruit")));
	hotkeys_.push_back(hotkey_item(HOTKEY_RECALL, "recall", _("Recall")));
	hotkeys_.push_back(hotkey_item(HOTKEY_ENDTURN, "endturn", _("End Turn")));
	hotkeys_.push_back(hotkey_item(HOTKEY_TOGGLE_GRID, "togglegrid", _("Toggle Grid")));
	hotkeys_.push_back(hotkey_item(HOTKEY_STATUS_TABLE, "statustable", _("Status Table")));
	hotkeys_.push_back(hotkey_item(HOTKEY_MUTE, "mute", _("Mute")));
	hotkeys_.push_back(hotkey_item(HOTKEY_SPEAK, "speak", _("Speak")));
	hotkeys_.push_back(hotkey_item(HOTKEY_CREATE_UNIT, "createunit", _("Create Unit (Debug!)")));
	hotkeys_.push_back(hotkey_item(HOTKEY_CHANGE_UNIT_SIDE, "changeside", _("Change Unit Side (Debug!)")));
	hotkeys_.push_back(hotkey_item(HOTKEY_PREFERENCES, "preferences", _("Preferences")));
	hotkeys_.push_back(hotkey_item(HOTKEY_OBJECTIVES, "objectives", _("Scenario Objectives")));
	hotkeys_.push_back(hotkey_item(HOTKEY_UNIT_LIST, "unitlist", _("Unit List")));
	hotkeys_.push_back(hotkey_item(HOTKEY_STATISTICS, "statistics", _("Statistics")));
	hotkeys_.push_back(hotkey_item(HOTKEY_QUIT_GAME, "quit", _("Quit Game")));
	hotkeys_.push_back(hotkey_item(HOTKEY_LABEL_TERRAIN, "labelterrain", _("Set Label")));
	hotkeys_.push_back(hotkey_item(HOTKEY_SHOW_ENEMY_MOVES, "showenemymoves", _("Show Enemy Moves")));
	hotkeys_.push_back(hotkey_item(HOTKEY_BEST_ENEMY_MOVES, "bestenemymoves", _("Best Possible Enemy Moves")));

	hotkeys_.push_back(hotkey_item(HOTKEY_EDIT_SET_TERRAIN, "editsetterrain", _("Set Terrain"),true));
	hotkeys_.push_back(hotkey_item(HOTKEY_EDIT_QUIT, "editquit", _("Quit Editor"),true));
	hotkeys_.push_back(hotkey_item(HOTKEY_EDIT_NEW_MAP, "editnewmap", _("New Map"),true));
	hotkeys_.push_back(hotkey_item(HOTKEY_EDIT_LOAD_MAP, "editloadmap", _("Load Map"),true));
	hotkeys_.push_back(hotkey_item(HOTKEY_EDIT_SAVE_MAP, "editsavemap", _("Save Map"),true));
	hotkeys_.push_back(hotkey_item(HOTKEY_EDIT_SAVE_AS, "editsaveas", _("Save As"),true));
	hotkeys_.push_back(hotkey_item(HOTKEY_EDIT_SET_START_POS, "editsetstartpos", _("Set Player Start Position"),true));
	hotkeys_.push_back(hotkey_item(HOTKEY_EDIT_FLOOD_FILL, "editfloodfill", _("Flood Fill"),true));
	hotkeys_.push_back(hotkey_item(HOTKEY_EDIT_FILL_SELECTION, "editfillselection", _("Fill Selection"),true));
	hotkeys_.push_back(hotkey_item(HOTKEY_EDIT_CUT, "editcut", _("Cut"),true));
	hotkeys_.push_back(hotkey_item(HOTKEY_EDIT_COPY, "editcopy", _("Copy"),true));
	hotkeys_.push_back(hotkey_item(HOTKEY_EDIT_PASTE, "editpaste", _("Paste"),true));
	hotkeys_.push_back(hotkey_item(HOTKEY_EDIT_REVERT, "editrevert", _("Revert from Disk"),true));
	hotkeys_.push_back(hotkey_item(HOTKEY_EDIT_RESIZE, "editresize", _("Resize Map"),true));
	hotkeys_.push_back(hotkey_item(HOTKEY_EDIT_FLIP, "editflip", _("Flip Map"),true));
	hotkeys_.push_back(hotkey_item(HOTKEY_EDIT_SELECT_ALL, "editselectall", _("Select All"),true));
	hotkeys_.push_back(hotkey_item(HOTKEY_EDIT_DRAW, "editdraw", _("Draw Terrain"),true));

	hotkeys_.push_back(hotkey_item(HOTKEY_DELAY_SHROUD, "delayshroud", _("Delay Shroud Updates")));
	hotkeys_.push_back(hotkey_item(HOTKEY_UPDATE_SHROUD, "updateshroud", _("Update Shroud Now")));
	hotkeys_.push_back(hotkey_item(HOTKEY_CONTINUE_MOVE, "continue", _("Continue Move")));
	hotkeys_.push_back(hotkey_item(HOTKEY_SEARCH, "search", _("Find Label or Unit")));
	hotkeys_.push_back(hotkey_item(HOTKEY_SPEAK_ALLY, "speaktoally", _("Speak to Ally")));
	hotkeys_.push_back(hotkey_item(HOTKEY_SPEAK_ALL, "speaktoall", _("Speak to All")));
	hotkeys_.push_back(hotkey_item(HOTKEY_HELP, "help", _("Help")));
	hotkeys_.push_back(hotkey_item(HOTKEY_CHAT_LOG, "chatlog", _("View Chat Log")));
	hotkeys_.push_back(hotkey_item(HOTKEY_USER_CMD, "command", _("Enter user command")));
}

manager::~manager()
{

}

void load_hotkeys(const config& cfg)
{
	const config::child_list& children = cfg.get_children("hotkey");
	for(config::child_list::const_iterator i = children.begin(); i != children.end(); ++i) {
		hotkey_item& h = get_hotkey((**i)["command"]);

		if(h.get_id() != HOTKEY_NULL) {
			h.load_from_config(**i);
		}
	}
}

void save_hotkeys(config& cfg)
{
	cfg.clear_children("hotkey");

	for(std::vector<hotkey_item>::iterator i = hotkeys_.begin(); i != hotkeys_.end(); ++i) {
		if (i->hidden() || i->get_keycode() == 0)
			continue;

		config& item = cfg.add_child("hotkey");

		item["command"] = i->get_command();

		if(i->get_keycode() >= SDLK_F1 && i->get_keycode() <= SDLK_F12) {
			std::string str = "FF";
			str[1] = '1' + i->get_keycode() - SDLK_F1;
			item["key"] = str;
		} else {
			item["key"] = i->get_keycode();
		}

		item["alt"] = i->get_alt() ? "yes" : "no";
		item["ctrl"] = i->get_ctrl() ? "yes" : "no";
		item["shift"] = i->get_shift() ? "yes" : "no";
		item["cmd"] = i->get_cmd() ? "yes" : "no";
	}
}

hotkey_item& get_hotkey(HOTKEY_COMMAND id) 
{
	std::vector<hotkey_item>::iterator itor;

	for (itor = hotkeys_.begin(); itor != hotkeys_.end(); ++itor) {
		if (itor->get_id() == id)
			break;
	}

	if (itor == hotkeys_.end())
		return null_hotkey_;

	return *itor;
}

hotkey_item& get_hotkey(const std::string& command) 
{
	std::vector<hotkey_item>::iterator itor;

	for (itor = hotkeys_.begin(); itor != hotkeys_.end(); ++itor) {
		if (itor->get_command() == command)
			break;
	}

	if (itor == hotkeys_.end())
		return null_hotkey_;

	return *itor;
}

hotkey_item& get_hotkey(int keycode, bool shift, bool ctrl, bool alt, bool cmd, bool mods)
{

	std::vector<hotkey_item>::iterator itor;

	for (itor = hotkeys_.begin(); itor != hotkeys_.end(); ++itor) {
		if(mods) {
			if(itor->get_keycode() == keycode 
					&& (shift == itor->get_shift() || shift == true)
					&& (ctrl == itor->get_ctrl() || ctrl == true)
					&& (alt == itor->get_alt() || alt == true)
					&& (cmd == itor->get_cmd() || cmd == true))
				break;
		} else {
			if(itor->get_keycode() == keycode 
					&& shift == itor->get_shift() 
					&& ctrl == itor->get_ctrl() 
					&& alt == itor->get_alt() 
					&& cmd == itor->get_cmd())
				break;
		}
	}

	if (itor == hotkeys_.end())
		return null_hotkey_;

	return *itor;
}

hotkey_item& get_hotkey(const SDL_KeyboardEvent& event, bool mods)
{
	return get_hotkey(event.keysym.sym, 
			event.keysym.mod & KMOD_SHIFT, 
			event.keysym.mod & KMOD_CTRL, 
			event.keysym.mod & KMOD_ALT, 
			event.keysym.mod & KMOD_LMETA, 
			mods);
}

hotkey_item& get_visible_hotkey(int index)
{
	int counter = 0;

	std::vector<hotkey_item>::iterator itor;
	for (itor = hotkeys_.begin(); itor != hotkeys_.end(); ++itor) {
		if (itor->hidden())
			continue;

		if (index == counter)
			break;

		counter++;
	}

	if (itor == hotkeys_.end())
		return null_hotkey_;

	return *itor;
}

std::vector<hotkey_item>& get_hotkeys()
{
	return hotkeys_;
}

basic_handler::basic_handler(display* disp, command_executor* exec) : disp_(disp), exec_(exec) {}

void basic_handler::handle_event(const SDL_Event& event)
{
	if(event.type == SDL_KEYDOWN && disp_ != NULL) {

		//if we're in a dialog we only want to handle things that are explicitly handled
		//by the executor. If we're not in a dialog we can call the regular key event handler
		if(!gui::in_dialog()) {
			key_event(*disp_,event.key,exec_);
		} else if(exec_ != NULL) {
			key_event_execute(*disp_,event.key,exec_);
		}
	}
}


void key_event(display& disp, const SDL_KeyboardEvent& event, command_executor* executor)
{
	if(event.keysym.sym == SDLK_ESCAPE && disp.in_game()) {
		std::cerr << "escape pressed..showing quit\n";
		const int res = gui::show_dialog(disp,NULL,_("Quit"),_("Do you really want to quit?"),gui::YES_NO);
		if(res == 0) {
			throw end_level_exception(QUIT);
		} else {
			return;
		}
	}

	key_event_execute(disp,event,executor);
}

void key_event_execute(display& disp, const SDL_KeyboardEvent& event, command_executor* executor)
{
	const hotkey_item* hk = &get_hotkey(event);

	if(hk->null()) {
		//no matching hotkey was found, but try an in-exact match.
		hk = &get_hotkey(event, true);
	}

	if(hk->null())
		return;

	execute_command(disp,hk->get_id(),executor);
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
		case HOTKEY_LOAD_GAME:
			if(executor)
				executor->load_game();
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
		case HOTKEY_SPEAK_ALLY:
			if(executor) {
				preferences::set_message_private(true);
				executor->speak();
			}
			break;
		case HOTKEY_SPEAK_ALL:
			if(executor) {
				preferences::set_message_private(false);
				executor->speak();
			}
			break;
		case HOTKEY_CREATE_UNIT:
			if(executor)
				executor->create_unit();
			break;
		case HOTKEY_CHANGE_UNIT_SIDE:
			if(executor)
				executor->change_unit_side();
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
				const int res = gui::show_dialog(disp,NULL,_("Quit"),_("Do you really want to quit?"),gui::YES_NO);
				if(res == 0) {
					throw end_level_exception(QUIT);
				}
			}

			break;
		}
		case HOTKEY_HELP:
			if(executor) {
				executor->show_help();
			}
			break;
		case HOTKEY_CHAT_LOG:
			if(executor) {
				executor->show_chat_log();
			}
			break;
		case HOTKEY_USER_CMD:
			if(executor) {
				executor->user_command();
			}
			break;
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
		 case HOTKEY_EDIT_RESIZE:
			if(executor)
				executor->edit_resize();
			break;
		 case HOTKEY_EDIT_FLIP:
			if(executor)
				executor->edit_flip();
			break;
		 case HOTKEY_EDIT_SELECT_ALL:
			if(executor)
				executor->edit_select_all();
			break;
		 case HOTKEY_EDIT_DRAW:
			if(executor)
				executor->edit_draw();
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
