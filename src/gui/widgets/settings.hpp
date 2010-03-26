/* $Id$ */
/*
   Copyright (C) 2007 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file src/gui/widgets/settings.hpp
 * This file contains the settings handling of the widget library.
 */

#ifndef GUI_WIDGETS_SETTING_HPP_INCLUDED
#define GUI_WIDGETS_SETTING_HPP_INCLUDED

#include "gui/auxiliary/widget_definition/window.hpp"

#include <string>
#include <vector>

namespace gui2 {

/** Do we wish to use the new library or not. */
extern bool new_widgets;

/**
 * Registers a window.
 *
 * This function registers the available windows defined in WML. All windows
 * need to register themselves before @ref gui2::init) is called.
 *
 * @warning This function runs before @ref main() so needs to be careful
 * regarding the static initialization problem.
 *
 * @note Double registering a window can't hurt, but no way to probe for it,
 * this can be added if needed. The same for an unregister function.
 *
 * @param id                      The id of the window to register.
 */
void register_window(const std::string& id);


	tresolution_definition_ptr get_control(
		const std::string& control_type, const std::string& definition);

	std::vector<twindow_builder::tresolution>::const_iterator
		get_window_builder(const std::string& type);

	/** Loads the setting for the theme. */
	void load_settings();

	/** This namespace contains the 'global' settings. */
	namespace settings {

		/**
		 * The screen resolution should be available for all widgets since
		 * their drawing method will depend on it.
		 */
		extern unsigned screen_width;
		extern unsigned screen_height;

		/**
		 * The size of the map area, if not available equal to the screen
		 * size.
		 */
		extern unsigned gamemap_width;
		extern unsigned gamemap_height;

		/** These are copied from the active gui. */
		extern unsigned popup_show_delay;
		extern unsigned popup_show_time;
		extern unsigned help_show_time;
		extern unsigned double_click_time;
		extern unsigned repeat_button_repeat_time;

		extern std::string sound_button_click;
		extern std::string sound_toggle_button_click;
		extern std::string sound_toggle_panel_click;
		extern std::string sound_slider_adjust;
	}

} // namespace gui2

#endif

