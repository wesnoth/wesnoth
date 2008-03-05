/* $Id$ */
/*
  Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
  Part of the Battle for Wesnoth Project http://www.wesnoth.org/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2
  or at your option any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the COPYING file for more details.
*/

//! @file editor.cpp
//! Map-editor.

#include "SDL.h"
#include "SDL_keysym.h"

#include "../config.hpp"
#include "../construct_dialog.hpp"
#include "../cursor.hpp"
#include "../file_chooser.hpp"
#include "../filesystem.hpp"
#include "../font.hpp"
#include "../game_config.hpp"
#include "../gettext.hpp"
#include "../key.hpp"
#include "../language.hpp"
#include "../widgets/menu.hpp"
#include "../pathfind.hpp"

#include "../preferences.hpp"
#include "../sdl_utils.hpp"
#include "../tooltips.hpp"
#include "../team.hpp"
#include "../util.hpp"
#include "../video.hpp"
#include "../wml_separators.hpp"
#include "../wml_exception.hpp"
#include "serialization/parser.hpp"
#include "serialization/string_utils.hpp"

#include "editor.hpp"
#include "map_manip.hpp"
#include "editor_dialogs.hpp"
#include "editor_palettes.hpp"

#include <cctype>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <cmath>

namespace {
	static std::string wm_title_string;

	// Milliseconds to sleep in every iteration of the main loop.
	const unsigned int sdl_delay = 20;
	const std::string prefs_filename = get_dir(get_user_data_dir() + "/editor")
		+ "/preferences";

	// Return the side that has it's starting position at hex,
	// or -1 if none has.
	int starting_side_at(const gamemap& map, const gamemap::location hex) {
		int start_side = -1;
		for (int i = 0; i < gamemap::MAX_PLAYERS+1; i++) {
			if (map.starting_position(i) == hex) {
				start_side = i;
			}
		}
		return start_side;
	}
}

