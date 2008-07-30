/*
   Copyright (C) 2008 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "action.hpp"
#include "editor_controller.hpp"
#include "editor_display.hpp"
#include "editor_layout.hpp"
#include "editor_map.hpp"
#include "editor_palettes.hpp"
#include "mouse_action.hpp"

#include "gui/dialogs/editor_new_map.hpp"
#include "gui/widgets/button.hpp"

#include "../config_adapter.hpp"
#include "../construct_dialog.hpp"
#include "../cursor.hpp"
#include "../filechooser.hpp"
#include "../filesystem.hpp"
#include "../font.hpp"
#include "../foreach.hpp"
#include "../gettext.hpp"
#include "../hotkeys.hpp"
#include "../preferences.hpp"
#include "../wml_exception.hpp"

#include "SDL.h"

namespace editor2 {

editor_controller::editor_controller(const config &game_config, CVideo& video)
: controller_base(SDL_GetTicks(), game_config, video)
, mouse_handler_base(get_map())
, map_context_(editor_map(game_config, 44, 33, t_translation::GRASS_LAND))
, gui_(NULL), do_quit_(false), quit_mode_(EXIT_ERROR)
{
	init(video);
	floating_label_manager_ = new font::floating_label_context();
	size_specs_ = new size_specs();
	adjust_sizes(gui(), *size_specs_);
	palette_ = new terrain_palette(gui(), *size_specs_, get_map(), game_config,
		foreground_terrain_, background_terrain_);
	//brush_bar_ = new brush_bar(gui(), *size_specs_);
	
	brushes_.push_back(brush());
	brushes_[0].add_relative_location(0, 0);
	const config::child_list& children = game_config.get_children("brush");
	foreach (const config* i, game_config.get_children("brush")) {
		brushes_.push_back(brush(*i));
	}
	if (brushes_.size() == 1) {
		WRN_ED << "No brushes defined!";
	}
	brush_ = &brushes_[0];
	mouse_actions_.insert(std::make_pair(hotkey::HOTKEY_EDITOR_TOOL_PAINT, 
		new mouse_action_paint(foreground_terrain_, &brush_)));
	mouse_actions_.insert(std::make_pair(hotkey::HOTKEY_EDITOR_TOOL_FILL, 
		new mouse_action_fill(foreground_terrain_)));
	mouse_actions_.insert(std::make_pair(hotkey::HOTKEY_EDITOR_TOOL_SELECT, 
		new mouse_action_select(&brush_)));
	mouse_actions_.insert(std::make_pair(hotkey::HOTKEY_EDITOR_PASTE,
		new mouse_action_paste(clipboard_)));
	hotkey_set_mouse_action(hotkey::HOTKEY_EDITOR_TOOL_PAINT);	
	get_map_context().set_starting_position_labels(gui());
	cursor::set(cursor::NORMAL);
	gui_->invalidate_game_status();
	palette_->adjust_size();
	refresh_all();
	gui_->draw();
	events::raise_draw_event();	
}

void editor_controller::init(CVideo& video)
{
	config dummy;
	const config* theme_cfg = get_theme(game_config_, "editor2");
	theme_cfg = theme_cfg ? theme_cfg : &dummy;
	gui_ = new editor_display(video, get_map(), *theme_cfg, game_config_, config());
	gui_->set_grid(preferences::grid());
	prefs_disp_manager_ = new preferences::display_manager(gui_);
}

editor_controller::~editor_controller()
{
	delete palette_;
	delete size_specs_;
	delete floating_label_manager_;
    delete gui_;
	typedef std::pair<hotkey::HOTKEY_COMMAND, mouse_action*> apr;
	foreach (apr a, mouse_actions_) {
		delete a.second;
	}	
	delete prefs_disp_manager_;
}

EXIT_STATUS editor_controller::main_loop()
{
	while (!do_quit_) {
		play_slice();
	}
	return quit_mode_;
}

void editor_controller::quit_confirm(EXIT_STATUS mode)
{
	std::string message = _("Do you really want to quit?");
	if (get_map_context().modified()) {
		message += " ";
		message += _("There are unsaved changes in the map.");
	}
	int res = gui::dialog(gui(),_("Quit"),message,gui::YES_NO).show();
	if (res == 0) {
		do_quit_ = true;
		quit_mode_ = mode;
	}
}

bool editor_controller::confirm_discard()
{
	if (get_map_context().modified()) {
		return !gui::dialog(gui(), _("There are unsaved changes in the map"),
			_("Do you want to discard all changes you made te the map?"), gui::YES_NO).show();
	} else {
		return true;
	}
}

void editor_controller::load_map_dialog()
{
	if (!confirm_discard()) return;
	std::string fn = get_map_context().get_filename();
	if (fn.empty()) {
		fn = get_dir(get_dir(get_user_data_dir() + "/editor") + "/maps");
	}
	int res = dialogs::show_file_chooser_dialog(gui(), fn, _("Choose a Map to Load"));
	if (res == 0) {
		load_map(fn);
	}
}

void editor_controller::new_map_dialog()
{
	if (!confirm_discard()) return;
	gui2::teditor_new_map dialog;;
	dialog.set_map_width(get_map().total_width());
	dialog.set_map_height(get_map().total_height());
	
	dialog.show(gui().video());
	int res = dialog.get_retval();
	if(res == gui2::tbutton::OK) {
		int w = dialog.map_width();
		int h = dialog.map_height();
		t_translation::t_terrain fill = t_translation::GRASS_LAND;
		new_map(w, h, fill);
	}
}

void editor_controller::save_map_as_dialog()
{
	const std::string default_dir =	get_dir(get_dir(get_user_data_dir() + "/editor") + "/maps");
	std::string input_name = get_map_context().get_filename();
	if (input_name.empty()) {
		input_name = default_dir;
	}
	const std::string old_input_name = input_name;

	int res = 0;
	int overwrite_res = 1;
	do {
		input_name = old_input_name;
		res = dialogs::show_file_chooser_dialog(gui(), input_name, _("Save the Map As"));
		if (res == 0) {
			if (file_exists(input_name)) {
				overwrite_res = gui::dialog(gui(), "",
					_("The map already exists. Do you want to overwrite it?"),
					gui::YES_NO).show();
			} else {
				overwrite_res = 0;
			}
		} else {
			return; //cancel pressed
		}
	} while (overwrite_res != 0);

	save_map_as(input_name);
}

bool editor_controller::save_map_as(const std::string& filename)
{
	std::string old_filename = get_map_context().get_filename();
	get_map_context().set_filename(filename);
	if (!save_map(true)) {
		get_map_context().set_filename(old_filename);
		return false;
	} else {
		return true;
	}
}

bool editor_controller::save_map(bool display_confirmation)
{
	try {
		get_map_context().save();
		if (display_confirmation) {
			gui::message_dialog(gui(), "", _("Map saved.")).show();
		}
	} catch (io_exception& e) {
		utils::string_map symbols;
		symbols["msg"] = e.what();
		const std::string msg = vgettext("Could not save the map: $msg", symbols);
		gui::message_dialog(gui(), "", msg).show();
		return false;
	}
	return true;
}

void editor_controller::load_map(const std::string& filename)
{
	std::string map_string = read_file(filename);
	try {
		editor_map new_map(game_config_, map_string);
		get_map_context().set_filename(filename);
		set_map(new_map);
		//TODO when this fails see if it's a scenario with a mapdata= key and give
		//the user an option of loading that map instead of just failing
	} catch (gamemap::incorrect_format_exception& e) {
		std::string message = "There was an error while loading the map: \n";
		message += e.msg_;
		gui::message_dialog(gui(), "Error loading map (format)", message).show();
		return;
	} catch (twml_exception& e) {
		std::string message = "There was an error while loading the map: \n";
		message += e.user_message;
		gui::message_dialog(gui(), "Error loading map (wml)", message).show();
		return;
	}
}

void editor_controller::revert_map()
{
	if (!confirm_discard()) return;
	const std::string& filename = get_map_context().get_filename();
	if (filename.empty()) {
		ERR_ED << "Empty filename in map revert\n";
		return;
	}
	load_map(filename);
}

void editor_controller::new_map(int width, int height, t_translation::t_terrain fill)
{
	set_map(editor_map(game_config_, width, height, fill));
}

void editor_controller::set_map(const editor_map& map)
{
	get_map_context().clear_starting_position_labels(gui());
	get_map() = map;
	gui().reload_map();
	get_map_context().set_starting_position_labels(gui());
	refresh_all();
}

void editor_controller::reload_map()
{
	get_map_context().clear_starting_position_labels(gui());
	gui().reload_map();
	get_map_context().set_starting_position_labels(gui());
	refresh_all();
}


bool editor_controller::can_execute_command(hotkey::HOTKEY_COMMAND command, int /*index*/) const
{
	using namespace hotkey; //reduce hotkey:: clutter
	switch (command) {
		case HOTKEY_ZOOM_IN:
		case HOTKEY_ZOOM_OUT:
		case HOTKEY_ZOOM_DEFAULT:
		case HOTKEY_FULLSCREEN:
		case HOTKEY_SCREENSHOT:
		case HOTKEY_MAP_SCREENSHOT:
		case HOTKEY_TOGGLE_GRID:
		case HOTKEY_MOUSE_SCROLL:
		case HOTKEY_MUTE:
		case HOTKEY_PREFERENCES:
		case HOTKEY_HELP:
		case HOTKEY_QUIT_GAME:
			return true; //general hotkeys we can always do
		case HOTKEY_UNDO:
			return true;
		case HOTKEY_REDO:
			return true;
		case HOTKEY_EDITOR_QUIT_TO_DESKTOP:
		case HOTKEY_EDITOR_MAP_NEW:
		case HOTKEY_EDITOR_MAP_LOAD:
		case HOTKEY_EDITOR_MAP_SAVE_AS:
		case HOTKEY_EDITOR_BRUSH_NEXT:
		case HOTKEY_EDITOR_TOOL_NEXT:
		case HOTKEY_EDITOR_TERRAIN_PALETTE_SWAP:
			return true; //editor hotkeys we can always do
		case HOTKEY_EDITOR_MAP_SAVE:
			return true;
		case HOTKEY_EDITOR_MAP_REVERT:
			return !get_map_context().get_filename().empty();
			return true;
		case HOTKEY_EDITOR_TOOL_PAINT:
		case HOTKEY_EDITOR_TOOL_FILL:
		case HOTKEY_EDITOR_TOOL_SELECT:
		case HOTKEY_EDITOR_TOOL_STARTING_POSITION:
			return true; //tool selection always possible
		case HOTKEY_EDITOR_CUT:
		case HOTKEY_EDITOR_COPY:
		case HOTKEY_EDITOR_SELECTION_ROTATE:
		case HOTKEY_EDITOR_SELECTION_FLIP:
		case HOTKEY_EDITOR_SELECTION_GENERATE:
		case HOTKEY_EDITOR_SELECTION_RANDOMIZE:
			return !get_map().selection().empty();
		case HOTKEY_EDITOR_PASTE:
			return !clipboard_.empty();
		case HOTKEY_EDITOR_SELECT_ALL:		
		case HOTKEY_EDITOR_MAP_RESIZE:
		case HOTKEY_EDITOR_MAP_ROTATE:
		case HOTKEY_EDITOR_MAP_FLIP_X:
		case HOTKEY_EDITOR_MAP_FLIP_Y:
		case HOTKEY_EDITOR_MAP_GENERATE:
		case HOTKEY_EDITOR_REFRESH:
		case HOTKEY_EDITOR_UPDATE_TRANSITIONS:
		case HOTKEY_EDITOR_AUTO_UPDATE_TRANSITIONS:
			return true;
		default:
			return false;
	}
}

