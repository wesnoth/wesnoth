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
#include "gui/dialogs/editor_generate_map.hpp"
#include "gui/dialogs/editor_resize_map.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/window.hpp"

#include "../config_adapter.hpp"
#include "../construct_dialog.hpp"
#include "../cursor.hpp"
#include "../filechooser.hpp"
#include "../filesystem.hpp"
#include "../font.hpp"
#include "../foreach.hpp"
#include "../gettext.hpp"
#include "../hotkeys.hpp"
#include "../map_create.hpp"
#include "../mapgen.hpp"
#include "../preferences.hpp"
#include "../random.hpp"
#include "../wml_exception.hpp"

#include "SDL.h"

#include <memory>

#include <boost/bind.hpp>

namespace {
	std::string default_dir = get_dir(get_dir(get_user_data_dir() + "/editor") + "/maps");
}

namespace editor2 {

editor_controller::editor_controller(const config &game_config, CVideo& video)
: controller_base(SDL_GetTicks(), game_config, video)
, mouse_handler_base(get_map()), rng_(NULL), rng_setter_(NULL)
, map_context_(editor_map(game_config, 44, 33, t_translation::GRASS_LAND))
, gui_(NULL), map_generator_(NULL), tooltip_manager_(video), floating_label_manager_(NULL)
, do_quit_(false), quit_mode_(EXIT_ERROR)
, toolbar_dirty_(true), auto_update_transitions_(true)
{
	init(video);
	rng_ = new rand_rng::rng();
	rng_setter_ = new rand_rng::set_random_generator(rng_);
	floating_label_manager_ = new font::floating_label_context();
	size_specs_ = new size_specs();
	adjust_sizes(gui(), *size_specs_);
	palette_ = new terrain_palette(gui(), *size_specs_, get_map(), game_config,
		foreground_terrain_, background_terrain_);
	foreach (const config* i, game_config.get_children("brush")) {
		brushes_.push_back(brush(*i));
	}
	if (brushes_.size() == 0) {
		ERR_ED << "No brushes defined!";
		brushes_.push_back(brush());
		brushes_[0].add_relative_location(0, 0);
	}
	brush_ = &brushes_[0];
	brush_bar_ = new brush_bar(gui(), *size_specs_, brushes_, &brush_);
	
	mouse_actions_.insert(std::make_pair(hotkey::HOTKEY_EDITOR_TOOL_PAINT, 
		new mouse_action_paint(foreground_terrain_, background_terrain_, &brush_, key_)));
	mouse_actions_.insert(std::make_pair(hotkey::HOTKEY_EDITOR_TOOL_FILL, 
		new mouse_action_fill(foreground_terrain_, background_terrain_, key_)));
	mouse_actions_.insert(std::make_pair(hotkey::HOTKEY_EDITOR_TOOL_SELECT, 
		new mouse_action_select(&brush_, key_)));
	mouse_actions_.insert(std::make_pair(hotkey::HOTKEY_EDITOR_TOOL_STARTING_POSITION,
		new mouse_action_starting_position(key_)));
	mouse_actions_.insert(std::make_pair(hotkey::HOTKEY_EDITOR_PASTE,
		new mouse_action_paste(clipboard_, key_)));
	foreach (const theme::menu& menu, gui().get_theme().menus()) {
		if (menu.items().size() == 1) {
			mouse_action_map::iterator i = mouse_actions_.find(hotkey::get_hotkey(menu.items().front()).get_id());
			if (i != mouse_actions_.end()) {
				i->second->set_toolbar_button(&menu);
			}
		}
	}
	foreach (const config* c, game_config.get_children("editor2_tool_hint")) {
		mouse_action_map::iterator i = mouse_actions_.find(hotkey::get_hotkey((*c)["id"]).get_id());
		if (i != mouse_actions_.end()) {
			mouse_action_hints_.insert(std::make_pair(i->first, (*c)["text"]));
		}
	}	
	hotkey_set_mouse_action(hotkey::HOTKEY_EDITOR_TOOL_PAINT);	
	hotkey::get_hotkey(hotkey::HOTKEY_QUIT_GAME).set_description(_("Quit Editor"));
	background_terrain_ = t_translation::GRASS_LAND;
	foreground_terrain_ = t_translation::MOUNTAIN;
	set_mouseover_overlay();
	get_map_context().set_starting_position_labels(gui());
	cursor::set(cursor::NORMAL);
	gui_->invalidate_game_status();
	palette_->adjust_size();
	brush_bar_->adjust_size();
	refresh_all();
	gui_->draw();
	palette_->draw(true);
	brush_bar_->draw(true);
	load_tooltips();
	redraw_toolbar();
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
	gui_->add_redraw_observer(boost::bind(&editor_controller::display_redraw_callback, this, _1));
}

void editor_controller::load_tooltips()
{
	// Tooltips for the groups
	palette_->load_tooltips();
}

editor_controller::~editor_controller()
{
	delete palette_;
	delete brush_bar_;
	delete size_specs_;
	delete floating_label_manager_;
	delete map_generator_;
    delete gui_;
	foreach (const mouse_action_map::value_type a, mouse_actions_) {
		delete a.second;
	}
	delete prefs_disp_manager_;
	delete rng_setter_;
	delete rng_;
}

EXIT_STATUS editor_controller::main_loop()
{
	try {
		while (!do_quit_) {
			play_slice();
		}
	} catch (editor_exception& e) {
		gui::message_dialog(gui(), _("Fatal error"), e.what()).show();
		return EXIT_ERROR;
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
			_("Do you want to discard all changes you made to the map?"), gui::YES_NO).show();
	} else {
		return true;
	}
}

