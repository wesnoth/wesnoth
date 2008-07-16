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
#include "editor_map.hpp"
#include "editor_main.hpp"
#include "editor_mode.hpp"

#include "../config.hpp"
#include "../controller_base.hpp"
#include "../events.hpp"
#include "../hotkeys.hpp"
#include "../key.hpp"
#include "../mouse_handler_base.hpp"
#include "../sdl_utils.hpp"

#include <deque>
#include <boost/utility.hpp>

namespace editor2 {

class editor_controller : public controller_base, 
	public editor_mode, public events::mouse_handler_base,
	private boost::noncopyable
{
	public:
		editor_controller(const config &game_config, CVideo& video);
		~editor_controller();
		EXIT_STATUS main_loop();
		void hotkey_quit();
		void quit_confirm(EXIT_STATUS status);
		void load_map_dialog();
		void load_map(const std::string& filename);
		void set_map(const editor_map& map);
		bool can_execute_command(hotkey::HOTKEY_COMMAND, int index = -1) const;
		hotkey::ACTION_STATE get_action_state(hotkey::HOTKEY_COMMAND command) const;
		bool execute_command(hotkey::HOTKEY_COMMAND command, int index = -1);
		void show_menu(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu);
		void cycle_brush();
		void preferences();
		void toggle_grid();
		
		void hotkey_set_mouse_action(hotkey::HOTKEY_COMMAND command);
		bool is_mouse_action_set(hotkey::HOTKEY_COMMAND command) const;

		
		/* mouse_handler_base */
		void mouse_motion(int x, int y, const bool browse, bool update);
		editor_display& gui() { return *gui_; }
		const editor_display& gui() const { return *gui_; }
		bool left_click(int x, int y, const bool browse);
		void left_drag_end(int x, int y, const bool browse);
		
	protected:
		mouse_handler_base& get_mouse_handler_base();
		editor_display& get_display();	
	private:    
		/**
		 * Container type used to store actions in the undo and redo stacks
		 */
		typedef std::deque<editor_action*> action_stack;
		
		/** init the display object and general set-up */ 
		void init(CVideo& video);
		
		/**
		 * Performs an action (thus modyfying the map). An appropriate undo action is added to
		 * the undo stack. The redo stack is cleared.
		 */
		void perform_action(const editor_action& action);
		
		void perform_partial_action(const editor_action& action);

		void refresh_after_action(const editor_action& action);
		
		void refresh_all();

		/**
		 * Checks if an action stack reached its capacity and removes the front element if so.
		 */
		void trim_stack(action_stack& stack);

		/**
		 * Clears an action stack and deletes all its contents. Helper function used when the undo
		 * or redo stack needs to be cleared
		 */
		void clear_stack(action_stack& stack);

		/**
		 * @return true when undo can be performed, false otherwise
		 */
		bool can_undo() const;

		/**
		 * @return true when redo can be performed, false otherwise
		 */
		bool can_redo() const;

		/**
		 * Un-does an action, and puts it in the redo stack for a possible redo
		 */
		void undo();

		/**
		 * Re-does a previousle undid action, and puts it back in the undo stack.
		 */
		void redo();

		/**
		 * Perform an action at the back of one stack, and then move it to the back of the other stack.
		 * This is the implementation of both undo and redo which only differ in the direction.
		 */
		void perform_action_between_stacks(action_stack& from, action_stack& to);
		
		/** The current map object */
		editor_map map_;
		
		std::string filename_;
		
		/** The display object used and owned by the editor. Possibly recreated when a new map is created */
		editor_display* gui_;
		
		preferences::display_manager* prefs_disp_manager_;
		
		/**
		 * The undo stack. A double-ended queues due to the need to add items to one end,
		 * and remove from both when performing the undo or when trimming the size. This container owns
		 * all contents, i.e. no action in the stack shall be deleted, and unless otherwise noted the contents 
		 * could be deleted at an time during normal operation of the stack. To work on an action, either
		 * remove it from the container or make a copy. Actions are inserted at the back of the container
		 * and disappear from the front when the capacity is exceeded.
		 * @todo Use boost's pointer-owning container?
		 */
		action_stack undo_stack_;
		
		/**
		 * The redo stack. @see undo_stack_
		 */
		action_stack redo_stack_;
		
		/**
		 * Action stack (i.e. undo and redo) maximum size
		 */
		static const int max_action_stack_size_;
		
		int actions_since_save_;
		
		bool do_quit_;
		EXIT_STATUS quit_mode_;
		
		std::vector<brush> brushes_;
		int current_brush_index_;
		std::map<hotkey::HOTKEY_COMMAND, mouse_action*> mouse_actions_;
		
};

} //end namespace editor2

#endif