hotkey::ACTION_STATE editor_controller::get_action_state(hotkey::HOTKEY_COMMAND command) const {
	using namespace hotkey;
	switch (command) {
		case HOTKEY_EDITOR_TOOL_PAINT:
		case HOTKEY_EDITOR_TOOL_FILL:
		case HOTKEY_EDITOR_TOOL_SELECT:
//		case HOTKEY_EDITOR_TOOL_STARTING_POSITION:
			return is_mouse_action_set(command) ? ACTION_ON : ACTION_OFF;
		default:
			return command_executor::get_action_state(command);
	}
}

bool editor_controller::execute_command(hotkey::HOTKEY_COMMAND command, int index)
{
	SCOPE_ED;
	using namespace hotkey;
	switch (command) {
		case HOTKEY_QUIT_GAME:
			quit_confirm(EXIT_NORMAL);
			return true;
		case HOTKEY_EDITOR_QUIT_TO_DESKTOP:
			quit_confirm(EXIT_QUIT_TO_DESKTOP);
			return true;
		case HOTKEY_EDITOR_TERRAIN_PALETTE_SWAP:
			palette_->swap();
			return true;
		case HOTKEY_EDITOR_TOOL_PAINT:
		case HOTKEY_EDITOR_TOOL_FILL:
		case HOTKEY_EDITOR_TOOL_SELECT:
		case HOTKEY_EDITOR_PASTE:
//		case HOTKEY_EDITOR_TOOL_STARTING_POSITION:
			hotkey_set_mouse_action(command);
			return true;
		case HOTKEY_EDITOR_BRUSH_NEXT:
			cycle_brush();
			return true;
		case HOTKEY_EDITOR_COPY:
			copy_selection();
			return true;
		case HOTKEY_EDITOR_CUT:
			cut_selection();
			return true;
		case HOTKEY_EDITOR_SELECT_ALL:
			if (!get_map_context().get_map().everything_selected()) {
				get_map_context().perform_action(editor_action_select_all());
				refresh_after_action();
				return true;
			} //else intentionally fall through
		case HOTKEY_EDITOR_SELECT_INVERSE:
			get_map_context().perform_action(editor_action_select_inverse());
			refresh_after_action();
			return true;
		case HOTKEY_EDITOR_MAP_FLIP_X: {
			editor_action_flip_x fx;
			get_map_context().perform_action(fx);
			refresh_after_action();
			return true;
			}
		case HOTKEY_EDITOR_MAP_FLIP_Y: {
			LOG_ED << "FlipYhk\n";
			editor_action_flip_y fy;
			get_map_context().perform_action(fy);
			refresh_after_action();
			return true;
			}
		case HOTKEY_EDITOR_MAP_LOAD:
			load_map_dialog();
			return true;
		case HOTKEY_EDITOR_MAP_REVERT:
			revert_map();
			return true;
		case HOTKEY_EDITOR_MAP_NEW:
			new_map_dialog();
			return true;
		case HOTKEY_EDITOR_MAP_SAVE:
			if (get_map_context().get_filename().empty()) {
				save_map_as_dialog();
			} else {
				save_map();
			}
			return true;
		case HOTKEY_EDITOR_MAP_SAVE_AS:
			save_map_as_dialog();
			return true;
		default:
			return controller_base::execute_command(command, index);
	}
}