void editor_controller::load_map_dialog()
{
	if (!confirm_discard()) return;
	std::string fn = get_map_context().get_filename();
	if (fn.empty()) {
		fn = default_dir;
	}
	int res = dialogs::show_file_chooser_dialog(gui(), fn, _("Choose a Map to Load"));
	if (res == 0) {
		load_map(fn);
	}
}

void editor_controller::new_map_dialog()
{
	if (!confirm_discard()) return;
	gui2::teditor_new_map dialog;
	dialog.set_map_width(get_map().w());
	dialog.set_map_height(get_map().h());
	
	dialog.show(gui().video());
	int res = dialog.get_retval();
	if(res == gui2::twindow::OK) {
		int w = dialog.map_width();
		int h = dialog.map_height();
		t_translation::t_terrain fill = t_translation::GRASS_LAND;
		new_map(w, h, fill);
	}
}

void editor_controller::save_map_as_dialog()
{
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

void editor_controller::generate_map_dialog()
{
	if (map_generator_ == NULL) {
		// Initialize the map generator if it has not been used before
		const config* const toplevel_cfg = game_config_.find_child("multiplayer","id","multiplayer_Random_Map");
		const config* const cfg = toplevel_cfg == NULL ? NULL : toplevel_cfg->child("generator");
		if (cfg == NULL) {
			WRN_ED << "No random map generator\n";
			return;
		}
		else {
			map_generator_ = create_map_generator("", cfg);
		}
	}
	gui2::teditor_generate_map dialog;
	dialog.set_map_generator(map_generator_);
	dialog.set_gui(&gui());
	dialog.show(gui().video());
	
	int res = dialog.get_retval();
	if(res == gui2::twindow::OK) {
		if (!confirm_discard()) return;
		std::string map_string =
			map_generator_->create_map(std::vector<std::string>());
		if (map_string.empty()) {
			gui::message_dialog(gui(), "",
							 _("Map creation failed.")).show();
		} else {
			editor_map new_map(game_config_, map_string);
			set_map(new_map);
		}
	}
}

void editor_controller::resize_map_dialog()
{
	gui2::teditor_resize_map dialog;
	dialog.set_map_width(get_map().w());
	dialog.set_map_height(get_map().h());
	dialog.set_old_map_width(get_map().w());
	dialog.set_old_map_height(get_map().h());
	
	dialog.show(gui().video());
	int res = dialog.get_retval();
	if(res == gui2::twindow::OK) {
		int w = dialog.map_width();
		int h = dialog.map_height();
		if (w != get_map().w() || h != get_map().h()) {
			t_translation::t_terrain fill = background_terrain_;
			if (dialog.copy_edge_terrain()) {
				fill = t_translation::NONE_TERRAIN;
			}
			int x_offset = get_map().w() - w;
			int y_offset = get_map().h() - h;
			switch (dialog.expand_direction()) {
				case gui2::teditor_resize_map::EXPAND_BOTTOM_RIGHT:
				case gui2::teditor_resize_map::EXPAND_BOTTOM:
				case gui2::teditor_resize_map::EXPAND_BOTTOM_LEFT:
					y_offset = 0;
					break;
				case gui2::teditor_resize_map::EXPAND_RIGHT:
				case gui2::teditor_resize_map::EXPAND_CENTER:
				case gui2::teditor_resize_map::EXPAND_LEFT:
					y_offset /= 2;
					break;
				case gui2::teditor_resize_map::EXPAND_TOP_RIGHT:
				case gui2::teditor_resize_map::EXPAND_TOP:
				case gui2::teditor_resize_map::EXPAND_TOP_LEFT:
					break;
				default:
					y_offset = 0;
					WRN_ED << "Unknown resize expand direction\n";
			}
			switch (dialog.expand_direction()) {
				case gui2::teditor_resize_map::EXPAND_BOTTOM_RIGHT:
				case gui2::teditor_resize_map::EXPAND_RIGHT:
				case gui2::teditor_resize_map::EXPAND_TOP_RIGHT:
					x_offset = 0;
					break;
				case gui2::teditor_resize_map::EXPAND_BOTTOM:
				case gui2::teditor_resize_map::EXPAND_CENTER:
				case gui2::teditor_resize_map::EXPAND_TOP:
					x_offset /= 2;
					break;
				case gui2::teditor_resize_map::EXPAND_BOTTOM_LEFT:
				case gui2::teditor_resize_map::EXPAND_LEFT:
				case gui2::teditor_resize_map::EXPAND_TOP_LEFT:
					break;
				default:
					x_offset = 0;
			}
			editor_action_resize_map a(w, h, x_offset, y_offset, fill);
			perform_refresh(a);
		}
	}
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
		std::string message = _("There was an error while loading the map:");
		message += "\n";
		message += e.msg_;
		gui::message_dialog(gui(), _("Error loading map (format)"), message).show();
		return;
	} catch (twml_exception& e) {
		std::string message = _("There was an error while loading the map:");
		message += "\n";
		message += e.user_message;
		gui::message_dialog(gui(), _("Error loading map (wml)"), message).show();
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
		case HOTKEY_EDITOR_SELECTION_FILL:
		case HOTKEY_EDITOR_SELECTION_RANDOMIZE:
			return !get_map().selection().empty();
		case HOTKEY_EDITOR_SELECTION_ROTATE:
		case HOTKEY_EDITOR_SELECTION_FLIP:
		case HOTKEY_EDITOR_SELECTION_GENERATE:			
			return false; //not implemented
		case HOTKEY_EDITOR_PASTE:
			return !clipboard_.empty();
		case HOTKEY_EDITOR_SELECT_ALL:
		case HOTKEY_EDITOR_SELECT_INVERSE:
		case HOTKEY_EDITOR_SELECT_NONE:
		case HOTKEY_EDITOR_MAP_RESIZE:
		case HOTKEY_EDITOR_MAP_FLIP_X:
		case HOTKEY_EDITOR_MAP_FLIP_Y:
		case HOTKEY_EDITOR_MAP_GENERATE:
		case HOTKEY_EDITOR_REFRESH:
		case HOTKEY_EDITOR_UPDATE_TRANSITIONS:
		case HOTKEY_EDITOR_AUTO_UPDATE_TRANSITIONS:
		case HOTKEY_EDITOR_REFRESH_IMAGE_CACHE:
			return true;
		case HOTKEY_EDITOR_MAP_ROTATE:
			return false; //not implemented
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
		case HOTKEY_EDITOR_TOOL_STARTING_POSITION:
			return is_mouse_action_set(command) ? ACTION_ON : ACTION_OFF;
		case HOTKEY_EDITOR_AUTO_UPDATE_TRANSITIONS:
			return auto_update_transitions_ ? ACTION_ON : ACTION_OFF;
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
			if (get_mouse_action()->uses_terrains()) set_mouseover_overlay();
			return true;
		case HOTKEY_EDITOR_TOOL_PAINT:
		case HOTKEY_EDITOR_TOOL_FILL:
		case HOTKEY_EDITOR_TOOL_SELECT:
		case HOTKEY_EDITOR_TOOL_STARTING_POSITION:
			hotkey_set_mouse_action(command);
			return true;
		case HOTKEY_EDITOR_PASTE: //paste is somewhat different as it might be "one action then revert to previous mode"
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
			if (!get_map().everything_selected()) {
				perform_refresh(editor_action_select_all());
				return true;
			} //else intentionally fall through
		case HOTKEY_EDITOR_SELECT_INVERSE:
			perform_refresh(editor_action_select_inverse());
			return true;
		case HOTKEY_EDITOR_SELECT_NONE:
			perform_refresh(editor_action_select_none());
		case HOTKEY_EDITOR_SELECTION_FILL:
			fill_selection();
			return true;
		case HOTKEY_EDITOR_SELECTION_RANDOMIZE:
			perform_refresh(editor_action_shuffle_area(get_map().selection()));
			return true;
		case HOTKEY_EDITOR_MAP_FLIP_X:
			perform_refresh(editor_action_flip_x());
			return true;
		case HOTKEY_EDITOR_MAP_FLIP_Y:
			perform_refresh(editor_action_flip_y());
			return true;
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
		case HOTKEY_EDITOR_MAP_GENERATE:
			generate_map_dialog();
			return true;
		case HOTKEY_EDITOR_MAP_RESIZE:
			resize_map_dialog();
			return true;
		case HOTKEY_EDITOR_AUTO_UPDATE_TRANSITIONS:
			auto_update_transitions_ = !auto_update_transitions_;
			if (!auto_update_transitions_) {
				return true;
			} // else intentionally fall through
		case HOTKEY_EDITOR_UPDATE_TRANSITIONS:
			refresh_all();
			return true;
		case HOTKEY_EDITOR_REFRESH:
			reload_map();
			return true;
		case HOTKEY_EDITOR_REFRESH_IMAGE_CACHE:
			refresh_image_cache();
			return true;
		default:
			return controller_base::execute_command(command, index);
	}
	return false;
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
	perform_refresh(a);
}

