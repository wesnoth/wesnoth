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

#include "../display.hpp"
#include "../events.hpp"

#include <map>

namespace map_editor {

/// A saved action that may be undone.
struct map_undo_action {
  map_undo_action(const gamemap::TERRAIN& old_tr,
		  const gamemap::TERRAIN& new_tr,
		  const gamemap::location& lc)
    : old_terrain(old_tr), new_terrain(new_tr), location(lc) {}
  gamemap::TERRAIN old_terrain;
  gamemap::TERRAIN new_terrain;
  gamemap::location location;
};
typedef std::deque<map_undo_action> map_undo_list;

class map_editor;

/// A class that receives events for a map editor.
class editor_event_handler : public events::handler {
public:
  /// Constructor.
  /// editor is the map editor object on which operations should be
  /// performed when events are received.
  editor_event_handler(map_editor &editor);

  virtual void handle_event(const SDL_Event &event);

  /// Handle a keyboard event. mousex and mousey is the current position
  /// of the mouse.
  void handle_keyboard_event(const SDL_KeyboardEvent &event,
			     const int mousex, const int mousey);

  /// Handle a mouse button event. mousex and mousey is the current
  /// position of the mouse.
  void handle_mouse_button_event(const SDL_MouseButtonEvent &event,
				 const int mousex, const int mousey);

private:
  map_editor &editor_;
};

/// Size specifications for the map editor.
// This should be replaced by theme handling.
struct size_specs {
  size_t nterrains;
  size_t terrain_size;
  size_t terrain_padding;
  size_t terrain_space;
  size_t palette_x;
  size_t button_x;
  size_t top_button_y;
  size_t palette_y;
  size_t bot_button_y;
};

/// How to abort the map editor.
/// DONT_ABORT is set during normal operation.
/// When ABORT_NORMALLY is set, the editor asks for confirmation and
/// if save is desired before it exits. 
/// When ABORT_HARD is set, the editor exists without asking any
/// questions or saving.
enum ABORT_MODE {DONT_ABORT, ABORT_NORMALLY, ABORT_HARD};

/// A map editor.
class map_editor {
public:
  map_editor(display &gui, gamemap &map);

  /// Enter the main loop. The loop runs until set_abort() is called to
  /// set an abort mode which makes the loop exit.
  void main_loop();

  /// Set the filename that map should be saved as. 
  void set_file_to_save_as(const std::string);

  void undo();
  void redo();

  void zoom_in();
  void zoom_out();
  void zoom_default();

  /// Set the abort flag, which indicates if the editor should exit in
  /// some way after the current iteration.
  void set_abort(const ABORT_MODE abort=ABORT_NORMALLY);

  /// Set the starting position for the given player to the location
  /// indicated by the x and y coordinates.
  void set_starting_position(const int player, const int x, const int y);

  /// Scroll the terrain palette down one step if possible.
  void scroll_palette_down();
  /// Scroll the terrain palette up one step if possible.
  void scroll_palette_up();

  /// Save the current map. If filename is an empty string, use the
  /// filename that is set with set_file_to_save_as(). Return false if
  /// the save failed.
  bool save_map(const std::string filename="",
		const bool display_confirmation=true);

  /// Adjust the internal size specifications to fit the display.
  // This should be replaced by theme handling.
  void adjust_sizes(const display &disp);

private:
  /// Called in every iteration when the left mouse button is held
  /// down. Note that this differs from a click.
  void left_button_down_(const int mousex, const int mousey);

  /// Called in every iteration when the right mouse button is held
  /// down. Note that this differs from a click.
  void right_button_down_(const int mousex, const int mousey);

  /// Called in every iteration when the middle mouse button is held
  /// down. Note that this differs from a click.
  void middle_button_down_(const int mousex, const int mousey);

  /// Confirm that exiting is desired and ask for saving of the map.
  /// Return true if exit is confirmed and the save is successful or not
  /// wanted. Return false if exit is cancelled or the requested save
  /// failed.
  bool confirm_exit_and_save_();

  display &gui_;
  gamemap &map_;
  std::vector<gamemap::TERRAIN> terrains_;
  gui::button tup_, tdown_;
  gamemap::TERRAIN selected_terrain_;
  map_undo_list undo_stack_;
  map_undo_list redo_stack_;
  std::string filename_;
  ABORT_MODE abort_;
  unsigned int tstart_;
  // Keep track of the number of operations performed since the last
  // save. If this is zero when the editor is exited there is no need to
  // ask the user to save.
  unsigned int num_operations_since_save_;
  size_specs size_specs_;
  CKey key_;
};

/// Display a dialog with map filenames and return the chosen
/// one. Create a temporary display to use.
std::string get_filename_from_dialog(CVideo &video, config &cfg);
void drawbar(display& disp);
bool drawterrainpalette(display& disp, int start, gamemap::TERRAIN selected,
			gamemap map, size_specs specs);

/// Return the number of the tile that is at coordinates (x, y).
int tile_selected(const unsigned int x, const unsigned int y,
		  const display& disp, const size_specs specs);
		  
bool is_invalid_terrain(char c);


}

#endif // EDITOR_H_INCLUDED
