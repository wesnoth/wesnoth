/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"

namespace gui2
{
/***** ***** ***** ***** Registrars ***** ***** ***** *****/

/**
 * Registers a window.
 *
 * This function is utilized by the @ref REGISTER_WINDOW macro and notes a
 * window to look for when a GUI definition is initialized.
 *
 * All windows need to register themselves before @ref gui2::init is called.
 *
 * @warning This function runs before @ref main() so needs to be careful
 * regarding the static initialization problem.
 *
 * @note A window can't be registered twice. Any subsequently added windows
 * with duplicate IDs will be ignored. Might be worth looking into adding an
 * unregister function in the future if this becomes an issue.
 *
 * @param id                  The id of the window to register.
 */
void register_window(const std::string& id);

/** Function type alias for @ref register_widget. */
using widget_parser_t = std::function<styled_widget_definition_ptr(const config&)>;

/**
 * Registers a widget type.
 *
 * This function is utilized by the @ref REGISTER_WIDGET macro and sets the
 * the parser function used to process the widget type's WML when a GUI
 * definition is initialized.
 *
 * All widgets need to register themselves before @ref gui2::init is called.
 *
 * @warning This function runs before @ref main() so needs to be careful
 * regarding the static initialization problem.
 *
 * @param type                The type of the widget to register.
 * @param f                   The function to parse the definition config.
 * @param key                 The tagname from which to read the widget's
 *                            definition in the game config. If nullptr the
 *                            default [<id>_definition] is used.
 */
void register_widget(const std::string& type, widget_parser_t f, const char* key = nullptr);

/** Function type alias for @ref register_widget_builder. */
using widget_builder_func_t = std::function<builder_widget_ptr(const config&)>;

/**
 * Registers a widget builder.
 *
 * A widget builder simply creates and returns a pointer to a widget type's
 * builder struct. This is part of the static registry since widget builders
 * are simply used to instantiate a widget object when a dialog is being
 * assembled.
 *
 * If the widget inherits from @ref styled_widget, any theme-dependent info
 * will be fetched from the current GUI theme upon construction.
 *
 * @warning This function runs before @ref main() so needs to be careful
 * regarding the static initialization problem.
 *
 * @param type                The type of the widget as used in WML.
 * @param functor             The functor to create the widget.
 */
void register_widget_builder(const std::string& type, widget_builder_func_t functor);


/***** ***** ***** ***** Accessors ***** ***** ***** *****/

/*
 * Notes on the registered widget and window lists.
 *
 * These lists are independent of the active GUI definition and are filled
 * during static initialization via @ref register_widget and @register_window.

 * Also note these cannot be free-standing static data members within this file
 * since that can cause a crash.
 */

/** Returns the list of registered windows. */
std::set<std::string>& registered_window_types();

struct registered_widget_parser
{
    /** The widget definition WML parser function. */
	widget_parser_t parser;

    /** The tag containing the definition WML. */
	const char* key;
};

using registered_widget_map = std::map<std::string, registered_widget_parser>;

/** Returns the list of registered widgets and their parsers. */
registered_widget_map& registered_widget_types();

using widget_builder_map = std::map<std::string, widget_builder_func_t>;

/** Returns the list of registered widget builders. */
widget_builder_map& widget_builder_lookup();

} // namespace gui2
