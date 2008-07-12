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

#include "brush.hpp"
#include "action_base.hpp"
#include "editor_common.hpp"
#include "editor_display.hpp"
#include "editor_map.hpp"
#include "editor_mouse_handler.hpp"
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
		void main_loop();
		bool can_execute_command(hotkey::HOTKEY_COMMAND, int index = -1) const;
		void preferences();
		
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
		
		/** The display object used and owned by the editor. Possibly recreated when a new map is created */
		editor_display* gui_;
		
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
		
		std::vector<brush> brushes_;
};

} //end namespace editor2

#endif
