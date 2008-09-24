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

#ifndef EDITOR2_EDITOR_CONTROLLER_HPP_INCLUDED
#define EDITOR2_EDITOR_CONTROLLER_HPP_INCLUDED

#include "action_base.hpp"
#include "brush.hpp"
#include "editor_common.hpp"
#include "editor_display.hpp"
#include "editor_main.hpp"
#include "map_context.hpp"
#include "map_fragment.hpp"

#include "../controller_base.hpp"
#include "../events.hpp"
#include "../hotkeys.hpp"
#include "../key.hpp"
#include "../mouse_handler_base.hpp"
#include "../sdl_utils.hpp"
#include "../tooltips.hpp"

#include <deque>
#include <boost/utility.hpp>

class config;
class map_generator;

namespace tooltips {
class manager;
}

namespace font {
class floating_label_context;
}

namespace rand_rng {
class rng;
class set_random_generator;
}

namespace editor2 {

class brush_bar;
class size_specs;
class terrain_palette;
class editor_map;

/**
 * The editor_controller class containts the mouse and keyboard event handling
 * routines for the editor. It also serves as the main editor class with the 
 * general logic.
 */
class editor_controller : public controller_base, 
	public events::mouse_handler_base,
	private boost::noncopyable
{
	public:
		editor_controller(const config &game_config, CVideo& video, editor_map* init_map = NULL);
		~editor_controller();
		
		EXIT_STATUS main_loop();
		
		/**
		 * Process a hotkey quit command
		 */
		void hotkey_quit();
		
		/**
		 * Show a quit confirmation dialog and if confirmed quit with the given exit status
		 */
		void quit_confirm(EXIT_STATUS status);
		
		void editor_settings_dialog();
		
		/**
		 * Shows an are-you-sure dialog if the map was modified.
		 * @return true if the user confirmed or the map was not modified, false otherwise
		 */
		bool confirm_discard();
		
		/** Get the current map context object */
		map_context& get_map_context() { return *map_context_; }
		/** Get the current map context object - const version */
		const map_context& get_map_context() const { return *map_context_; }
		/** Get the map from the current map context object */
		editor_map& get_map() { return get_map_context().get_map(); }
		/** Get the map from the current map context object - const version*/
		const editor_map& get_map() const { return get_map_context().get_map(); }
		
		/** Display a load map dialog and process user input. */
		void load_map_dialog();

		/** Display a new map dialog and process user input. */
		void new_map_dialog();

		/** Display a save map as dialog and process user input. */
		void save_map_as_dialog();

		/** Display a generate random map dialog and process user input. */
		void generate_map_dialog();

		/** Display a load map dialog and process user input. */
		void resize_map_dialog();
		
		/**
		 * Save the map under a given filename.
		 * @return true on success
		 */
		bool save_map_as(const std::string& filename);
		
		/**
		 * Save the map under a given filename. Displays an error message on failure.
		 * @return true on success
		 */
		bool save_map(bool display_confirmation = false);
		
		/**
		 * Create a new map.
		 */
		void new_map(int width, int height, t_translation::t_terrain fill);
		
		/**
		 * Load a map given the filename
		 */
		void load_map(const std::string& filename);
		
		/**
		 * Revert the map by reloading it from disk
		 */
		void revert_map();
				
		/**
		 * Reload the map after ot has significantly changed (when e.g. the dimensions changed).
		 * This is necessary to avoid issues with parts of the map being cached in the display class.
		 */
		void reload_map();
		
		/** command_executor override */
		bool can_execute_command(hotkey::HOTKEY_COMMAND, int index = -1) const;
		
		/** command_executor override */
		hotkey::ACTION_STATE get_action_state(hotkey::HOTKEY_COMMAND command) const;
		
		/** command_executor override */
		bool execute_command(hotkey::HOTKEY_COMMAND command, int index = -1);

		/** WIP idea for context-menu starting position setting*/
		void expand_starting_position_menu(std::vector<std::string>& items);
		
		/** controller_base override */
		void show_menu(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu);
		
		/** Cycle to the next brush. */
		void cycle_brush();
		
		/** Show the preferences dialog */
		void preferences();
		
		/** Grid toggle */
		void toggle_grid();
		
		/** Copy the selection on the current map to the clipboard */
		void copy_selection();

		/** Cut the selection from the current map to the clipboard */
		void cut_selection();
		
		/** Fill the selection with the foreground terrain */
		void fill_selection();
		
		/**
		 * Set the current mouse action based on a hotkey id
		 */
		void hotkey_set_mouse_action(hotkey::HOTKEY_COMMAND command);
		
		/**
		 * @return true if the mouse action identified by the hotkey is active
		 */
		bool is_mouse_action_set(hotkey::HOTKEY_COMMAND command) const;
		
		void update_mouse_action_highlights();
	
		/* mouse_handler_base overrides */
		void mouse_motion(int x, int y, const bool browse, bool update);
		editor_display& gui() { return *gui_; }
		const editor_display& gui() const { return *gui_; }
		bool allow_mouse_wheel_scroll(int x, int y);
		bool right_click_show_menu(int x, int y, const bool browse);
		bool left_click(int x, int y, const bool browse);
		void left_drag_end(int x, int y, const bool browse);
		void left_mouse_up(int x, int y, const bool browse);
		bool right_click(int x, int y, const bool browse);
		void right_drag_end(int x, int y, const bool browse);
		void right_mouse_up(int x, int y, const bool browse);
		
		void set_mouseover_overlay();
		void clear_mouseover_overlay();
		
	protected:
		/* controller_base overrides */
		void process_keyup_event(const SDL_Event& event);
		mouse_handler_base& get_mouse_handler_base();
		editor_display& get_display();	
		
		/** Get the current brush */
		brush* get_brush();
		
		/** Get the current mouse action */
		mouse_action* get_mouse_action();
		
		/**
		 * Perform an action, then delete the action object.
		 * The pointer can be NULL, in which case nothing will happen.
		 */
		void perform_delete(editor_action* action);
		
		/**
		 * Peform an action on the current map_context, then refresh the display 
		 * and delete the pointer. The pointer can be NULL, in which case nothing will happen.
		 */
		void perform_refresh_delete(editor_action* action, bool drag_part = false);
		
		/**
		 * Peform an action on the current map_context, then refresh the display.
		 */
		void perform_refresh(const editor_action& action, bool drag_part = false);
		
		void editor_settings_dialog_redraw_callback(int r, int g, int b);
	private:    		
		/** init the display object and general set-up */ 
		void init_gui(CVideo& video);
		
		/** init the sidebar objects */
		void init_sidebar(const config& game_config);
		
		/** init the brushes */
		void init_brushes(const config& game_config);

		/** init the mouse actions (tools) */
		void init_mouse_actions(const config& game_config);
		
		/** init available random map generators */
		void init_map_generators(const config& game_config);
		
		/** init the available time-of-day settings */
		void init_tods(const config& game_config);
		
		/** Load editor-specific tooltips */
		void load_tooltips();
		
		void redraw_toolbar();
		
		/** Reload images */
		void refresh_image_cache();
		
		/**
		 * Refresh the display after an action has been performed. 
		 * The map context contains details of what needs to be refreshed.
		 */
		void refresh_after_action(bool drag_part = false);
		
		/**
		 * Refresh everything unconditionally
		 */
		void refresh_all();
		
		/**
		 * Callback function passed to display to be called on each redraw_everything run.
		 * Redraws toolbar, brush bar and related items.
		 */
		void display_redraw_callback(display&);
		
		/**
		 * Undos an action in the current map context
		 */
		void undo();

		/**
		 * Redos an action in the current map context
		 */
		void redo();
		
		rand_rng::rng* rng_;
		
		rand_rng::set_random_generator* rng_setter_;
		
		/** The current map object */
		map_context* map_context_;
		
		/** The display object used and owned by the editor. */
		editor_display* gui_;
		
		std::vector<map_generator*> map_generators_;
		
		int current_map_generator_index_;
		
		std::vector<time_of_day> tods_;
		
		int current_tod_index_;
		
		/** Legacy object required by the legacy terrain palette and brush bar */
		size_specs* size_specs_;
		
		/** The terrain palette */
		terrain_palette* palette_;
		
		/** The brush selector */
		brush_bar* brush_bar_;
		
		/* managers */
		preferences::display_manager* prefs_disp_manager_;
		tooltips::manager tooltip_manager_;
		font::floating_label_context* floating_label_manager_;		
		
		/** Quit main loop flag */
		bool do_quit_;
		EXIT_STATUS quit_mode_;
		
		/** All available brushes */
		std::vector<brush> brushes_;
		
		/** The current brush */
		brush* brush_;
		
		typedef std::map<hotkey::HOTKEY_COMMAND, mouse_action*> mouse_action_map;
		/** The mouse actions */
		mouse_action_map mouse_actions_;
		
		typedef std::map<hotkey::HOTKEY_COMMAND, std::string> mouse_action_string_map;
		/** Usage tips for mouse actions */
		mouse_action_string_map mouse_action_hints_;
		
		/** The current mouse action */
		mouse_action* mouse_action_;
		
		/** Toolbar-requires-redraw flag */
		bool toolbar_dirty_;
		
		/** Palette's active fg tereain */
		t_translation::t_terrain foreground_terrain_;

		/** Palette's active fg tereain */
		t_translation::t_terrain background_terrain_;		

		/** Clipboard map_fragment -- used for copy-paste. */
		map_fragment clipboard_;
		
		/** Flag to rebuild terrain on every terrain change */
		bool auto_update_transitions_;
		
		/** Default directory for map load/save as dialogs */
		std::string default_dir_;
};

} //end namespace editor2

#endif
