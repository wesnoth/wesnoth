/* $Id$ */
/*
  Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
  Part of the Battle for Wesnoth Project http://www.wesnoth.org/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2
  or at your option any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the COPYING file for more details.
*/
#ifndef EDITOR_H_INCLUDED
#define EDITOR_H_INCLUDED

#include "global.hpp"

#include "SDL.h"

#include "editor_palettes.hpp"
#include "editor_layout.hpp"
#include "editor_undo.hpp"
#include "map_manip.hpp"

#include "../display.hpp"
#include "../events.hpp"
#include "../font.hpp"
#include "../hotkeys.hpp"
#include "../preferences_display.hpp"
#include "../theme.hpp"
#include "../tooltips.hpp"
//#include "gui/key_listener.hpp"
//#include "gui/mouse_listener.hpp"

#include <map>
#include <queue>
#include <set>
#include <vector>

namespace map_editor {

bool check_data(std::string &data, std::string &filename, bool &from_scenario, config &game_cfg);

/// A map editor. Receives SDL events and can execute hotkey commands.
class map_editor : public events::handler,
				   public hotkey::command_executor {
public:
	map_editor(editor_display &gui, editormap &map, config &theme, config &game_config);
	virtual ~map_editor();

	/// Enter the main loop. The loop runs until set_abort() is called
	/// to set an abort mode which makes the loop exit.
	void main_loop();

	/// Set the filename that map should be saved as.
	void set_file_to_save_as(const std::string, bool from_scenario);

	/// How to abort the map editor.
	/// DONT_ABORT is set during normal operation.
	/// When ABORT_NORMALLY is set, the editor asks for confirmation and
	/// if save is desired before it exits.
	/// When ABORT_HARD is set, the editor exists without asking any
	/// questions or saving.
	enum ABORT_MODE {DONT_ABORT, ABORT_NORMALLY, ABORT_HARD};

	/// Set the abort flag, which indicates if the editor should exit in
	/// some way after the current iteration of the main loop.
	void set_abort(const ABORT_MODE abort=ABORT_NORMALLY);

	/// Save the current map. If filename is an empty string, use the
	/// filename that is set with set_file_to_save_as(). A message box
	/// that shows confirmation that the map was saved is shown if
	/// display_confirmation is true. Return false if the save failed.
	bool save_map(const std::string filename="",
				  const bool display_confirmation=true);

	/// Exception thrown when the loading of a map failed.
	struct load_map_exception {};

	virtual void handle_event(const SDL_Event &event);

	/// Handle a keyboard event. mousex and mousey is the current
	/// position of the mouse.
	void handle_keyboard_event(const SDL_KeyboardEvent &event,
							   const int mousex, const int mousey);

	/// Handle a mouse button event. mousex and mousey is the current
	/// position of the mouse.
	void handle_mouse_button_event(const SDL_MouseButtonEvent &event,
								   const int mousex, const int mousey);

	/// Return true if the map has changed since the last time it was
	/// saved.
	bool changed_since_save() const;

	/// Recalculate layout and redraw everything.
	void redraw_everything();

	// Change the language in_game
	virtual void change_language();

	// Methods inherited from command_executor. Used to perform
	// operations on menu/hotkey commands.
	virtual void toggle_grid();
	virtual void undo();
	virtual void redo();
	virtual void preferences();
	virtual void edit_quit();
	virtual void edit_new_map();
	virtual void edit_load_map();
	virtual void edit_save_map();
	virtual void edit_save_as();
	/// Display a dialog asking for a player number and set the starting
	/// position of the given player to the currently selected hex.
	virtual void edit_set_start_pos();
	virtual void edit_flood_fill();
	virtual void edit_fill_selection();
	virtual void edit_rotate_selection();
	virtual void edit_cut();
	virtual void edit_copy();
	virtual void edit_paste();
	virtual void edit_revert();
	virtual void edit_resize();
	virtual void edit_flip();
	/// Either select or deselect all hexes on the map depending on if
	/// this operations has been invoked before or not.
	virtual void edit_select_all();
	virtual void edit_draw();
	virtual void edit_refresh();
	virtual void edit_update();
	virtual void edit_auto_update();

	void perform_flood_fill(const t_translation::t_letter fill_with);
	void perform_paste();
	void perform_set_starting_pos();

	virtual bool can_execute_command(hotkey::HOTKEY_COMMAND command, int) const;
	//Does the action control a toggle switch? If so, return the state of the action (on or off)
	virtual hotkey::ACTION_STATE get_action_state(hotkey::HOTKEY_COMMAND command) const;


	/// Exception thrown when new map is to be loaded.
	struct new_map_exception {
		new_map_exception(const std::string &map, const std::string filename="", const bool scenario = false)
			: new_map(map), new_filename(filename), from_scenario(scenario) {}
		const std::string new_map;
		const std::string new_filename;
		const bool from_scenario;
	};

private:
	/// What to perform while the left button is held down.
	enum LEFT_BUTTON_HELD_FUNC {DRAW_TERRAIN, ADD_SELECTION, REMOVE_SELECTION,
								MOVE_SELECTION, NONE};

	/// What to perform on a left button click.
	enum LEFT_BUTTON_FUNC {DRAW, SELECT_HEXES, FLOOD_FILL,
						   SET_STARTING_POSITION, PASTE, NUM_L_BUTTON_FUNC};

	/// Called in every iteration when the left mouse button is held
	/// down. Note that this differs from a click.
	void left_button_down(const int mousex, const int mousey);

	/// Handle a left click on the location.
	void left_click(const gamemap::location loc);

	/// Called in every iteration when the right mouse button is held
	/// down. Note that this differs from a click.
	void right_button_down(const int mousex, const int mousey);

	/// Handle a right click on the location.
	void right_click(const gamemap::location loc);

	/// Called in every iteration when the middle mouse button is held
	/// down. Note that this differs from a click.
	void middle_button_down(const int mousex, const int mousey);

	/// Confirm that exiting is desired and ask for saving of the map.
	/// Return true if exit is confirmed and the save is successful or not
	/// wanted. Return false if exit is cancelled or the requested save
	/// failed.
	bool confirm_exit_and_save();

	/// Set the starting position for the given player to the location
	/// given.
	void set_starting_position(const int player, const gamemap::location loc);

	/// Check whether the filename contains illegal characters.
	bool verify_filename(const std::string& filename, bool show_error) const;

	/// Display a menu with given items and at the given location.
	void show_menu(const std::vector<std::string>& items_arg, const int xloc,
				   const int yloc, const bool context_menu=false);

	/// Pass the command onto the hotkey handling system. Quit requests
	/// are intercepted because the editor does not want the default
	/// behavior of those.
	void execute_command(const hotkey::HOTKEY_COMMAND command);

	/// Draw terrain at a location. The operation is saved in the undo
	/// stack. Update the map to reflect the change.
	void draw_terrain(const t_translation::t_letter terrain,
					  const gamemap::location hex);


	/////////////////////////////////////////////////////////////////////
	// NOTE: after any terrain has changed, one of the invalidate      //
	// methods must be called with that location among the arguments.  //
	/////////////////////////////////////////////////////////////////////


	/// Invalidate the given hex and all the adjacent ones. Assume the
	/// hex has changed, so rebuild the dynamic terrain at the hex and
	/// the adjacent hexes.
	void invalidate_adjacent(const gamemap::location hex);

	/// Invalidate the hexes in the give vector and the ones that are
	/// adjacent. Rebuild the terrain on the same hexes. Make sure that
	/// the operations only happen once per hex for efficiency purposes.
	void invalidate_all_and_adjacent(const std::vector<gamemap::location> &hexes);
	void invalidate_all_and_adjacent(const std::set<gamemap::location> &hexes);

	/// Re-set the labels for the starting positions of the
	/// players. Should be called when the terrain has changed, which
	/// may have changed the starting positions.
	void recalculate_starting_pos_labels();

	/// Update the selection and highlightning of the hexes the mouse
	/// currently is over.
	void update_mouse_over_hexes(const gamemap::location mouse_over_hex);

	/// Insert the currently selected locations in the clipboard.
	void insert_selection_in_clipboard();

	/// Commit hexes filling.
	void perform_fill_hexes(std::set<gamemap::location> &fill_hexes,
		const t_translation::t_letter terrain, map_undo_action &undo_action);
	
	/// Commit the movement of a selection.
	void perform_selection_move();

	/// Highlight the currently selected hexes. If clear_old is true the
	/// old highlighting is cleared, otherwise the current selection is
	/// only added, which may leave old selected terrain still
	/// highlighted.
	void highlight_selected_hexes(const bool clear_old=true);

	/// Clear the highlighted hexes in the gui and set a variable to
	/// indicate this so that the brush size highlighting may be
	/// refreshed.
	void clear_highlighted_hexes_in_gui();

	// Sets the special overlay with the selection of fore and background
	void set_mouseover_overlay();
	// Clears the special overlay
	void reset_mouseover_overlay() { gui_.clear_mouseover_hex_overlay(); }

	/// Terrain has changed at the specified hex through user drawing
	/// (not undo/redo or other special things).
	void terrain_changed(const gamemap::location &hex);
	void terrain_changed(const std::vector<gamemap::location> &hexes);
	void terrain_changed(const std::set<gamemap::location> &hexes);

	/// Save an action so that it may be undone. Add an operation to the
	/// number done since save.
	void save_undo_action(const map_undo_action &action);

	/// Call when the left mouse button function has changed. Updated
	/// the report indicating what will be performed. New_function is
	/// the hotkey-string describing the action.
	void left_button_func_changed(const LEFT_BUTTON_FUNC func);

	/// Draw black squares around the buttons that are used to select
	/// the left button function and draw a read square around the
	/// currently selected function.
	void update_l_button_palette();

	/// Return the hotkey-string representing the left button
	/// function. The "action_" is left out.
	std::string get_action_name(const LEFT_BUTTON_FUNC func) const;

	/// Return true if the menu is a button used for setting the left
	/// mouse button function.
	bool is_left_button_func_menu(const theme::menu &menu) const;

	/// Draw the terrain on the hexes the mouse is over, taking account
	/// for brush size.
	void draw_on_mouseover_hexes(const t_translation::t_letter t);

	// Load the tooltips for each button
	void load_tooltips(void);

	/// An item in the buffer. Consists of the copied terrain and an
	/// offset. When pasting stuff, the offset is used to calculate
	/// where to put the pasted hex
	struct buffer_item {
		buffer_item(const gamemap::location &o, t_translation::t_letter t, int start_side) :
			offset(o), terrain(t), starting_side(start_side) {}
		gamemap::location offset;
		t_translation::t_letter terrain;
		int starting_side;
	};

	// map_buffer is used for clipboard and other map operations
	typedef std::vector<buffer_item> map_buffer;

	void copy_buffer(map_buffer& buffer, const std::set<gamemap::location> &locs,
			 const gamemap::location &origin);
 	void paste_buffer(const map_buffer& buffer, const gamemap::location &loc,
 			map_undo_action &undo_action);
 	void clear_buffer(map_buffer& buffer) {buffer.clear();};

	editor_display &gui_;
	editormap &map_;
	std::string filename_, original_filename_;
	bool from_scenario_;
	ABORT_MODE abort_;
	// Keep track of the number of operations performed since the last
	// save. If this is zero when the editor is exited there is no need
	// to ask the user to save.
	static int num_operations_since_save_;
	size_specs size_specs_;
	config &theme_;
	config &game_config_;
	CKey key_;
	gamemap::location selected_hex_;
	// When map_dirty_ is true, schedule redraw of the minimap and
	// perform some updates like recalculating labels of starting
	// positions. But only it the option auto_update is enabled
	bool map_dirty_;
	bool auto_update_;
	bool l_button_palette_dirty_;
	bool everything_dirty_;
	terrain_palette palette_;
	brush_bar brush_;
	std::vector<gamemap::location> starting_positions_;
	std::set<gamemap::location> mouse_over_hexes_;
	std::set<gamemap::location> selected_hexes_;
	map_buffer clipboard_;
	
	LEFT_BUTTON_HELD_FUNC l_button_held_func_;
	gamemap::location selection_move_start_;

	tooltips::manager tooltip_manager_;
	font::floating_label_context floating_label_manager_;
	// mouse_moved_ will be true if the mouse have moved between two
	// cycles.
	bool mouse_moved_;
	bool highlighted_locs_cleared_;
	const hotkey::manager hotkey_manager_;
	const preferences::display_manager prefs_disp_manager_;
	static config prefs_;
	static config hotkeys_;
	static bool first_time_created_;
	static LEFT_BUTTON_FUNC l_button_func_;
	static t_translation::t_letter old_fg_terrain_, old_bg_terrain_;
	static int old_brush_size_;
	bool all_hexes_selected_;

};

}

#endif // EDITOR_H_INCLUDED