void editor_controller::expand_starting_position_menu(std::vector<std::string>& items)
{
	for (unsigned int i = 0; i < items.size(); ++i) {
		if (items[i] == "editor-STARTING-POSITION") {
			items.erase(items.begin() + i);
			std::vector<std::string> newitems;
			std::vector<std::string> newsaves;
			for (int player_i = 0; player_i < gamemap::MAX_PLAYERS; ++player_i) {
				//TODO gettext format
				std::string name = "Set starting position for player " + lexical_cast<std::string>(player_i);
				newitems.push_back(name);
			}

			items.insert(items.begin()+i, newitems.begin(), newitems.end());
			break;
		}
	}
}

void editor_controller::show_menu(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu)
{
	if (context_menu) {
		if (!get_map().on_board_with_border(gui().hex_clicked_on(xloc, yloc))) {
			return;
		}
	}
	
	std::vector<std::string> items = items_arg;
	hotkey::HOTKEY_COMMAND command;
	std::vector<std::string>::iterator i = items.begin();
	while(i != items.end()) {
		command = hotkey::get_hotkey(*i).get_id();
		if (command == hotkey::HOTKEY_UNDO) {
			if (get_map_context().can_undo()) {
				hotkey::get_hotkey(*i).set_description(_("Undo"));
			} else {
				hotkey::get_hotkey(*i).set_description(_("Can't Undo"));
			}
		} else if (command == hotkey::HOTKEY_REDO) {
			if (get_map_context().can_redo()) {
				hotkey::get_hotkey(*i).set_description(_("Redo"));
			} else {
				hotkey::get_hotkey(*i).set_description(_("Can't Redo"));
			}
		}
		++i;
	}
	expand_starting_position_menu(items);
	controller_base::show_menu(items, xloc, yloc, context_menu);
}

