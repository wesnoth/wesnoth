/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "config.hpp"
#include "display.hpp"
#include "events.hpp"
#include "hotkeys.hpp"
#include "gettext.hpp"
#include "playlevel.hpp"
#include "preferences_display.hpp"
#include "show_dialog.hpp"
#include "util.hpp"
#include "video.hpp"
#include "wesconfig.h"
#include "wml_separators.hpp"
#include "SDL.h"

#include <algorithm>
#include <cstdlib>
#include <map>

#define ERR_G LOG_STREAM(err, general)

namespace {

const struct {
	hotkey::HOTKEY_COMMAND id;
	const char* command;
	const char* description;
	bool hidden;
} hotkey_list_[] = {
	{ hotkey::HOTKEY_CYCLE_UNITS, "cycle", N_("Next Unit"), false },
	{ hotkey::HOTKEY_CYCLE_BACK_UNITS, "cycleback", N_("Previous Unit"), false },
	{ hotkey::HOTKEY_UNIT_HOLD_POSITION, "holdposition", N_("Hold Position"), false},
	{ hotkey::HOTKEY_END_UNIT_TURN, "endunitturn", N_("End Unit Turn"), false },
	{ hotkey::HOTKEY_LEADER, "leader", N_("Leader"), false },
	{ hotkey::HOTKEY_UNDO, "undo", N_("Undo"), false },
	{ hotkey::HOTKEY_REDO, "redo", N_("Redo"), false },
	{ hotkey::HOTKEY_ZOOM_IN, "zoomin", N_("Zoom In"), false },
	{ hotkey::HOTKEY_ZOOM_OUT, "zoomout", N_("Zoom Out"), false },
	{ hotkey::HOTKEY_ZOOM_DEFAULT, "zoomdefault", N_("Default Zoom"), false },
	{ hotkey::HOTKEY_FULLSCREEN, "fullscreen", N_("Toggle Full Screen"), false },
	{ hotkey::HOTKEY_SCREENSHOT, "screenshot", N_("Screenshot"), false },
	{ hotkey::HOTKEY_ACCELERATED, "accelerated", N_("Accelerated"), false },
	{ hotkey::HOTKEY_UNIT_DESCRIPTION, "describeunit", N_("Unit Description"), false },
	{ hotkey::HOTKEY_RENAME_UNIT, "renameunit", N_("Rename Unit"), false },
	{ hotkey::HOTKEY_SAVE_GAME, "save", N_("Save Game"), false },
	{ hotkey::HOTKEY_LOAD_GAME, "load", N_("Load Game"), false },
	{ hotkey::HOTKEY_RECRUIT, "recruit", N_("Recruit"), false },
	{ hotkey::HOTKEY_REPEAT_RECRUIT, "repeatrecruit", N_("Repeat Recruit"), false },
	{ hotkey::HOTKEY_RECALL, "recall", N_("Recall"), false },
	{ hotkey::HOTKEY_ENDTURN, "endturn", N_("End Turn"), false },
	{ hotkey::HOTKEY_TOGGLE_GRID, "togglegrid", N_("Toggle Grid"), false },
	{ hotkey::HOTKEY_STATUS_TABLE, "statustable", N_("Status Table"), false },
	{ hotkey::HOTKEY_MUTE, "mute", N_("Mute"), false },
	{ hotkey::HOTKEY_SPEAK, "speak", N_("Speak"), false },
	{ hotkey::HOTKEY_CREATE_UNIT, "createunit", N_("Create Unit (Debug!)"), false },
	{ hotkey::HOTKEY_CHANGE_UNIT_SIDE, "changeside", N_("Change Unit Side (Debug!)"), false },
	{ hotkey::HOTKEY_PREFERENCES, "preferences", N_("Preferences"), false },
	{ hotkey::HOTKEY_OBJECTIVES, "objectives", N_("Scenario Objectives"), false },
	{ hotkey::HOTKEY_UNIT_LIST, "unitlist", N_("Unit List"), false },
	{ hotkey::HOTKEY_STATISTICS, "statistics", N_("Statistics"), false },
	{ hotkey::HOTKEY_QUIT_GAME, "quit", N_("Quit Game"), false },
	{ hotkey::HOTKEY_LABEL_TERRAIN, "labelterrain", N_("Set Label"), false },
	{ hotkey::HOTKEY_SHOW_ENEMY_MOVES, "showenemymoves", N_("Show Enemy Moves"), false },
	{ hotkey::HOTKEY_BEST_ENEMY_MOVES, "bestenemymoves", N_("Best Possible Enemy Moves"), false },
	{ hotkey::HOTKEY_PLAY_REPLAY, "playreplay", N_("Play"), false },
	{ hotkey::HOTKEY_RESET_REPLAY, "resetreplay", N_("Reset"), false },
	{ hotkey::HOTKEY_STOP_REPLAY, "stopreplay", N_("Stop"), false },
	{ hotkey::HOTKEY_REPLAY_NEXT_TURN, "replaynextturn", N_("Next Turn"), false },
	{ hotkey::HOTKEY_REPLAY_NEXT_SIDE, "replaynextside", N_("Next Side"), false },
	{ hotkey::HOTKEY_REPLAY_SHROUD, "replayswitchshroud", N_("Shroud"), false },
	{ hotkey::HOTKEY_REPLAY_FOG, "replayswitchfog", N_("Fog"), false },
	{ hotkey::HOTKEY_REPLAY_SKIP_ANIMATION, "replayskipanimation", N_("Skip animation"), false },

	{ hotkey::HOTKEY_EDIT_SET_TERRAIN, "editsetterrain", N_("Set Terrain"),true },
	{ hotkey::HOTKEY_EDIT_QUIT, "editquit", N_("Quit Editor"),true },
	{ hotkey::HOTKEY_EDIT_NEW_MAP, "editnewmap", N_("New Map"),true },
	{ hotkey::HOTKEY_EDIT_LOAD_MAP, "editloadmap", N_("Load Map"),true },
	{ hotkey::HOTKEY_EDIT_SAVE_MAP, "editsavemap", N_("Save Map"),true },
	{ hotkey::HOTKEY_EDIT_SAVE_AS, "editsaveas", N_("Save As"),true },
	{ hotkey::HOTKEY_EDIT_SET_START_POS, "editsetstartpos", N_("Set Player's keep"),true },
	{ hotkey::HOTKEY_EDIT_FLOOD_FILL, "editfloodfill", N_("Flood Fill"),true },
	{ hotkey::HOTKEY_EDIT_FILL_SELECTION, "editfillselection", N_("Fill Selection"),true },
	{ hotkey::HOTKEY_EDIT_CUT, "editcut", N_("Cut"),true },
	{ hotkey::HOTKEY_EDIT_COPY, "editcopy", N_("Copy"),true },
	{ hotkey::HOTKEY_EDIT_PASTE, "editpaste", N_("Paste"),true },
	{ hotkey::HOTKEY_EDIT_REVERT, "editrevert", N_("Revert from Disk"),true },
	{ hotkey::HOTKEY_EDIT_RESIZE, "editresize", N_("Resize Map"),true },
	{ hotkey::HOTKEY_EDIT_FLIP, "editflip", N_("Flip Map"),true },
	{ hotkey::HOTKEY_EDIT_SELECT_ALL, "editselectall", N_("Select All"),true },
	{ hotkey::HOTKEY_EDIT_DRAW, "editdraw", N_("Draw Terrain"),true },
	{ hotkey::HOTKEY_EDIT_REFRESH, "editrefresh", N_("Refresh Image Cache"), true },

	{ hotkey::HOTKEY_DELAY_SHROUD, "delayshroud", N_("Delay Shroud Updates"), false },
	{ hotkey::HOTKEY_UPDATE_SHROUD, "updateshroud", N_("Update Shroud Now"), false },
	{ hotkey::HOTKEY_CONTINUE_MOVE, "continue", N_("Continue Move"), false },
	{ hotkey::HOTKEY_SEARCH, "search", N_("Find Label or Unit"), false },
	{ hotkey::HOTKEY_SPEAK_ALLY, "speaktoally", N_("Speak to Ally"), false },
	{ hotkey::HOTKEY_SPEAK_ALL, "speaktoall", N_("Speak to All"), false },
	{ hotkey::HOTKEY_HELP, "help", N_("Help"), false },
	{ hotkey::HOTKEY_CHAT_LOG, "chatlog", N_("View Chat Log"), false },
	{ hotkey::HOTKEY_USER_CMD, "command", N_("Enter user command"), false },

	{ hotkey::HOTKEY_LANGUAGE, "changelanguage", N_("Change the language"), true },

	{ hotkey::HOTKEY_NULL, NULL, NULL, true }
};

std::vector<hotkey::hotkey_item> hotkeys_;
hotkey::hotkey_item null_hotkey_;

}

