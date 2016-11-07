/*
   Copyright (C) 2007 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * This file contains the settings handling of the widget library.
 */

#ifndef GUI_WIDGETS_SETTING_HPP_INCLUDED
#define GUI_WIDGETS_SETTING_HPP_INCLUDED

#include "utils/functional.hpp"
#include "config.hpp"
#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"
#include "tstring.hpp"

#include <string>
#include <vector>

namespace gui2
{

struct gui_definition;
class game_tip;

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

/**
 * Special helper class to get the list of registered windows.
 *
 * This is used in the unit tests, but these implementation details shouldn't
 * be used in the normal code.
 */
class tunit_test_access_only
{
	friend std::vector<std::string>& unit_test_registered_window_list();

	/** Returns a copy of the list of registered windows. */
	static std::vector<std::string> get_registered_window_list();
};

/**
 * Registers a widgets.
 *
 * This function registers the available widgets defined in WML. All widgets
 * need to register themselves before @ref gui2::init) is called.
 *
 * @warning This function runs before @ref main() so needs to be careful
 * regarding the static initialization problem.
 *
 * @param id                      The id of the widget to register.
 * @param functor                 The function to load the definitions.
 */
void register_widget(const std::string& id,
					 std::function<void(gui_definition& gui,
										  const std::string& definition_type,
										  const config& cfg,
										  const char* key)> functor);

/**
 * Loads the definitions of a widget.
 *
 * @param gui_definition          The gui definition the widget definition
 *                                belongs to.
 * @param definition_type         The type of the widget whose definitions are
 *                                to be loaded.
 * @param definitions             The definitions serialized from a config
 *                                object.
 */
void load_widget_definitions(
		gui_definition& gui,
		const std::string& definition_type,
		const std::vector<control_definition_ptr>& definitions);

/**
 * Loads the definitions of a widget.
 *
 * This function is templated and kept small so only loads the definitions from
 * the config and then lets the real job be done by the @ref
 * load_widget_definitions above.
 *
 * @param gui_definition          The gui definition the widget definition
 *                                belongs to.
 * @param definition_type         The type of the widget whose definitions are
 *                                to be loaded.
 * @param cfg                     The config to serialise the definitions
 *                                from.
 * @param key                     Optional id of the definition to load.
 */
template <class T>
void load_widget_definitions(gui_definition& gui,
							 const std::string& definition_type,
							 const config& cfg,
							 const char* key)
{
	std::vector<control_definition_ptr> definitions;

	for (const auto & definition :
			cfg.child_range(key ? key : definition_type + "_definition"))
	{

		definitions.push_back(std::make_shared<T>(definition));
	}

	load_widget_definitions(gui, definition_type, definitions);
}


resolution_definition_ptr get_control(const std::string& control_type,
									   const std::string& definition);

/** Helper struct to signal that get_window_builder failed. */
struct twindow_builder_invalid_id
{
};

/**
 * Returns an interator to the requested builder.
 *
 * The builder is determined by the @p type and the current screen
 * resolution.
 *
 * @pre                       There is a valid builder for @p type at the
 *                            current resolution.
 *
 * @throw twindow_builder_invalid_id
 *                            When the precondition is violated.
 *
 * @param type                The type of builder window to get.
 *
 * @returns                   An iterator to the requested builder.
 */
std::vector<builder_window::window_resolution>::const_iterator
get_window_builder(const std::string& type);

/** Loads the setting for the theme. */
void load_settings();

/** This namespace contains the 'global' settings. */
namespace settings
{

/**
 * The screen resolution should be available for all widgets since
 * their drawing method will depend on it.
 */
extern unsigned screen_width;
extern unsigned screen_height;

/**
 * The offset between the left edge of the screen and the gamemap.
 */
extern unsigned gamemap_x_offset;

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

extern t_string has_helptip_message;

std::vector<game_tip> get_tips();
}

} // namespace gui2

#endif
