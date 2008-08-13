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

namespace editor2 {

class brush_bar;
class size_specs;
class terrain_palette;

class editor_controller : public controller_base, 
	public events::mouse_handler_base,
	private boost::noncopyable
{
	public:
		editor_controller(const config &game_config, CVideo& video);
		~editor_controller();
		EXIT_STATUS main_loop();
		void hotkey_quit();
		void quit_confirm(EXIT_STATUS status);
		bool confirm_discard();
		
		map_context& get_map_context() { return map_context_; }
		const map_context& get_map_context() const { return map_context_; }

		editor_map& get_map() { return get_map_context().get_map(); }
		const editor_map& get_map() const { return get_map_context().get_map(); }
		
		void load_map_dialog();
		void new_map_dialog();
		void save_map_as_dialog();
		void generate_map_dialog();
		void resize_map_dialog();
		bool save_map(bool display_confirmation = false);
		bool save_map_as(const std::string& filename);
		void new_map(int width, int height, t_translation::t_terrain fill);
		void load_map(const std::string& filename);
		void revert_map();
		void set_map(const editor_map& map);
		void reload_map();
				
		bool can_execute_command(hotkey::HOTKEY_COMMAND, int index = -1) const;
		hotkey::ACTION_STATE get_action_state(hotkey::HOTKEY_COMMAND command) const;
		bool execute_command(hotkey::HOTKEY_COMMAND command, int index = -1);
		void expand_starting_position_menu(std::vector<std::string>& items);
		void show_menu(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu);
		
		void cycle_brush();
		void preferences();
		void toggle_grid();
		void copy_selection();
		void cut_selection();
		void fill_selection();
		
		void hotkey_set_mouse_action(hotkey::HOTKEY_COMMAND command);
		bool is_mouse_action_set(hotkey::HOTKEY_COMMAND command) const;
	
		
		/* mouse_handler_base */
		void mouse_motion(int x, int y, const bool browse, bool update);
		editor_display& gui() { return *gui_; }
		const editor_display& gui() const { return *gui_; }
		bool allow_mouse_wheel_scroll(int x, int y);
		bool left_click(int x, int y, const bool browse);
		void left_drag_end(int x, int y, const bool browse);
		void left_mouse_up(int x, int y, const bool browse);
		
	protected:
		void process_keyup_event(const SDL_Event& event);
		mouse_handler_base& get_mouse_handler_base();
		editor_display& get_display();	
		brush* get_brush();
		mouse_action* get_mouse_action();
		void perform_refresh_delete(editor_action* action);
		void perform_refresh(const editor_action& action);
		
	private:    		
		/** init the display object and general set-up */ 
		void init(CVideo& video);
		
		void load_tooltips();
		
		void redraw_toolbar();
		
		void refresh_image_cache();
		
		void refresh_after_action(bool drag_part = false);
		
		void refresh_all();
		
		void display_redraw_callback(display&);
		
		/**
		 * Un-does an action, and puts it in the redo stack for a possible redo
		 */
		void undo();

		/**
		 * Re-does a previously undid action, and puts it back in the undo stack.
		 */
		void redo();
		
		/** The current map object */
		map_context map_context_;
		
		/** The display object used and owned by the editor. */
		editor_display* gui_;
		
		map_generator* map_generator_;
		
		size_specs* size_specs_;
		terrain_palette* palette_;
		brush_bar* brush_bar_;
		
		preferences::display_manager* prefs_disp_manager_;
		tooltips::manager tooltip_manager_;
		font::floating_label_context* floating_label_manager_;		
		
		bool do_quit_;
		EXIT_STATUS quit_mode_;
		
		std::vector<brush> brushes_;
		brush* brush_;
		typedef std::map<hotkey::HOTKEY_COMMAND, mouse_action*> mouse_action_map;
		mouse_action_map mouse_actions_;
		mouse_action* mouse_action_;
		
		bool toolbar_dirty_;
		
		t_translation::t_terrain foreground_terrain_;
		t_translation::t_terrain background_terrain_;		
		map_fragment clipboard_;
		
		bool auto_update_transitions_;
};

} //end namespace editor2

#endif
