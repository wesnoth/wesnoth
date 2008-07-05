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

#include "editor_common.hpp"
#include "editor_map.hpp"
#include "editor_display.hpp"

#include <boost/utility.hpp>

#include "../key.hpp"
#include "../config.hpp"
#include "../events.hpp"
#include "../hotkeys.hpp"
#include "../sdl_utils.hpp"

namespace editor2 {

class editor_controller : public events::handler, public hotkey::command_executor,
		private boost::noncopyable
{
	public:
		editor_controller(const config &game_config, CVideo& video);
		~editor_controller();
		void main_loop();
		bool can_execute_command(hotkey::HOTKEY_COMMAND, int) const;
		void handle_event(const SDL_Event&){}
	private:    
		/** init the display object and general set-up */ 
		void init(CVideo& video);
		/** cref to the main game config */
		const config& game_config_;
		/** The current map object */
		editor_map map_;
		/** The display object used and owned by the editor. Possibly recreated when a new map is created */
		editor_display* gui_;
		
		CKey key_;
        bool map_dirty_;
};

} //end namespace editor2

#endif