void editor_controller::cycle_brush()
{
	int x, y;
	SDL_GetMouseState(&x, &y);
	gamemap::location hex_clicked = gui().hex_clicked_on(x,y);
	gui().invalidate(get_brush()->project(hex_clicked));
	if (brush_ == &brushes_.back()) {
		brush_ = &brushes_.front();
	} else {
		++brush_;
	}
	std::set<gamemap::location> new_brush_locs = get_brush()->project(hex_clicked);
	gui().set_brush_locs(new_brush_locs);
	gui().invalidate(new_brush_locs);
}

void editor_controller::preferences()
{
	preferences::show_preferences_dialog(*gui_, game_config_);
	gui_->redraw_everything();
}

void editor_controller::toggle_grid()
{
	preferences::set_grid(!preferences::grid());
	gui_->invalidate_all();
}

void editor_controller::copy_selection()
{
	if (!get_map().selection().empty()) {
		clipboard_ = map_fragment(get_map(), get_map().selection());
		clipboard_.center();
	}
}

void editor_controller::cut_selection()
{
	copy_selection();
	editor_action_paint_area a(get_map().selection(), background_terrain_);
	get_map_context().perform_action(a);
	refresh_after_action();
}

void editor_controller::hotkey_set_mouse_action(hotkey::HOTKEY_COMMAND command)
{
	std::map<hotkey::HOTKEY_COMMAND, mouse_action*>::iterator i = mouse_actions_.find(command);
	if (i != mouse_actions_.end()) {
		mouse_action_ = i->second;
	} else {
		ERR_ED << "Invalid hotkey command (" << (int)command << ") passed to set_mouse_action\n";
	}
}