namespace hotkey {

hotkey_item::hotkey_item(HOTKEY_COMMAND id, const std::string& command, const std::string& description, bool hidden)
	: id_(id), command_(command), description_(description), keycode_(0),
	  shift_(false), ctrl_(false), alt_(false), cmd_(false), hidden_(hidden)
{
}

void hotkey_item::load_from_config(const config& cfg)
{
	const std::string& code = cfg["key"];
	if(code.empty()) {
		keycode_ = 0;
	} else {
		keycode_ = sdl_keysym_from_name(code);
		if (keycode_ == SDLK_UNKNOWN) {
			if(code.size() >= 2  && code.size() <= 3 && tolower(code[0]) == 'f') {
				const int num = lexical_cast_default<int>(std::string(code.begin() + 1, code.end()), 1);
				keycode_ = num + SDLK_F1 - 1;
			} else if (code.size() == 1) {
				keycode_ = code[0];
			}
		}
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
			str << "cmd+";

		str << SDL_GetKeyName(SDLKey(keycode_));
		return str.str();
	} else {
		return "";
	}
}

void hotkey_item::set_description(const std::string& description)
{
	description_ = description;
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
	for (int i = 0; hotkey_list_[i].command; ++i) {
		hotkeys_.push_back(hotkey_item(hotkey_list_[i].id, hotkey_list_[i].command,
				"", hotkey_list_[i].hidden));
	}
}


manager::~manager()
{
	hotkeys_.clear();
}

void load_descriptions()
{
	for (int i = 0; hotkey_list_[i].command; ++i) {
		if (i >= hotkeys_.size()) {
			ERR_G << "Hotkey list too short: " << hotkeys_.size() << "\n";
		}
		hotkeys_[i].set_description(dsgettext(PACKAGE "-lib", hotkey_list_[i].description));
	}
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
		item["key"] = SDL_GetKeyName(SDLKey(i->get_keycode()));
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
			(event.keysym.mod & KMOD_SHIFT) != 0,
			(event.keysym.mod & KMOD_CTRL) != 0,
			(event.keysym.mod & KMOD_ALT) != 0,
			(event.keysym.mod & KMOD_LMETA) != 0
#ifdef __APPLE__
			|| (event.keysym.mod & KMOD_RMETA) != 0
#endif
			, mods);
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
		case HOTKEY_SCREENSHOT:
			disp.screenshot();
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
		case HOTKEY_CYCLE_BACK_UNITS:
			if(executor)
				executor->cycle_back_units();
			break;
		case HOTKEY_ENDTURN:
			if(executor)
				executor->end_turn();
			break;
		case HOTKEY_UNIT_HOLD_POSITION:
			if(executor)
                executor->unit_hold_position();
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
		 case HOTKEY_EDIT_REFRESH:
			if(executor)
				executor->edit_refresh();
			break;
		 case HOTKEY_LANGUAGE:
			if(executor)
				executor->change_language();
			break;
		 case HOTKEY_PLAY_REPLAY:
			 if (executor)
				 executor->play_replay();
			 break;
		 case HOTKEY_RESET_REPLAY:
			 if (executor)
				 executor->reset_replay();
			 break;
		 case HOTKEY_STOP_REPLAY:
			 if (executor)
				executor->stop_replay();
			 break;
		 case HOTKEY_REPLAY_NEXT_TURN:
			 if (executor)
				 executor->replay_next_turn();
			 break;
		 case HOTKEY_REPLAY_NEXT_SIDE:
			 if (executor)
				 executor->replay_next_side();
			 break;
		 case HOTKEY_REPLAY_SHROUD:
			 if (executor)
				 executor->replay_switch_shroud();
			 break;
		 case HOTKEY_REPLAY_FOG:
			 if (executor)
				 executor->replay_switch_fog();
			 break;
		 case HOTKEY_REPLAY_SKIP_ANIMATION:
			 if (executor)
				 executor->replay_skip_animation();
			 break;
		default:
			std::cerr << "command_executor: unknown command number " << command << ", ignoring.\n";
			break;
	}
}