namespace map_editor {

bool check_data(std::string &data, std::string &filename, bool &from_scenario, config &game_cfg)
{
	std::string file_content = data;

	if (valid_mapdata(file_content, game_cfg)) {
		from_scenario = false;
		return true;
	}

	std::string::size_type start, stop;
	start = file_content.find("map_data=");
	if (start==std::string::npos)
		return false;
	start += 10;
	stop = file_content.find('\"', start);
	if (stop==std::string::npos)
		return false;
	std::string nfname;
	std::string newfilename = std::string(file_content, start, stop-start);
	if (newfilename[0]=='{' && newfilename[newfilename.size()-1]=='}') {
		newfilename.erase(newfilename.begin());
		newfilename.erase(newfilename.end() - 1);

		// If the filename begins with a '~', then look in the user's data directory.
		// If the filename begins with a '@' then we look in the user's data directory,
		// but default to the standard data directory if it's not found there.
		if(newfilename != "" && (newfilename[0] == '~' || newfilename[0] == '@')) {
			nfname = newfilename;
			nfname.erase(nfname.begin(),nfname.begin()+1);
			nfname = get_user_data_dir() + "/data/" + nfname;

			std::cout << "got relative name '" << newfilename << "' -> '" << nfname << "'\n";

			if(newfilename[0] == '@' && file_exists(nfname) == false && is_directory(nfname) == false) {
				nfname = "data/" + newfilename.substr(1);
			}
		} else
		if(newfilename.size() >= 2 && newfilename[0] == '.' &&
			newfilename[1] == '/' ) {
			// If the filename begins with a "./",
			// then look in the same directory
			// as the file currrently being preprocessed
			nfname = newfilename;
			nfname.erase(nfname.begin(),nfname.begin()+2);
			nfname = directory_name(filename) + nfname;

		} else {
				nfname = "data/" + newfilename;
		}

		data = read_file(nfname);
		filename = nfname;
		from_scenario = false;
	} else {
		data = newfilename;
		from_scenario = true;
	}
	return true;
}

// The map_editor object will be recreated when operations that affect
// the whole map takes place. It may not be the most beautiful solution,
// but it is the way the least interference with the game system is
// needed. That is the reason we need some static variables to handle
// things that should be permanent through the program's life time.
// Of course, the functionality of this assumes that no more than one
// map_editor object will exist, but that is a reasonable restriction imho.
bool map_editor::first_time_created_ = true;
int map_editor::num_operations_since_save_ = 0;
config map_editor::prefs_;
config map_editor::hotkeys_;
// Do not init the l_button_func_ to DRAW, since it should be changed
// in the constructor to update the report the first time.
map_editor::LEFT_BUTTON_FUNC map_editor::l_button_func_ = PASTE;
t_translation::t_terrain map_editor::old_fg_terrain_;
t_translation::t_terrain map_editor::old_bg_terrain_;
int map_editor::old_brush_size_;

map_editor::map_editor(editor_display &gui, editormap &map, config &theme, config &game_config)
	: gui_(gui), map_(map), abort_(DONT_ABORT),
	  theme_(theme), game_config_(game_config), map_dirty_(false),  auto_update_(true), l_button_palette_dirty_(true),
	  everything_dirty_(false), palette_(gui, size_specs_, map, game_config), brush_(gui, size_specs_),
	  l_button_held_func_(NONE), tooltip_manager_(gui_.video()), floating_label_manager_(),
	  mouse_moved_(false),
	  highlighted_locs_cleared_(false), prefs_disp_manager_(&gui_), all_hexes_selected_(false) {

	// Set size specs.
	adjust_sizes(gui_, size_specs_);
	if (first_time_created_) {
		// Perform some initializations that should only be performed
		// the first time the editor object is created.
		try {
			scoped_istream stream = istream_file(prefs_filename);
			read(prefs_, *stream);
		}
		catch (config::error e) {
			std::cerr << "Error when reading " << prefs_filename << ": "
					  << e.message << std::endl;
		}
		hotkey::load_hotkeys(theme_);
		hotkey::load_hotkeys(prefs_);
		left_button_func_changed(DRAW);
		first_time_created_ = false;
	}
	else {
		hotkey::load_hotkeys(hotkeys_);

		//! @todo FIXME: saved hotkeys don't reload correctly,
		//! so we just reread the theme hotkeys every time
		hotkey::load_hotkeys(theme_);

		palette_.select_fg_terrain(old_fg_terrain_);
		palette_.select_bg_terrain(old_bg_terrain_);
		brush_.select_brush_size(old_brush_size_);
	}

	hotkey::load_descriptions();
	recalculate_starting_pos_labels();
	gui_.invalidate_game_status();
	//gui_.begin_game();
	gui_.invalidate_all();
	gui_.draw();
	palette_.adjust_size();
	brush_.adjust_size();
	events::raise_draw_event();
	redraw_everything();
	load_tooltips();
}

/**
 * @todo
 * This should be replaced by a WML tag called 'tooltip='
 * in the data/themes/editor.cfg file.
 * The theme and display classes should then load
 * the given tooltip in the button.
 */
void map_editor::load_tooltips()
{
	// Clear all previous tooltips
	tooltips::clear_tooltips();

	// Add tooltips to all buttons
	const theme &t = gui_.get_theme();
	const std::vector<theme::menu> &menus = t.menus();
	std::vector<theme::menu>::const_iterator it;
	for (it = menus.begin(); it != menus.end(); it++) {

		// Get the button's screen location
		SDL_Rect screen = gui_.screen_area();

		const SDL_Rect tooltip_rect = (*it).location(screen);
		std::string text = "";

		const std::vector<std::string> &menu_items = (*it).items();
		if (menu_items.size() == 1) {
			if(menu_items.back() == "editdraw")
				text = _("Draw tiles");
			else if(menu_items.back() == "editfloodfill")
				text = _("Fill");
			else if(menu_items.back() == "editsetstartpos")
				text = _("Set player's starting position");
			else if(menu_items.back() == "zoomin")
				text = _("Zoom in");
			else if(menu_items.back() == "zoomout")
				text = _("Zoom out");
			else if(menu_items.back() == "undo")
				text = _("Undo");
			else if(menu_items.back() == "redo")
				text = _("Redo");
			else if(menu_items.back() == "zoomdefault")
				text = _("Zoom to default view");
			else if(menu_items.back() == "togglegrid")
				text = _("Toggle grid");
			else if(menu_items.back() == "editresize")
				text = _("Resize the map");
			else if(menu_items.back() == "editflip")
				text = _("Flip map");
			else if(menu_items.back() == "editupdate")
				text = _("Update transitions");
			else if(menu_items.back() == "editautoupdate")
				text = _("Delay transition updates");
		}

		if(text != "")
			tooltips::add_tooltip(tooltip_rect, text);
	}

	// Tooltips for the groups
	palette_.load_tooltips();
}

map_editor::~map_editor() {
	// Save the hotkeys so that they are remembered if the editor is recreated.
	hotkey::save_hotkeys(hotkeys_);
	old_fg_terrain_ = palette_.selected_fg_terrain();
	old_bg_terrain_ = palette_.selected_bg_terrain();
	old_brush_size_ = brush_.selected_brush_size();
	try {
		scoped_ostream prefs_file = ostream_file(prefs_filename);
		write(*prefs_file, prefs_);
	}
	catch (io_exception& e) {
		std::cerr << "Error when writing to " << prefs_filename << ": "
				  << e.what() << std::endl;
	}
}

void map_editor::handle_event(const SDL_Event &event) {
	if (event.type == SDL_VIDEORESIZE) {
		everything_dirty_ = true;
	}
	int mousex, mousey;

	SDL_GetMouseState(&mousex,&mousey);
	const SDL_KeyboardEvent keyboard_event = event.key;
	handle_keyboard_event(keyboard_event, mousex, mousey);

	const SDL_MouseButtonEvent mouse_button_event = event.button;
	handle_mouse_button_event(mouse_button_event, mousex, mousey);

	tooltips::process(mousex, mousey);
}

void map_editor::handle_keyboard_event(const SDL_KeyboardEvent &event,
                                       const int /*mousex*/, const int /*mousey*/) {
	if (event.type == SDL_KEYDOWN) {
		const SDLKey sym = event.keysym.sym;
		// We must intercept escape-presses here,
		// because we don't want the default shutdown behavior,
		// we want to ask for saving.
		if (sym == SDLK_ESCAPE) {
			set_abort();
		}
		else {
			const bool old_fullscreen = preferences::fullscreen();
			const std::pair<int, int> old_resolution = preferences::resolution();
			hotkey::key_event(gui_, event, this);
			// A key event may change the video mode.
			// The redraw functionality inside the preferences module
			// does not redraw our palettes, so we need to check
			// if the mode has changed and if so redraw everything.
			if (preferences::fullscreen() != old_fullscreen
				|| old_resolution != preferences::resolution()) {
				everything_dirty_ = true;
			}
		}
	}
}

void map_editor::handle_mouse_button_event(const SDL_MouseButtonEvent &event,
                                           const int mousex, const int mousey) {
	if (event.type == SDL_MOUSEBUTTONDOWN) {
		const Uint8 button = event.button;
		int scrollx = 0;
		int scrolly = 0;
		if (button == SDL_BUTTON_RIGHT) {
			selected_hex_ = gui_.hex_clicked_on(mousex, mousey);
			const theme::menu* const m = gui_.get_theme().context_menu();
			if (m != NULL) {
				show_menu(m->items(), mousex, mousey + 1, true);
			}
		}
		if (button == SDL_BUTTON_LEFT) {
			gamemap::location hex_clicked = gui_.hex_clicked_on(mousex, mousey);
			if (map_.on_board(hex_clicked, true)) {
				left_click(hex_clicked);
			}
		}
		if (button == SDL_BUTTON_RIGHT) {
			gamemap::location hex_clicked = gui_.hex_clicked_on(mousex, mousey);
			if (map_.on_board(hex_clicked, true)) {
				right_click(hex_clicked);
			}
		}
		// Mimic the game's behavior on middle click and mouse wheel movement.
		// It would be nice to have had these in functions provided from the game,
		// but I don't want to interfer too much with the game code
		// and it's fairly simple stuff to rip.
		if (button == SDL_BUTTON_MIDDLE) {
			const SDL_Rect& rect = gui_.map_area();
			const int centerx = (rect.x + rect.w) / 2;
			const int centery = (rect.y + rect.h) / 2;

			const int xdisp = mousex - centerx;
			const int ydisp = mousey - centery;
			gui_.scroll(xdisp, ydisp);
		}
		if (point_in_rect(mousex, mousey, gui_.map_outside_area())) {
			if (event.button == SDL_BUTTON_WHEELUP) {
				scrolly = - preferences::scroll_speed();
			} else if (event.button == SDL_BUTTON_WHEELDOWN) {
				scrolly = preferences::scroll_speed();
			} else if (event.button == SDL_BUTTON_WHEELLEFT) {
				scrollx = - preferences::scroll_speed();
			} else if (event.button == SDL_BUTTON_WHEELRIGHT) {
				scrollx = preferences::scroll_speed();
			}
		}

		if (scrollx != 0 || scrolly != 0) {
		CKey pressed;
		// Alt + mousewheel do an 90° rotation on the scroll direction
		if (pressed[SDLK_LALT] || pressed[SDLK_RALT])
			gui_.scroll(scrolly,scrollx);
		else
			gui_.scroll(scrollx,scrolly);
		}

	}
	if (event.type == SDL_MOUSEBUTTONUP) {
		// If we miss the mouse up event, we need to perform the actual
		// movement if we are in the progress of moving a selection.
		if (l_button_held_func_ == MOVE_SELECTION) {
			perform_selection_move();
		}
		l_button_held_func_ = NONE;
	}
}

void map_editor::left_click(const gamemap::location hex_clicked) {
	if (key_[SDLK_LSHIFT] || key_[SDLK_RSHIFT]) {
		if (selected_hexes_.find(hex_clicked)
			== selected_hexes_.end()) {
			l_button_held_func_ = ADD_SELECTION;
		}
		else {
			l_button_held_func_ = REMOVE_SELECTION;
		}
	}
	else if (key_[SDLK_RCTRL] || key_[SDLK_LCTRL]) {
		const t_translation::t_terrain terrain = map_.get_terrain(selected_hex_);
		if(palette_.selected_fg_terrain() != terrain) {
			palette_.select_fg_terrain(terrain);
		}
		l_button_held_func_ = NONE;
	}
	else if (selected_hexes_.find(hex_clicked) != selected_hexes_.end()) {
		l_button_held_func_ = MOVE_SELECTION;
		selection_move_start_ = hex_clicked;
	}
	else if (!selected_hexes_.empty()) {
		// If hexes are selected, clear them
		// and do not draw anything.
		selected_hexes_.clear();
		clear_highlighted_hexes_in_gui();
	}
	else if (l_button_func_ == DRAW) {
		reset_mouseover_overlay();
		draw_on_mouseover_hexes(palette_.selected_fg_terrain());
		l_button_held_func_ = DRAW_TERRAIN;
	}
	else if (l_button_func_ == FLOOD_FILL) {
		perform_flood_fill(palette_.selected_fg_terrain());
	}
	else if (l_button_func_ == SET_STARTING_POSITION) {
		perform_set_starting_pos();
	}
	else if (l_button_func_ == PASTE) {
		perform_paste();
	}
}

void map_editor::right_click(const gamemap::location hex_clicked ) {
	if (key_[SDLK_RCTRL] || key_[SDLK_LCTRL]) {
		const t_translation::t_terrain terrain = map_.get_terrain(hex_clicked);
		if(palette_.selected_fg_terrain() != terrain) {
			palette_.select_bg_terrain(terrain);
		}
	}
	else if (l_button_func_ == FLOOD_FILL) {
		perform_flood_fill(palette_.selected_bg_terrain());
	}
}

/**
 * Change the language (effectively reload the editor).
 */
void map_editor::change_language() {
	std::vector<language_def> langdefs = get_languages();

	// This only works because get_languages() returns a fresh vector
	// at each calls unless show_gui cleans the "*" flag.
	const std::vector<language_def>::iterator current = std::find(langdefs.begin(),langdefs.end(),get_language());
	if(current != langdefs.end()) {
		(*current).language = "*" + (*current).language;
	}

	// Prepare a copy with just the labels for the list to be displayed
	std::vector<std::string> langs;
	langs.reserve(langdefs.size());
	std::transform(langdefs.begin(),langdefs.end(),std::back_inserter(langs),languagedef_name);

	const std::string language = _("Language");
	const std::string preferred = _("Choose your preferred language:");
	gui::dialog lmenu = gui::dialog(gui_, language, preferred,
					  gui::OK_CANCEL);
	lmenu.set_menu(langs);
	int res = lmenu.show();
	const std::vector<language_def>& languages = get_languages();
	if(size_t(res) < languages.size()) {
		::set_language(languages[res]);
		preferences::set_language(languages[res].localename);

		game_config_.reset_translation();

		// Reload tooltips and menu items
		load_tooltips();
	}

	// Update the frame title
	wm_title_string = _("Battle for Wesnoth Map Editor");
	wm_title_string += " - " + game_config::version
			+ (game_config::svnrev.empty() ? "" :
			" (" + game_config::svnrev + ")");
	SDL_WM_SetCaption(wm_title_string.c_str(), NULL);

	font::load_font_config();
	hotkey::load_descriptions();

	// To reload the terrain names, we need to reload the configuration file
	editormap new_map(game_config_, map_.write());
	map_ = new_map;

	// Update the selected terrain strings
	palette_.update_selected_terrains();
}

void map_editor::edit_save_as() {
	const std::string default_dir =
		get_dir(get_dir(get_user_data_dir() + "/editor") + "/maps");
	std::string input_name = filename_.empty() ? default_dir : filename_;
	const std::string old_input_name = input_name;

	int res = 0;
	int overwrite = 1;
	do {
		input_name = old_input_name;
		res = dialogs::show_file_chooser_dialog(gui_, input_name, _("Save the Map As"));
		if (res == 0) {

			// Check whether the filename contains illegal characters
			if(!verify_filename(input_name, true))
			{
				input_name = old_input_name;
				continue;
			}
			else if (file_exists(input_name)) {
				overwrite = gui::dialog(gui_, "",
					_("The map already exists. Do you want to overwrite it?"),
					gui::YES_NO).show();
			}
			else
				overwrite = 0;
		}
	} while (res == 0 && overwrite != 0);

	// Try to save the map, if it fails we reset the filename
	if (res == 0) {
		std::string old_file_name = filename_;
		set_file_to_save_as(input_name, from_scenario_);
		if(!save_map("", true))
		{
			filename_ = old_file_name;
		}
	}
}

void map_editor::perform_set_starting_pos() {
	std::vector<std::string> players;

	std::stringstream none_str;
	none_str << _("None");
	players.push_back(none_str.str());

	for (int i = 0; i < gamemap::MAX_PLAYERS; i++) {
		std::stringstream str;
		str << _("Player") << " " << i + 1;
		players.push_back(str.str());
	}
	gui::dialog pmenu = gui::dialog(gui_,
				       _("Which Player?"),
				       _("Which player should start here?"),
				       gui::OK_CANCEL);
	pmenu.set_menu(players);
	int res = pmenu.show();
	if (res >= 0) {
		// We erase previous starting position on this hex.
		// This will prevent to cause a "stack" of these.
		//! @todo TODO: only use 1 undo to revert it (instead of 2)
		const int current_side = starting_side_at(map_, selected_hex_);
		if (current_side != -1) {
			set_starting_position(current_side, gamemap::location());
		}
		if (res > 0) {
			set_starting_position(res, selected_hex_);
		}
	}
}

void map_editor::edit_set_start_pos() {
	left_button_func_changed(SET_STARTING_POSITION);
}

void map_editor::perform_flood_fill(const t_translation::t_terrain fill_with) {
	terrain_log log;
	flood_fill(map_, selected_hex_, fill_with, &log);
	map_undo_action action;
	for (terrain_log::iterator it = log.begin(); it != log.end(); it++) {
		action.add_terrain((*it).second, palette_.selected_fg_terrain(),
			   (*it).first);
		terrain_changed((*it).first);
	}

	save_undo_action(action);
}

void map_editor::edit_flood_fill() {
	left_button_func_changed(FLOOD_FILL);
}

void map_editor::edit_save_map() {
	save_map("", true);
}

void map_editor::edit_quit() {
	set_abort();
}

void map_editor::edit_new_map() {
	const std::string map = new_map_dialog(gui_, palette_.selected_bg_terrain(),
	                                       changed_since_save(), game_config_);
	if (map != "") {
		num_operations_since_save_ = 0;
		clear_undo_actions();
		throw new_map_exception(map);
	}
}

void map_editor::edit_load_map() {
	std::string fn = filename_.empty() ?
		get_dir(get_dir(get_user_data_dir() + "/editor") + "/maps") : filename_;
	int res = dialogs::show_file_chooser_dialog(gui_, fn, _("Choose a Map to Load"));
	if (res == 0) {
		// Check if the mapname contains any illegal characters
		if(!verify_filename(fn, false))	{
			gui::message_dialog(gui_, "", _("Warning: Illegal characters found in the map name. Please save under a different name.")).show();
		}

		std::string new_map = read_file(fn);
		bool scenario;
		if (check_data(new_map, fn, scenario, game_config_)) {
			if (!changed_since_save() || confirm_modification_disposal(gui_)) {
				num_operations_since_save_ = 0;
				clear_undo_actions();
				throw new_map_exception(new_map, fn, scenario);
			}
		}
		else {
			gui::message_dialog(gui_, "", _("The file does not contain a valid map.")).show();
		}
	}
}

void map_editor::edit_fill_selection() {
	map_undo_action undo_action;
	perform_fill_hexes(selected_hexes_, palette_.selected_bg_terrain(), undo_action);
	save_undo_action(undo_action);
}

void map_editor::edit_cut() {
	edit_copy();
	edit_fill_selection();
}

void map_editor::edit_copy() {
	clear_buffer(clipboard_);
	insert_selection_in_clipboard();
}

void map_editor::perform_paste() {
	map_undo_action undo_action;
	paste_buffer(clipboard_, selected_hex_, undo_action);
	save_undo_action(undo_action);
}

void map_editor::edit_paste() {
	left_button_func_changed(PASTE);
}

void map_editor::edit_rotate_selection()
{
	if (selected_hexes_.empty()) {return;}

	// we use the selected hex as center
	gamemap::location center = selected_hex_;
	if (!center.valid()) {
		// except if invalid (e.g the mouse is in menu)
		// then we search the "center of mass" 
		center = gamemap::location(0,0);
		std::set<gamemap::location>::const_iterator it;
		for(it = selected_hexes_.begin(); it != selected_hexes_.end(); it++) {
			center = center + *it;
		}
		center.x = center.x / selected_hexes_.size();
		center.y = center.y / selected_hexes_.size();
	}

	map_buffer buf;
	copy_buffer(buf, selected_hexes_, center);

	std::vector<buffer_item>::iterator it;
	for(it = buf.begin(); it != buf.end(); it++) {
		gamemap::location l(0,0);
		int x = it->offset.x;
		int y = it->offset.y;
		// rotate the X-Y axes to SOUTH/SOUTH_EAST - SOUTH_WEST axes
		// but if x is odd, simply using x/2 + x/2 will lack a step
		l = l.get_direction(gamemap::location::SOUTH, (x+is_odd(x))/2);
		l = l.get_direction(gamemap::location::SOUTH_EAST, (x-is_odd(x))/2 );
		l = l.get_direction(gamemap::location::SOUTH_WEST, y);
		it->offset = l;
	}

	map_undo_action undo_action;
	paste_buffer(buf, center, undo_action);
	save_undo_action(undo_action);
}

void map_editor::edit_revert() {
	std::string new_map = read_file(filename_);
	bool scenario;
	if (check_data(new_map, filename_, scenario, game_config_)) {
		map_undo_action action;
		action.set_map_data(map_.write(), new_map);
		save_undo_action(action);
		throw new_map_exception(new_map, filename_, scenario);
	}
	else {
		gui::message_dialog(gui_, "", _("The file does not contain a valid map.")).show();
	}
}

void map_editor::edit_resize() {

	unsigned width = map_.w(), height = map_.h();
	int x_offset = 0, y_offset = 0;
	bool do_expand = true;
	if(resize_dialog(gui_, width, height, x_offset, y_offset, do_expand)) {

		try {
			// we need the old map data for the undo so store it
			// before the map gets modified.
			const std::string old_data = map_.write();

			const std::string resized_map =
				resize_map(map_, width, height, x_offset, y_offset,
				do_expand, palette_.selected_bg_terrain());

			if (resized_map != "") {
				map_undo_action action;
				action.set_map_data(old_data, resized_map);
				save_undo_action(action);
				throw new_map_exception(resized_map, filename_, from_scenario_);
			}
		} catch (gamemap::incorrect_format_exception& e) {
			std::cerr << "ERROR: " << e.msg_ << '\n';
			gui::message_dialog(gui_, "", e.msg_).show();
		} catch(twml_exception& e) {
			e.show(gui_);
		}
	}
}

void map_editor::edit_flip() {
	const FLIP_AXIS flip_axis = flip_dialog(gui_);
	if (flip_axis != NO_FLIP) {
		const std::string flipped_map = flip_map(map_, flip_axis);
		map_undo_action action;
		action.set_map_data(map_.write(), flipped_map);
		save_undo_action(action);
		throw new_map_exception(flipped_map, filename_, from_scenario_);
	}
}

void map_editor::edit_select_all() {
	if (!all_hexes_selected_) {
		for (int i = 0; i < map_.w(); i++) {
			for (int j = 0; j < map_.h(); j++) {
				selected_hexes_.insert(gamemap::location(i, j));
			}
		}
		all_hexes_selected_ = true;
	}
	else {
		selected_hexes_.clear();
		all_hexes_selected_ = false;
	}
	highlight_selected_hexes();
}

void map_editor::edit_draw() {
	left_button_func_changed(DRAW);
}

void map_editor::edit_refresh() {
	image::flush_cache();
	redraw_everything();
}

void map_editor::edit_update() {
	if (map_dirty_) {
		map_dirty_ = false;
		gui_.rebuild_all();
		gui_.invalidate_all();
		recalculate_starting_pos_labels();
		gui_.recalculate_minimap();
	}
}

void map_editor::edit_auto_update() {
	auto_update_ = !auto_update_;
}

hotkey::ACTION_STATE map_editor::get_action_state(hotkey::HOTKEY_COMMAND command) const {
	if(command == hotkey::HOTKEY_EDIT_AUTO_UPDATE) {
		return (auto_update_) ? hotkey::ACTION_OFF : hotkey::ACTION_ON;
	}
	return command_executor::get_action_state(command);
}

void map_editor::copy_buffer(map_buffer& buffer, const std::set<gamemap::location> &locs, const gamemap::location &origin)
{
	std::set<gamemap::location>::const_iterator it;
	for (it = locs.begin(); it != locs.end(); it++) {
		t_translation::t_terrain terrain = map_.get_terrain(*it);
		buffer.push_back(buffer_item(*it-origin, terrain, starting_side_at(map_, *it)));
	}
}

void map_editor::paste_buffer(const map_buffer &buffer, const gamemap::location &loc, map_undo_action &undo_action)
{
	std::set<gamemap::location> filled;
	std::vector<buffer_item>::const_iterator it;
	for (it = buffer.begin(); it != buffer.end(); it++) {
		//the addition of locations is not commutative !
		gamemap::location target = it->offset + loc;

		if (map_.on_board(target, true)) {
			undo_action.add_terrain(map_.get_terrain(target), it->terrain, target);
			map_.set_terrain(target, it->terrain);
			terrain_changed(target);

			const int start_side = it->starting_side;
			if (start_side != -1) {
				undo_action.add_starting_location(start_side, start_side,
					map_.starting_position(start_side), target);
				map_.set_starting_position(start_side, target);
			}
			filled.insert(target);
		}
	}

	undo_action.set_selection(selected_hexes_, filled);
	selected_hexes_ = filled;
	highlight_selected_hexes(true);
}

void map_editor::insert_selection_in_clipboard() {
	// Find the hex that is closest to the selected one,
	// use this as origin
	gamemap::location origin(1000,1000);
	std::set<gamemap::location>::const_iterator it;
	for (it = selected_hexes_.begin(); it != selected_hexes_.end(); it++) {
		if (distance_between(selected_hex_, *it) <
			distance_between(selected_hex_, origin)) {
			origin = *it;
		}
	}

	copy_buffer(clipboard_, selected_hexes_, origin);
}

void map_editor::perform_fill_hexes(std::set<gamemap::location> &fill_hexes,
	const t_translation::t_terrain terrain, map_undo_action &undo_action) {
	std::set<gamemap::location>::const_iterator it;
	for (it = fill_hexes.begin(); it != fill_hexes.end(); it++) {
		if (map_.on_board(*it, true)) {
			undo_action.add_terrain(map_.get_terrain(*it), terrain, *it);
			map_.set_terrain(*it, terrain);
			terrain_changed(*it);
		}
	}
}

void map_editor::perform_selection_move() {
	map_undo_action undo_action;
	std::set<gamemap::location> old_selection = selected_hexes_;

	map_buffer buf;
	copy_buffer(buf, selected_hexes_, selection_move_start_);
	paste_buffer(buf,selected_hex_, undo_action);

	std::set<gamemap::location>::const_iterator it;
	for (it = old_selection.begin(); it != old_selection.end(); it++) {
		if (selected_hexes_.find(*it) != selected_hexes_.end())
			old_selection.erase(*it);
	}
	perform_fill_hexes(old_selection, palette_.selected_bg_terrain(),undo_action);

	save_undo_action(undo_action);
}


bool map_editor::can_execute_command(hotkey::HOTKEY_COMMAND command, int) const {
	switch (command) {
	case hotkey::HOTKEY_UNDO:
	case hotkey::HOTKEY_REDO:
	case hotkey::HOTKEY_ZOOM_IN:
	case hotkey::HOTKEY_ZOOM_OUT:
	case hotkey::HOTKEY_ZOOM_DEFAULT:
	case hotkey::HOTKEY_FULLSCREEN:
	case hotkey::HOTKEY_SCREENSHOT:
	case hotkey::HOTKEY_TOGGLE_GRID:
	case hotkey::HOTKEY_MOUSE_SCROLL:
	case hotkey::HOTKEY_PREFERENCES:

	case hotkey::HOTKEY_EDIT_SAVE_MAP:
	case hotkey::HOTKEY_EDIT_SAVE_AS:
	case hotkey::HOTKEY_EDIT_QUIT:
	case hotkey::HOTKEY_EDIT_SET_START_POS:
	case hotkey::HOTKEY_EDIT_NEW_MAP:
	case hotkey::HOTKEY_EDIT_LOAD_MAP:
	case hotkey::HOTKEY_EDIT_FLOOD_FILL:
	case hotkey::HOTKEY_EDIT_FILL_SELECTION:
	case hotkey::HOTKEY_EDIT_ROTATE_SELECTION:
	case hotkey::HOTKEY_EDIT_COPY:
	case hotkey::HOTKEY_EDIT_CUT:
	case hotkey::HOTKEY_EDIT_PASTE:
	case hotkey::HOTKEY_EDIT_REVERT:
	case hotkey::HOTKEY_EDIT_RESIZE:
	case hotkey::HOTKEY_EDIT_FLIP:
	case hotkey::HOTKEY_EDIT_SELECT_ALL:
	case hotkey::HOTKEY_EDIT_DRAW:
	case hotkey::HOTKEY_EDIT_REFRESH:
	case hotkey::HOTKEY_EDIT_UPDATE:
	case hotkey::HOTKEY_EDIT_AUTO_UPDATE:
	case hotkey::HOTKEY_LANGUAGE:
		return true;
	default:
		return false;
	}
}

void map_editor::toggle_grid() {
	preferences::set_grid(!preferences::grid());
	gui_.set_grid(preferences::grid());
	gui_.invalidate_all();
}

void map_editor::save_undo_action(const map_undo_action &action) {
	// we skip empty action
	// NOTE: if changing this, improve drawing functions to discard those
	if (action.something_set()) {
		add_undo_action(action);
		num_operations_since_save_++;
	}
}

void map_editor::undo() {
	if(exist_undo_actions()) {
		--num_operations_since_save_;
		map_undo_action action = pop_undo_action();
		if (action.selection_set()) {
			selected_hexes_ = action.undo_selection();
			highlight_selected_hexes(true);
		}
		if (action.terrain_set()) {
			for(std::map<gamemap::location, t_translation::t_terrain>::const_iterator it =
					action.undo_terrains().begin();
				it != action.undo_terrains().end(); ++it) {
				map_.set_terrain(it->first, it->second);
				terrain_changed(it->first);
			}
		}
		if (action.starting_location_set()) {
			for (std::map<gamemap::location, int>::const_iterator it =
					 action.undo_starting_locations().begin();
				 it != action.undo_starting_locations().end(); it++) {
				map_.set_starting_position((*it).second, (*it).first);
			}
			recalculate_starting_pos_labels();
		}
		
		if (action.map_data_set()) {
			throw new_map_exception(action.old_map_data(), filename_, from_scenario_);
		}
	}
}

void map_editor::redo() {
	if(exist_redo_actions()) {
		++num_operations_since_save_;
		map_undo_action action = pop_redo_action();
		if (action.selection_set()) {
			selected_hexes_ = action.redo_selection();
			highlight_selected_hexes(true);
		}
		if (action.terrain_set()) {
			for(std::map<gamemap::location, t_translation::t_terrain>::const_iterator it =
					action.redo_terrains().begin();
				it != action.redo_terrains().end(); ++it) {
				map_.set_terrain(it->first, it->second);
				terrain_changed(it->first);
			}
		}
		if (action.starting_location_set()) {
			for (std::map<gamemap::location, int>::const_iterator it =
					 action.redo_starting_locations().begin();
				 it != action.redo_starting_locations().end(); it++) {
				map_.set_starting_position((*it).second, (*it).first);
			}
			recalculate_starting_pos_labels();
		}
		if (action.map_data_set()) {
			throw new_map_exception(action.new_map_data(), filename_, from_scenario_);
		}
	}
}

void map_editor::preferences() {
	preferences_dialog(gui_, prefs_);
	everything_dirty_ = true;
}

void map_editor::redraw_everything() {
	adjust_sizes(gui_, size_specs_);
	palette_.adjust_size();
	brush_.adjust_size();
	gui_.redraw_everything();
	update_l_button_palette();
	palette_.draw(true);
	brush_.draw(true);
	load_tooltips();
}

void map_editor::highlight_selected_hexes(const bool clear_old) {
	if (clear_old) {
		clear_highlighted_hexes_in_gui();
	}
	for (std::set<gamemap::location>::const_iterator it = selected_hexes_.begin();
		 it != selected_hexes_.end(); it++) {
		gui_.add_highlighted_loc(*it);
	}
}

void map_editor::clear_highlighted_hexes_in_gui() {
	gui_.clear_highlighted_locs();
	highlighted_locs_cleared_ = true;
}

void map_editor::set_mouseover_overlay()
{
	surface image_fg(image::get_image("terrain/" + map_.get_terrain_info(
				palette_.selected_fg_terrain()).editor_image() +
				".png"));
	surface image_bg(image::get_image("terrain/" + map_.get_terrain_info(
				palette_.selected_bg_terrain()).editor_image() +
				".png"));

	if (image_fg == NULL || image_bg == NULL) {
		std::cerr << "Missing terrain icon\n";
		gui_.set_mouseover_hex_overlay(NULL);
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
	const int zoom = static_cast<int>(size * gui_.get_zoom_factor());

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
	gui_.set_mouseover_hex_overlay(image);
}

bool map_editor::changed_since_save() const {
	return num_operations_since_save_ != 0;
}

void map_editor::set_starting_position(const int player, const gamemap::location loc) {
	if(map_.on_board(loc)) {
		map_undo_action action;

		action.add_starting_location(player, player, map_.starting_position(player), loc);
		map_.set_starting_position(player, loc);
		save_undo_action(action);
		recalculate_starting_pos_labels();
	}
	else {
		// If you selected an off-board hex,
		// we just use the standard invalid location
		map_undo_action action;
		action.add_starting_location(player, player, map_.starting_position(player), gamemap::location());
		map_.set_starting_position(player, gamemap::location());
		save_undo_action(action);
		recalculate_starting_pos_labels();
		//gui::message_dialog(gui_, "",
		//		                 _("You must have a hex selected on the board.")).show();
	}
}

void map_editor::set_abort(const ABORT_MODE abort) {
	abort_ = abort;
}

void map_editor::set_file_to_save_as(const std::string filename, bool from_scenario) {
	if (original_filename_.empty())
		original_filename_ = filename;
	filename_ = filename;
	from_scenario_ = from_scenario;
}

void map_editor::left_button_down(const int mousex, const int mousey) {
	const gamemap::location& minimap_loc = gui_.minimap_location_on(mousex,mousey);
	const gamemap::location hex = gui_.hex_clicked_on(mousex, mousey);
	if (minimap_loc.valid()) {
		gui_.scroll_to_tile(minimap_loc,display::WARP,false);
	}
	else if (key_[SDLK_RSHIFT] || key_[SDLK_LSHIFT]) {
		if (key_[SDLK_RALT] || key_[SDLK_LALT]) {
			// Select/deselect a component.
			std::set<gamemap::location> selected;
			selected = get_component(map_, selected_hex_);
			for (std::set<gamemap::location>::const_iterator it = selected.begin();
				 it != selected.end(); it++) {
				if (l_button_held_func_ == ADD_SELECTION) {
					gui_.add_highlighted_loc(*it);
					selected_hexes_.insert(*it);
				}
				else {
					gui_.remove_highlighted_loc(*it);
					selected_hexes_.erase(*it);
				}
			}
			update_mouse_over_hexes(hex);
		}
		else {
			// Select what the brush is over.
			std::vector<gamemap::location> selected;
			selected = get_tiles(map_, hex, brush_.selected_brush_size());
			for (std::vector<gamemap::location>::const_iterator it = selected.begin();
				 it != selected.end(); it++) {
				if (l_button_held_func_ == ADD_SELECTION) {
					gui_.add_highlighted_loc(*it);
					selected_hexes_.insert(*it);
				}
				else {
					gui_.remove_highlighted_loc(*it);
					selected_hexes_.erase(*it);
				}
			}
			update_mouse_over_hexes(hex);
		}
	}
	// If the left mouse button is down and we beforhand have registered
	// a mouse down event, draw terrain at the current location.
	else if (l_button_held_func_ == DRAW_TERRAIN && mouse_moved_) {
		reset_mouseover_overlay();
		draw_on_mouseover_hexes(palette_.selected_fg_terrain());
	}
	else if (l_button_held_func_ == MOVE_SELECTION && mouse_moved_) {
		reset_mouseover_overlay();
		//(*it-selection_move_start_) + hex
		// No other selections should be active when doing this.
		gui_.clear_highlighted_locs();
		std::set<gamemap::location>::const_iterator it;
		for (it = selected_hexes_.begin(); it != selected_hexes_.end(); it++) {
			const gamemap::location hl_loc = (*it-selection_move_start_) + hex;
			if (map_.on_board(hl_loc, true)) {
				gui_.add_highlighted_loc(hl_loc);
			}
		}
	}
}

void map_editor::draw_on_mouseover_hexes(const t_translation::t_terrain terrain) {
	if(map_.on_board(selected_hex_, true)) {
		std::vector<gamemap::location> hexes =
			get_tiles(map_, selected_hex_, brush_.selected_brush_size());
		draw_terrain(terrain, hexes);
	}
}

void map_editor::draw_terrain(const t_translation::t_terrain terrain,
		const std::vector<gamemap::location> &hexes)
{
	map_undo_action undo_action;
	
	for(std::vector<gamemap::location>::const_iterator it = hexes.begin();
			it != hexes.end(); ++it) {
		const t_translation::t_terrain old_terrain = map_.get_terrain(*it);
		if(terrain != old_terrain) {
			undo_action.add_terrain(old_terrain, terrain, *it);
			map_.set_terrain(*it, terrain);
			// always rebuild localy to show the drawing progress
			gui_.rebuild_terrain(*it);
			gui_.invalidate(*it);
			map_dirty_ = true;
		}
	}

	save_undo_action(undo_action);
}

void map_editor::terrain_changed(const gamemap::location &hex)
{
	if (!auto_update_) {
		gui_.rebuild_terrain(hex);
		gui_.invalidate(hex);
	}
	map_dirty_ = true;
}

// These 2 functions are useless now, maybe later?
/*
void map_editor::terrain_changed(const std::vector<gamemap::location> &hexes)
{
	std::vector<gamemap::location>::const_iterator it;
	for (it = hexes.begin(); it != hexes.end(); it++) {
		terrain_changed(*it);
	}
}

void map_editor::terrain_changed(const std::set<gamemap::location> &hexes) {
	std::set<gamemap::location>::const_iterator it;
	for (it = hexes.begin(); it != hexes.end(); it++) {
		terrain_changed(*it);
	}	
}
*/


void map_editor::right_button_down(const int /*mousex*/, const int /*mousey*/) {
	// Draw with the background terrain on rightclick,
	// no matter what operations are wanted with the left button.
	//! @todo TODO evaluate if this is what is the smartest thing to do.
	draw_on_mouseover_hexes(palette_.selected_bg_terrain());
}

void map_editor::middle_button_down(const int mousex, const int mousey) {
	const gamemap::location& minimap_loc = gui_.minimap_location_on(mousex,mousey);
	const gamemap::location hex = gui_.hex_clicked_on(mousex, mousey);
	if (minimap_loc.valid()) {
		gui_.scroll_to_tile(minimap_loc,display::WARP,false);
	}
}

bool map_editor::confirm_exit_and_save() {
	if (!changed_since_save())
		return true;
	if (gui::dialog(gui_, "",
	                     _("Quit Editor"), gui::YES_NO).show() != 0) {
		return false;
	}
	if (gui::dialog(gui_, "",
		             _("Do you want to save the map before quitting?"), gui::YES_NO).show() == 0) {
		if (!save_map("", false)) {
			return false;
		}
	}
	return true;
}

bool map_editor::save_map(const std::string fn, const bool display_confirmation) {
	std::string filename = fn;
	if (filename == "") {
		filename = filename_;
		if(filename == "") {
			edit_save_as();
			return true;
		}
	}
	else {
		filename_ = filename;
	}

	// Check if the filename is correct before saving.
	// We do this twice (also in the 'save as' routine),
	// because a file might already contain illegal characters if loaded.
	if(!verify_filename(filename, display_confirmation)) {
		return false;
	}

	try {
		std::string data;
		if (from_scenario_) {
			data = read_file(original_filename_);
			std::string::size_type start, stop;
			start = data.find("map_data=");
			if (start==std::string::npos)
				return false;
			start += 10;
			stop = data.find('\"', start);
			if (stop==std::string::npos)
				return false;
			data.replace(start, stop-start, map_.write());
		} else {
			data = map_.write();
		}

		write_file(filename, data);
		num_operations_since_save_ = 0;
		if (display_confirmation) {
			gui::message_dialog(gui_, "", _("Map saved.")).show();
		}
		if (from_scenario_)
			original_filename_ = filename;
	}
	catch (io_exception& e) {
		utils::string_map symbols;
		symbols["msg"] = e.what();
		const std::string msg = vgettext("Could not save the map: $msg",symbols);
		gui::message_dialog(gui_, "", msg).show();
		return false;
	}
	return true;
}

bool map_editor::verify_filename(const std::string& filename, bool show_error) const
{
	static const std::string allowed_characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_/\\.";
	static const std::string prefix = "\\/";
	size_t start_pos = filename.find_last_of(prefix);

	if(filename.find_first_not_of(allowed_characters, start_pos) != std::string::npos) {
		if(show_error) {
			gui::message_dialog(gui_, "", _("Error: Illegal character in filename.")).show();
		}
		return false;
	}

	return true;
}
void map_editor::show_menu(const std::vector<std::string>& items, const int xloc,
						   const int yloc, const bool /*context_menu*/) {
	// menu is what to display in the menu.
	if(items.size() == 1) {
		execute_command(hotkey::get_hotkey(items.front()).get_id());
		return;
	}
	static const std::string style = "menu2";
	gui::dialog kmenu = gui::dialog(gui_, "", "", gui::MESSAGE,
						    gui::dialog::hotkeys_style);
	kmenu.set_menu(get_menu_images(items));
	const int res = kmenu.show(xloc, yloc);
	if(res < 0 || static_cast<unsigned>(res) >= items.size())
		return;
	const hotkey::HOTKEY_COMMAND cmd = hotkey::get_hotkey(items[res]).get_id();
	execute_command(cmd);
}

void map_editor::execute_command(const hotkey::HOTKEY_COMMAND command) {
	if (command == hotkey::HOTKEY_QUIT_GAME) {
		set_abort();
	}
	else {
		hotkey::execute_command(gui_, command, this);
	}
}

void map_editor::recalculate_starting_pos_labels() {
	// Remove the current labels.
	for (std::vector<gamemap::location>::const_iterator it = starting_positions_.begin();
		 it != starting_positions_.end(); it++) {
		gui_.labels().set_label(*it, "");
	}
	starting_positions_.clear();
	// Set new labels.
	for (int i = 1; i < gamemap::MAX_PLAYERS+1; i++) {
		gamemap::location loc = map_.starting_position(i);
		if (loc.valid()) {
			std::stringstream ss;
			ss << _("Player") << " " << i;
			gui_.labels().set_label(loc, ss.str());
			starting_positions_.push_back(loc);
		}
	}
}

void map_editor::update_mouse_over_hexes(const gamemap::location mouse_over_hex)
{
	const int size = (l_button_func_ == DRAW) ? brush_.selected_brush_size() : 1;
	std::vector<gamemap::location> curr_locs = get_tiles(map_, mouse_over_hex, size);

	std::set<gamemap::location>::iterator it;
	for (it = mouse_over_hexes_.begin(); it != mouse_over_hexes_.end(); it++) {
		if (selected_hexes_.find(*it) == selected_hexes_.end()) {
			// Only remove highlightning if the hex is not selected
			// in an other way.
			gui_.remove_highlighted_loc(*it);
		}
	}
	mouse_over_hexes_.clear();
	if (mouse_over_hex != gamemap::location()) {
		// Only highlight if the mouse is on the map.
		for (std::vector<gamemap::location>::iterator itv = curr_locs.begin();
			 itv != curr_locs.end(); itv++) {
			mouse_over_hexes_.insert(*itv);
			gui_.add_highlighted_loc(*itv);
		}
	}

	gui_.highlight_hex(mouse_over_hex);

	if(l_button_func_ == DRAW || l_button_func_ == FLOOD_FILL) {
		set_mouseover_overlay();
	}
	selected_hex_ = mouse_over_hex;
}

void map_editor::left_button_func_changed(const LEFT_BUTTON_FUNC func) {
	if (func != l_button_func_) {
		reset_mouseover_overlay();
		l_button_func_ = func;
		gui_.set_report_content(reports::EDIT_LEFT_BUTTON_FUNCTION,
				hotkey::get_hotkey(get_action_name(func)).get_description());
		gui_.invalidate_game_status();
		l_button_palette_dirty_ = true;
	}
}

void map_editor::update_l_button_palette() {
	const theme &t = gui_.get_theme();
	const std::vector<theme::menu> &menus = t.menus();
	std::vector<theme::menu>::const_iterator it;
	for (it = menus.begin(); it != menus.end(); it++) {
		if (is_left_button_func_menu(*it)) {
			const SDL_Rect &r = (*it).location(gui_.screen_area());
			const int draw_x = maximum<int>(r.x - 1, gui_.screen_area().x);
			const int draw_y = maximum<int>(r.y - 1, gui_.screen_area().y);
			const int draw_w = minimum<int>(r.w + 2, gui_.screen_area().w);
			const int draw_h = minimum<int>(r.h + 2, gui_.screen_area().h);
			const SDL_Rect draw_rect = {draw_x, draw_y, draw_w, draw_h};
			SDL_Surface* const screen = gui_.video().getSurface();
			Uint32 color;
			if ((*it).items().back() == get_action_name(l_button_func_)) {
				color = SDL_MapRGB(screen->format,0xFF,0x00,0x00);
			}
			else {
				color = SDL_MapRGB(screen->format,0x00,0x00,0x00);
			}
			draw_rectangle(draw_rect.x, draw_rect.y, draw_rect.w, draw_rect.h,
				       color, gui_.video().getSurface());
			update_rect(draw_rect);
		}
	}
}

std::string map_editor::get_action_name(const LEFT_BUTTON_FUNC func) const {
	switch (func) {
	case DRAW:
		return "editdraw";
	case SELECT_HEXES:
		return ""; // Not implemented yet.
	case FLOOD_FILL:
		return "editfloodfill";
	case SET_STARTING_POSITION:
		return "editsetstartpos";
	case PASTE:
		return "editpaste";
	default:
		return "";
	}
}

bool map_editor::is_left_button_func_menu(const theme::menu &menu) const {
	const std::vector<std::string> &menu_items = menu.items();
	if (menu_items.size() == 1) {
		const std::string item_name = menu_items.back();
		for (size_t i = 0; i < NUM_L_BUTTON_FUNC; i++) {
			if (get_action_name(LEFT_BUTTON_FUNC(i)) == item_name) {
				return true;
			}
		}
	}
	return false;
}

void map_editor::main_loop() {
	unsigned int last_brush_size = brush_.selected_brush_size();
	while (abort_ == DONT_ABORT) {
		int mousex, mousey;
		const int scroll_speed = preferences::scroll_speed();
		Uint8 mouse_flags = SDL_GetMouseState(&mousex,&mousey);
		const bool l_button_down = (0 != (mouse_flags & SDL_BUTTON_LMASK));
		const bool r_button_down = (0 != (mouse_flags & SDL_BUTTON_RMASK));
		const bool m_button_down = (0 != (mouse_flags & SDL_BUTTON_MMASK));

		const gamemap::location cur_hex = gui_.hex_clicked_on(mousex,mousey);
		if (cur_hex != selected_hex_) {
			mouse_moved_ = true;
		}
		if (mouse_moved_ || last_brush_size != brush_.selected_brush_size()
			|| highlighted_locs_cleared_) {
			// The mouse has moved or the brush size has changed,
			// adjust the hexes the mouse is over.
			highlighted_locs_cleared_ = false;
			update_mouse_over_hexes(cur_hex);
			last_brush_size = brush_.selected_brush_size();
		}
		const theme::menu* const m = gui_.menu_pressed();
		if (m != NULL) {
			const SDL_Rect& menu_loc = m->location(gui_.screen_area());
			const int x = menu_loc.x + 1;
			const int y = menu_loc.y + menu_loc.h + 1;
			show_menu(m->items(), x, y, false);
		}

		if(key_[SDLK_UP] || mousey == 0) {
			gui_.scroll(0,-scroll_speed);
		}
		if(key_[SDLK_DOWN] || mousey == gui_.h()-1) {
			gui_.scroll(0,scroll_speed);
		}
		if(key_[SDLK_LEFT] || mousex == 0) {
			gui_.scroll(-scroll_speed,0);
		}
		if(key_[SDLK_RIGHT] || mousex == gui_.w()-1) {
			gui_.scroll(scroll_speed,0);
		}

		if (l_button_down) {
			left_button_down(mousex, mousey);
		}
		else {
			if (l_button_held_func_ == MOVE_SELECTION) {
				// When it is detected that the mouse is no longer down
				// and we are in the progress of moving a selection,
				// perform the movement.
				perform_selection_move();
			}
		}
		if (r_button_down) {
			right_button_down(mousex, mousey);
		}
		if (m_button_down) {
			middle_button_down(mousex, mousey);
		}

		gui_.draw(false);
		events::raise_draw_event();

		// When the map has changed, wait until the left mouse button
		// is not held down, and then update the minimap and
		// the starting position labels.
		if (map_dirty_) {
			if (!l_button_down && !r_button_down) {
				if (auto_update_) {
					gui_.rebuild_all();
					gui_.invalidate_all();
					map_dirty_ = false;
				}
				gui_.recalculate_minimap();
				recalculate_starting_pos_labels();
			}
		}
		if (l_button_palette_dirty_) {
			update_l_button_palette();
			l_button_palette_dirty_ = false;
		}
		gui_.update_display();
		SDL_Delay(sdl_delay);
		events::pump();
		if (everything_dirty_) {
			redraw_everything();
			everything_dirty_ = false;
		}
		if (abort_ == ABORT_NORMALLY) {
			if (!confirm_exit_and_save()) {
				set_abort(DONT_ABORT);
			}
		}
		mouse_moved_ = false;
	}
}
} // end namespace map_editor