bool editor_controller::is_mouse_action_set(hotkey::HOTKEY_COMMAND command) const
{
	std::map<hotkey::HOTKEY_COMMAND, mouse_action*>::const_iterator i = mouse_actions_.find(command);
	return (i != mouse_actions_.end()) && (i->second == mouse_action_);
}


events::mouse_handler_base& editor_controller::get_mouse_handler_base()
{
	return *this;
}

editor_display& editor_controller::get_display()
{
	return *gui_;
}

brush* editor_controller::get_brush()
{
	return brush_;
}

mouse_action* editor_controller::get_mouse_action()
{
	return mouse_action_;
}


void editor_controller::refresh_after_action()
{
	//TODO rebuild and ivalidate only what's really needed
	if (get_map_context().needs_reload()) {
		reload_map();
		get_map_context().set_needs_reload(false);
		get_map_context().set_needs_terrain_rebuild(false);
		get_map_context().clear_changed_locations();
	} else if (get_map_context().needs_terrain_rebuild()) {
		gui().rebuild_all();
		gui().invalidate_all();	
		get_map_context().set_needs_terrain_rebuild(false);
		get_map_context().clear_changed_locations();
	} else {
		if (get_map_context().everything_changed()) {
			gui().invalidate_all();
		} else {
			gui().invalidate(get_map_context().changed_locations());
		}
		get_map_context().clear_changed_locations();
	}
	gui().recalculate_minimap();
}