void command_executor::show_menu(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu, display& gui)
{
	std::vector<std::string> items = items_arg;
	if (can_execute_command(hotkey::get_hotkey(items.front()).get_id())){
		//if just one item is passed in, that means we should execute that item
		if(items.size() == 1 && items_arg.size() == 1) {
			hotkey::execute_command(gui,hotkey::get_hotkey(items.front()).get_id(),this);
			return;
		}

		std::vector<std::string> menu = get_menu_images(items);

		static const std::string style = "menu2";
		const int res = gui::show_dialog(gui,NULL,"","",
				gui::MESSAGE,&menu,NULL,"",NULL,-1,NULL,NULL,xloc,yloc,&style);
		if (size_t(res) >= items.size())
			return;

		const hotkey::HOTKEY_COMMAND cmd = hotkey::get_hotkey(items[res]).get_id();
		hotkey::execute_command(gui,cmd,this);
	}
}

std::string command_executor::get_menu_image(hotkey::HOTKEY_COMMAND command) const {
	switch(get_action_state(command)) {
		case ACTION_ON: return game_config::checked_menu_image;
		case ACTION_OFF: return game_config::unchecked_menu_image;
		default: return get_action_image(command);
	}
}

std::vector<std::string> command_executor::get_menu_images(const std::vector<std::string>& items){
	std::vector<std::string> result;
	bool has_image = false;

	for(std::vector<std::string>::const_iterator i = items.begin(); i != items.end(); ++i) {
		const hotkey::hotkey_item hk = hotkey::get_hotkey(*i);

		std::stringstream str;
		//see if this menu item has an associated image
		std::string img(get_menu_image(hk.get_id()));
		if(img.empty() == false) {
			has_image = true;
			str << IMAGE_PREFIX << img << COLUMN_SEPARATOR;
		}

		str << hk.get_description() << COLUMN_SEPARATOR << hk.get_name();

		result.push_back(str.str());
	}
	//If any of the menu items have an image, create an image column
	if(has_image)
		for(std::vector<std::string>::iterator i = result.begin(); i != result.end(); ++i)
			if(*(i->begin()) != IMAGE_PREFIX)
				i->insert(i->begin(), COLUMN_SEPARATOR);
	return result;
}

}
