/*
  Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
  Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the COPYING file for more details.
*/
#ifndef EDITOR_H_INCLUDED
#define EDITOR_H_INCLUDED

#include "editor_palettes.hpp"
#include "editor_layout.hpp"

#include "../display.hpp"
#include "../events.hpp"
#include "../hotkeys.hpp"
#include "../preferences.hpp"

#include <map>
#include <queue>
#include <set>
#include <vector>

namespace map_editor {

/// A saved action that may be undone.
class map_undo_action
{
public:
	map_undo_action() {
	}

	map_undo_action(const gamemap::TERRAIN& old_tr,
			const gamemap::TERRAIN& new_tr,
			const gamemap::location& lc){
		add_terrain(old_tr, new_tr, lc);
	}

 	const std::map<gamemap::location,gamemap::TERRAIN>& undo_terrains() const {
 		return old_terrain_;
 	}

 	const std::map<gamemap::location,gamemap::TERRAIN>& redo_terrains() const {
 		return new_terrain_;
 	}

	const std::set<gamemap::location> undo_selection() const {
		return old_selection_;
	}
	const std::set<gamemap::location> redo_selection() const {
		return new_selection_;
	}

	void add_terrain(const gamemap::TERRAIN& old_tr,
		 const gamemap::TERRAIN& new_tr,
		 const gamemap::location& lc) {
		old_terrain_[lc] = old_tr;
		new_terrain_[lc] = new_tr;
	}

	void set_selection(const std::set<gamemap::location> &new_selection,
					   const std::set<gamemap::location> &old_selection) {
		new_selection_ = new_selection;
		old_selection_ = old_selection;
	}

private:
	std::map<gamemap::location,gamemap::TERRAIN> old_terrain_;
	std::map<gamemap::location,gamemap::TERRAIN> new_terrain_;
	std::set<gamemap::location> new_selection_;
	std::set<gamemap::location> old_selection_;
};

typedef std::deque<map_undo_action> map_undo_list;


/// A map editor. Receives SDL events and can execute hotkey commands.
class map_editor : public events::handler,
				   public hotkey::command_executor {
public:
	map_editor(display &gui, gamemap &map, config &theme, config &game_config);
	virtual ~map_editor();
	
	/// Enter the main loop. The loop runs until set_abort() is called
	/// to set an abort mode which makes the loop exit.
	void main_loop();
	
	/// Set the filename that map should be saved as. 
	void set_file_to_save_as(const std::string);

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

	/// Load the map from filename. Return the string representation of
	/// the map, or the empty string of the load failed.
	std::string load_map(const std::string filename);
	
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
	virtual void edit_cut();
	virtual void edit_copy();
	virtual void edit_paste();
	virtual void edit_revert();
	virtual void edit_resize();

	virtual bool can_execute_command(hotkey::HOTKEY_COMMAND command) const;
	
	/// Exception thrown when new map is to be loaded.
	struct new_map_exception {
		new_map_exception(const std::string &map, const std::string filename="")
			: new_map(map), new_filename(filename) {}
		const std::string new_map;
		const std::string new_filename;
	};
	
private:
	/// Called in every iteration when the left mouse button is held
	/// down. Note that this differs from a click.
	void left_button_down(const int mousex, const int mousey);
	
	/// Called in every iteration when the right mouse button is held
	/// down. Note that this differs from a click.
	void right_button_down(const int mousex, const int mousey);
	
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
	
	/// Display a menu with given items and at the given location.
	void show_menu(const std::vector<std::string>& items_arg, const int xloc,
				   const int yloc, const bool context_menu=false);
	
	/// Pass the command onto the hotkey handling system. Quit requests
	/// are intercepted because the editor does not want the default
	/// behavior of those.
	void execute_command(const hotkey::HOTKEY_COMMAND command);
	
	/// Draw terrain at a location. The operation is saved in the undo
	/// stack. Update the map to reflect the change.
	void draw_terrain(const gamemap::TERRAIN terrain,
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

	/// Add an undo action to the undo stack. Resize the stack if it
	/// gets larger than the maximum size. Add an operation to the
	/// number done since save. If keep_selection is true, it indicates
	/// that the selection has not changed and the currently selected
	/// terrain should be kept if this action is redone/undone. Also
	/// clear the redo stack.
	void add_undo_action(map_undo_action &action, const bool keep_selection=true);

	/// Update the selection and highlightning of the hexes the mouse
	/// currently is over.
	void update_mouse_over_hexes(const gamemap::location mouse_over_hex);

	/// Insert the currently selected locations in the clipboard.
	void insert_selection_in_clipboard();

	/// Return the hex with the given offset from loc. Make calculations
	/// so the result with have the same _appearance_ as when the offset
	/// was calculated ,not the same representation.
	gamemap::location get_hex_with_offset(const gamemap::location loc,
										  const int x_offset, const int y_offset);

	/// Commit the movement of a selection.
	void perform_selection_move();

	/// Highlight the currently selected hexes. If clear_old is true the
	/// old highlighting is cleared, otherwise the current selection is
	/// only added, which may leave old selected terrain still
	/// highlighted.
	void highlight_selected_hexes(const bool clear_old=true);

	/// An item in the clipboard. Consists of the copied terrain and an
	/// offset. When pasting stuff, the offset is used to calculate
	/// where to put the pasted hex when calculating from the one
	/// selected when the paste takes place.
	struct clipboard_item {
		clipboard_item(int xo, int yo, gamemap::TERRAIN t) :
			x_offset(xo), y_offset(yo), terrain(t) {}
		int x_offset, y_offset;
		gamemap::TERRAIN terrain;
	};

	/// What to perform while the left button is held down.
	enum LEFT_BUTTON_FUNC {DRAW_TERRAIN, ADD_SELECTION, REMOVE_SELECTION,
						   MOVE_SELECTION, NONE};

	display &gui_;
	gamemap &map_;
	map_undo_list undo_stack_;
	map_undo_list redo_stack_;
	std::string filename_;
	ABORT_MODE abort_;
	// Keep track of the number of operations performed since the last
	// save. If this is zero when the editor is exited there is no need
	// to ask the user to save.
	int num_operations_since_save_;
	size_specs size_specs_;
	config &theme_;
	config &game_config_;
	CKey key_;
	gamemap::location selected_hex_;
	// When map_dirty_ is true, schedule redraw of the minimap and
	// perform some updates like recalculating labels of starting
	// positions.
	bool map_dirty_;
	terrain_palette palette_;
	brush_bar brush_;
	std::vector<gamemap::location> starting_positions_;
	std::set<gamemap::location> mouse_over_hexes_;
	std::set<gamemap::location> selected_hexes_;
	std::vector<clipboard_item> clipboard_;
	gamemap::location clipboard_offset_loc_;
	LEFT_BUTTON_FUNC l_button_func_;
	gamemap::location selection_move_start_;
	// mouse_moved_ will be true if the mouse have moved between two
	// cycles.
	bool mouse_moved_;
	const preferences::display_manager prefs_disp_manager_;
	config prefs_;
	
};

}

#endif // EDITOR_H_INCLUDED