void editor_controller::fill_selection()
{
	editor_action_paint_area a(get_map().selection(), foreground_terrain_);
	perform_refresh(a);
}


void editor_controller::hotkey_set_mouse_action(hotkey::HOTKEY_COMMAND command)
{
	std::map<hotkey::HOTKEY_COMMAND, mouse_action*>::iterator i = mouse_actions_.find(command);
	if (i != mouse_actions_.end()) {
		mouse_action_ = i->second;
		if (mouse_action_->uses_terrains()) {
			set_mouseover_overlay();
		} else {
			clear_mouseover_overlay();
		}
		redraw_toolbar();
		gui().set_report_content(reports::EDIT_LEFT_BUTTON_FUNCTION,
				hotkey::get_hotkey(command).get_description());
		gui().set_toolbar_hint(mouse_action_hints_[command]);
		gui().invalidate_game_status();		
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

void editor_controller::perform_refresh_delete(editor_action* action, bool drag_part /* =false */)
{
	if (action) {
		std::auto_ptr<editor_action> action_auto(action);
		perform_refresh(*action, drag_part);
	}
}

void editor_controller::perform_refresh(const editor_action& action, bool /* drag_part =false */)
{
	get_map_context().perform_action(action);
	refresh_after_action();
}


void editor_controller::redraw_toolbar()
{
	foreach (mouse_action_map::value_type a, mouse_actions_) {
		if (a.second->toolbar_button() != NULL) {
			SDL_Rect r = a.second->toolbar_button()->location(gui().screen_area());
			SDL_Rect outline = {r.x - 2, r.y - 2, r.h + 4, r.w + 4};
			//outline = intersect_rects(r, gui().screen_area());
			SDL_Surface* const screen = gui().video().getSurface();
			Uint32 color;
			if (a.second == mouse_action_) {
				color = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
			} else {
				color = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
			}
			draw_rectangle(outline.x, outline.y, outline.w, outline.h, color, gui().video().getSurface());
			update_rect(outline);
		}
	}
	toolbar_dirty_ = false;
}

void editor_controller::refresh_image_cache()
{
	image::flush_cache();
	refresh_all();
}

void editor_controller::refresh_after_action(bool drag_part)
{
	if (get_map_context().needs_reload()) {
		reload_map();
		get_map_context().set_needs_reload(false);
		get_map_context().set_needs_terrain_rebuild(false);
		get_map_context().set_needs_labels_reset(false);
	} else {
		if (get_map_context().needs_terrain_rebuild()) {
			if (!drag_part || auto_update_transitions_ || get_map_context().everything_changed()) {
				gui().rebuild_all();
				gui().invalidate_all();	
				get_map_context().set_needs_terrain_rebuild(false);
			} else {
				foreach (const gamemap::location& loc, get_map_context().changed_locations()) {
					gui().rebuild_terrain(loc);
				}
				gui().invalidate(get_map_context().changed_locations());
			}
		} else {
			if (get_map_context().everything_changed()) {
				gui().invalidate_all();
			} else {
				gui().invalidate(get_map_context().changed_locations());
			}
		}
		if (get_map_context().needs_labels_reset()) {
			get_map_context().clear_starting_position_labels(gui());
			get_map_context().set_starting_position_labels(gui());
			get_map_context().set_needs_labels_reset(false);
		}
	}
	get_map_context().clear_changed_locations();
	gui().recalculate_minimap();
}

void editor_controller::refresh_all()
{
	gui().rebuild_all();
	gui().redraw_everything();
	gui().recalculate_minimap();
	get_map_context().set_needs_terrain_rebuild(false);
	get_map_context().clear_changed_locations();
}

void editor_controller::display_redraw_callback(display&)
{
	adjust_sizes(gui(), *size_specs_);
	palette_->adjust_size();
	brush_bar_->adjust_size();
	palette_->draw(true);
	brush_bar_->draw(true);
	//display::redraw_everything removes our custom tooltips so reload them
	load_tooltips();
	gui().invalidate_all();
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

void editor_controller::mouse_motion(int x, int y, const bool /*browse*/, bool update)
{
	if (mouse_handler_base::mouse_motion_default(x, y, update)) return;
	gamemap::location hex_clicked = gui().hex_clicked_on(x, y);
	if (get_map().on_board_with_border(drag_from_hex_) && is_dragging()) {
		editor_action* a = NULL;
		bool partial = false;
		editor_action* last_undo = get_map_context().last_undo_action();
		if (dragging_left_ && (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(1)) != 0) {
			if (!get_map().on_board_with_border(hex_clicked)) return;
			a = get_mouse_action()->drag_left(*gui_, x, y, partial, last_undo);
		} else if (dragging_right_ && (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(3)) != 0) {
			if (!get_map().on_board_with_border(hex_clicked)) return;
			a = get_mouse_action()->drag_right(*gui_, x, y, partial, last_undo);
		}
		//Partial means that the mouse action has modified the last undo action and the controller shouldn't add
		//anything to the undo stack (hence a diferent perform_ call
		if (a != NULL) {
			std::auto_ptr<editor_action> aa(a);
			if (partial) {
				get_map_context().perform_partial_action(*a);
			} else {
				get_map_context().perform_action(*a);
			}
			refresh_after_action(true);
		}
	} else {
		get_mouse_action()->move(*gui_, hex_clicked);
	}
	gui().highlight_hex(hex_clicked);
}

bool editor_controller::allow_mouse_wheel_scroll(int x, int y)
{
	return get_map().on_board_with_border(gui().hex_clicked_on(x,y));
}

bool editor_controller::right_click_show_menu(int /*x*/, int /*y*/, const bool /*browse*/)
{
	return false;
}

bool editor_controller::left_click(int x, int y, const bool browse)
{
	clear_mouseover_overlay();
	LOG_ED << "Left click\n";
	if (mouse_handler_base::left_click(x, y, browse)) return true;
	LOG_ED << "Left click, after generic handling\n";
	gamemap::location hex_clicked = gui().hex_clicked_on(x, y);
	if (!get_map().on_board_with_border(hex_clicked)) return true;
	LOG_ED << "Left click action " << hex_clicked.x << " " << hex_clicked.y << "\n";
	editor_action* a = get_mouse_action()->click_left(*gui_, x, y);
	perform_refresh_delete(a, true);
	return false;
}

void editor_controller::left_drag_end(int x, int y, const bool /*browse*/)
{
	editor_action* a = get_mouse_action()->drag_end(*gui_, x, y);
	perform_refresh_delete(a);
}

void editor_controller::left_mouse_up(int /*x*/, int /*y*/, const bool /*browse*/)
{
	refresh_after_action();
	if (get_mouse_action()->uses_terrains()) set_mouseover_overlay();
}

bool editor_controller::right_click(int x, int y, const bool browse)
{
	clear_mouseover_overlay();
	LOG_ED << "Right click\n";
	if (mouse_handler_base::right_click(x, y, browse)) return true;
	LOG_ED << "Right click, after generic handling\n";
	gamemap::location hex_clicked = gui().hex_clicked_on(x, y);
	if (!get_map().on_board_with_border(hex_clicked)) return true;
	LOG_ED << "Right click action " << hex_clicked.x << " " << hex_clicked.y << "\n";
	editor_action* a = get_mouse_action()->click_right(*gui_, x, y);
	perform_refresh_delete(a, true);
	return false;
}

void editor_controller::right_drag_end(int x, int y, const bool /*browse*/)
{
	editor_action* a = get_mouse_action()->drag_end(*gui_, x, y);
	perform_refresh_delete(a);
}

void editor_controller::right_mouse_up(int /*x*/, int /*y*/, const bool /*browse*/)
{
	refresh_after_action();
	if (get_mouse_action()->uses_terrains()) set_mouseover_overlay();
}

void editor_controller::process_keyup_event(const SDL_Event& event)
{
	editor_action* a = get_mouse_action()->key_event(gui(), event);
	perform_refresh_delete(a);
}

//todo make this a virtual in mouse_action
void editor_controller::set_mouseover_overlay()
{
	surface image_fg(image::get_image("terrain/" + get_map().get_terrain_info(
				foreground_terrain_).editor_image() +
				".png"));
	surface image_bg(image::get_image("terrain/" + get_map().get_terrain_info(
				background_terrain_).editor_image() +
				".png"));

	if (image_fg == NULL || image_bg == NULL) {
		ERR_ED << "Missing terrain icon\n";
		gui().set_mouseover_hex_overlay(NULL);
		return; 
	}

	// Create a transparent surface of the right size.
	surface image = create_compatible_surface(image_fg, image_fg->w, image_fg->h);
	SDL_FillRect(image,NULL,SDL_MapRGBA(image->format,0,0,0, 0));

	// For efficiency the size of the tile is cached.
	// We assume all tiles are of the same size.
	// The zoom factor can change, so it's not cached.
	// NOTE: when zooming and not moving the mouse, there are glitches.
	// Since the optimal alpha factor is unknown, it has to be calculated
	// on the fly, and caching the surfaces makes no sense yet.
	static const Uint8 alpha = 196;
	static const int size = image_fg->w;
	static const int half_size = size / 2;
	static const int quarter_size = size / 4;
	static const int offset = 2;
	static const int new_size = half_size - 2;
	const int zoom = static_cast<int>(size * gui().get_zoom_factor());

	// Blit left side
	image_fg = scale_surface(image_fg, new_size, new_size);
	SDL_Rect rcDestLeft = { offset, quarter_size, 0, 0 };
	SDL_BlitSurface ( image_fg, NULL, image, &rcDestLeft );

	// Blit left side
	image_bg = scale_surface(image_bg, new_size, new_size);
	SDL_Rect rcDestRight = { half_size, quarter_size, 0, 0 };
	SDL_BlitSurface ( image_bg, NULL, image, &rcDestRight );

	// Add the alpha factor and scale the image
	image = scale_surface(adjust_surface_alpha(image, alpha), zoom, zoom);

	// Set as mouseover
	gui().set_mouseover_hex_overlay(image);
}

void editor_controller::clear_mouseover_overlay()
{
	gui().clear_mouseover_hex_overlay();
}

} //end namespace editor2
