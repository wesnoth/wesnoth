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

#include "gui/auxiliary/tips.hpp"
#include "gui/core/window_builder.hpp"
#include "gui/core/widget_definition.hpp"

#include <map>

class config;

namespace gui2
{
/**
 * A GUI theme definiton.
 *
 * Each theme defines the appearance and layout of widgets and windows. At least one theme
 * (the default) must exist for the game to run. That theme is expected to contain at least
 * one default definition for each widget type and a layout for each window recorded in the
 * static registry. Do note that a widget type may have any number of definitions defined
 * per theme, but a window may only have one layout.
 *
 * Non-default themes may omit a default widget defintion or a window layout, in which case
 * the game will fall back on the default definition (for widgets) or the layout (for windows)
 * specified in the default theme.
 *
 * Each widget definition and window layout may also define different variations of itself
 * to be used based on the current game resolution.
 *
 * As of December 2017 only the default theme is provided.
 */
class gui_definition
{
public:
	/** Private ctor. Use @ref create to initialize a new definition. */
	explicit gui_definition(const config& cfg);

	using widget_definition_map_t = std::map<std::string, styled_widget_definition_ptr>;

	/** Map of each widget type, by id, and a sub-map of each of the type's definitions, also by id. */
	std::map<std::string, widget_definition_map_t> widget_types;

	/** Map of all known windows (the builder class builds a window). */
	std::map<std::string, builder_window> window_types;

	/** Activates this GUI. */
	void activate() const;

private:
	std::string id_;

	t_string description_;

	unsigned popup_show_delay_;
	unsigned popup_show_time_;
	unsigned help_show_time_;
	unsigned double_click_time_;
	unsigned repeat_button_repeat_time_;

	std::string sound_button_click_;
	std::string sound_toggle_button_click_;
	std::string sound_toggle_panel_click_;
	std::string sound_slider_adjust_;

	t_string has_helptip_message_;

	std::vector<game_tip> tips_;
};

using gui_theme_map_t = std::map<std::string, gui_definition>;

/** Map of all known GUIs. */
extern gui_theme_map_t guis;

/** Iterator pointing to the current GUI. */
extern gui_theme_map_t::iterator current_gui;

/** Iterator pointing to the default GUI. */
extern gui_theme_map_t::iterator default_gui;

/**
 * Returns the appropriate config data for a widget instance fom the active
 * GUI definition.
 *
 * @param control_type        The widget type.
 * @param definition          The definition ID.
 *
 * @returns                   A pointer to the specified definition data struct
 *                            for the widget type, accounting for the current
 *                            screen resolution.
 */
resolution_definition_ptr get_control(const std::string& control_type, const std::string& definition);

/** Helper struct to signal that get_window_builder failed. */
struct window_builder_invalid_id
{
};

/**
 * Returns an reference to the requested builder.
 *
 * The builder is determined by the @p type and the current screen resolution.
 *
 * @pre                       There is a valid builder for @p type at the
 *                            current resolution.
 *
 * @throws window_builder_invalid_id
 *                            When the precondition is violated.
 *
 * @param type                The type of builder window to get.
 *
 * @returns                   An iterator to the requested builder.
 */
const builder_window::window_resolution& get_window_builder(const std::string& type);

/** Adds a widget definiton to the default GUI. */
bool add_single_widget_definition(const std::string& widget_type,
		const std::string& definition_id,
		const config& cfg);

/** Removes a widget definiton from the default GUI. */
void remove_single_widget_definition(const std::string& widget_type,
		const std::string& definition_id);

} // namespace gui2