void editor_controller::refresh_all()
{
	adjust_sizes(gui(), *size_specs_);
	//brush_bar_->adjust_size();
	palette_->draw(true);
	//brush_bar_->draw(true);
	gui().rebuild_all();
	gui().invalidate_all();
	gui().recalculate_minimap();
	get_map_context().set_needs_terrain_rebuild(false);
	get_map_context().clear_changed_locations();
}

void editor_controller::undo()
{
	get_map_context().undo();
	refresh_after_action();
}

void editor_controller::redo()
{
	get_map_context().redo();
	refresh_after_action();
}

void editor_controller::mouse_motion(int x, int y, const bool browse, bool update)
{
	if (mouse_handler_base::mouse_motion_default(x, y, update)) return;
	gamemap::location hex_clicked = gui().hex_clicked_on(x, y);
	if (dragging_ && (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LEFT) != 0
	&& get_map().on_board_with_border(drag_from_hex_)) {
		if (!get_map().on_board_with_border(hex_clicked)) return;
		if (get_mouse_action() != NULL) {
			LOG_ED << "Mouse drag\n";
			editor_action* last_undo = get_map_context().last_undo_action();
			bool partial = false;
			editor_action* a = get_mouse_action()->drag(*gui_, x, y, partial, last_undo);
			//Partial means that the mouse action has modified the last undo action and the controller shouldn't add
			//anything to the undo stack (hence a diferent perform_ call
			if (a != NULL) {
				if (partial) {
					get_map_context().perform_partial_action(*a);
				} else {
					get_map_context().perform_action(*a);
				}
				refresh_after_action();
				delete a;
			}
		} else {
			WRN_ED << __FUNCTION__ << ": There is no mouse action active!\n";
		}		
	} else {
		if (get_mouse_action() != NULL) {
			get_mouse_action()->move(*gui_, x, y);
		}
	}
}

bool editor_controller::left_click(int x, int y, const bool browse)
{
	LOG_ED << "Left click\n";
	if (mouse_handler_base::left_click(x, y, browse)) return true;
	LOG_ED << "Left click, after generic handling\n";
	gamemap::location hex_clicked = gui().hex_clicked_on(x, y);
	if (!get_map().on_board_with_border(hex_clicked)) return true;
	if (get_mouse_action() != NULL) {
		LOG_ED << "Left click action " << hex_clicked.x << " " << hex_clicked.y << "\n";
		editor_action* a = get_mouse_action()->click(*gui_, x, y);
		if (a != NULL) {
			get_map_context().perform_action(*a);
			refresh_after_action();
			delete a;
		}
		return true;
	} else {
		LOG_ED << __FUNCTION__ << ": There is no mouse action active!\n";
		return false;
	}
}

void editor_controller::left_drag_end(int x, int y, const bool browse)
{
	if (get_mouse_action() != NULL) {
		editor_action* a = get_mouse_action()->drag_end(*gui_, x, y);
		if (a != NULL) {
			get_map_context().perform_action(*a);
			refresh_after_action();
			delete a;
		}
	} else {
		LOG_ED << __FUNCTION__ << ": There is no mouse action active!\n";
	}	
}

} //end namespace editor2
