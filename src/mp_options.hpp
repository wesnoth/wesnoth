/*
   Copyright (C) 2012 - 2013 by Boldizsár Lipka <lipkab@zoho.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef MP_OPTIONS_HPP_INCLUDED
#define MP_OPTIONS_HPP_INCLUDED

#include <string>
#include "config.hpp"
#include "video.hpp"
#include "gui/widgets/widget.hpp"
#include "gui/widgets/window.hpp"

namespace mp
{

namespace options
{

config to_event(const config& options);

// TODO: there's an identical enum in mp_depcheck.hpp, maybe we should factor
// 		 out?
enum elem_type
{
	SCENARIO,
	ERA,
	MODIFICATION
};

class manager
{
public:

	/**
	 * Constructor.
	 *
	 * @param gamecfg			The config object holding all eras, scenarios
	 * 							and modifications.
	 *
	 * @param video				The screen to display the dialog on.
	 *
	 * @param initial_values	The initial values for each option.
	 */
	manager(const config& gamecfg, CVideo& video, const config& initial_values);

	/**
	 * Set the current values the options. This overrides ALL previously set
	 * values, even if a not all options are provided a new value for.
	 *
	 * @param values			The new values for each option.
	 */
	void set_values(const config& values);

	/**
	 * Sets the selected era. Whenever show_dialog is called, only
	 * options for the selected era will be displayed.
	 *
	 * @param id				The era's id.
	 */
	void set_era(const std::string& id);

	/**
	 * Sets the selected era. Whenever show_dialog is called, only
	 * options for the selected era will be displayed.
	 *
	 * @param index				The era's index.
	 */
	void set_era_by_index(int index);

	/**
	 * Sets the selected scenario. Whenever show_dialog is called, only
	 * options for the selected scenario will be displayed.
	 *
	 * @param id 				The scenario's id.
	 */
	void set_scenario(const std::string& id);

	/**
	 * Sets the selected scenario. Whenever show_dialog is called, only
	 * options for the selected scenario will be displayed.
	 *
	 * @param index 				The scenario's index.
	 */
	void set_scenario_by_index(int index);

	/**
	 * Sets the activated modifications. Whenever show_dialog is called, only
	 * options for the activated modifications will be displayed.
	 *
	 * @param ids					The ids of the modifications
	 */
	void set_modifications(const std::vector<std::string>& ids);

	/**
	 * Add options information of an era/scenario/modification not yet in the
	 * database.
	 *
	 * @param type					The type of the element,
	 *
	 * @param data					The config object which holds the
	 * 								information about the element's options in
	 * 								an [options] child.
	 *
	 * @param pos					The position to insert  the element into.
	 */
	void insert_element(elem_type type, const config& data, int pos);

	/**
	 * Shows the options dialog and saves the selected values.
	 */
	void show_dialog();

	/**
	 * Returns the the values for each option.
	 *
	 * @return						A config containing the values.
	 */
	const config& get_values() const { return values_; }

private:
	/** Stores needed info about each element and their configuration options */
	config options_info_;

	/** Stores the selected values for each option */
	config values_;

	/** The screen to display the dialog on */
	CVideo &video_;

	/** The id of the selected era */
	std::string era_;

	/** The id of the selected scenario */
	std::string scenario_;

	/** The ids of the selected modifications */
	std::vector<std::string> modifications_;

	/**
	 * Adds the necessary information about the specified component
	 * to options_info_.
	 *
	 * @param cfg 					The component's data.
	 * @param key					The component's type.
	 */
	void init_info(const config& cfg, const std::string& key);

	/**
	 * Creates a widget layout based on an [options] section.
	 *
	 * @param data					The [options] section.
	 * @param grid					The grid to create the layout in.
	 */
	void add_widgets(const config& data, config& grid) const;

	/**
	 * Creates a slider widget.
	 *
	 * @param data					A [slider] config.
	 * @param column				The grid cell to add the widget into.
	 */
	void add_slider(const config& data, config& column) const;

	/**
	 * Creates a checkbox (toggle button) widget.
	 *
	 * @param data					A [checkbox] config.
	 * @param column				The grid cell to add the widget into.
	 */
	void add_checkbox(const config& data, config& column) const;

	/**
	 * @todo Implement this function (along with a combo box widget, preferably)
	 *
	 * Creates a combo box widget.
	 *
	 * @param data					A [combobox] config.
	 * @param column				The grid cell to add the widget into.
	 */
	void add_combobox(const config& data, config& column) const;

	/**
	 * Creates a text entry widget.
	 *
	 * @param data					An [entry] config.
	 * @param column				The grid cell to add the widget into.
	 */
	void add_entry(const config& data, config& column) const;

	/**
	 * Returns the node which holds the selected value of an option. If that
	 * node is not yet created, the function creates it.
	 *
	 * @param id					The id of the option.
	 *
	 * @return						A reference to the config which the value
	 * 								for this option should be written into.
	 */
	config& get_value_cfg(const std::string& id);

	/**
	 * Returns the node which holds the selected value of an option. If that
	 * node is not yet created, the function returns an empty config.
	 *
	 * @param id					The id of the option.
	 *
	 * @return						A reference to the config which the value
	 * 								for this option should be written into or
	 * 								an empty config if that doesn't exist.
	 */
	const config& get_value_cfg_or_empty(const std::string& id) const;

	/**
	 * Returns the information about an option.
	 *
	 * @param id					The id of the option.
	 *
	 * @return						The config object which contains the
	 * 								settings of the option, or an empty config
	 * 								if the option was not found.
	 */
	const config& get_option_info_cfg(const std::string& id) const;

	/**
	 * Finds the parent node of an options.
	 *
	 * @param id					The id of the option.
	 *
	 * @return						A config::any_child object containing the
	 * 								key and the data of the parent node, or ""
	 * 								for the key and an empty config if the
	 * 								option was not found.
	 */
	config::any_child get_option_parent(const std::string& id) const;

	/**
	 * Retrieves the saved value for a certain option, or the default, if
	 * there's no such.
	 *
	 * @param id					The id of the option.
	 *
	 * @return 						The value saved in values_ for this option
	 * 								or its specified default value if a saved
	 * 								value can't be found.
	 */
	config::attribute_value get_stored_value(const std::string& id) const;

	/**
	 * Retrieves the default value for a certain option.
	 *
	 * @param id					The id of the option.
	 *
	 * @return						The default value for this option.
	 */
	config::attribute_value get_default_value(const std::string& id) const;

	/**
	 * Gets the current value of a slider widget.
	 *
	 * @param id					The id of the widget.
	 * @param win					The window to find the widget in.
	 *
	 * @return						The integer currently set on the slider.
	 */
	int get_slider_value(const std::string& id, gui2::twindow* win) const;

	/**
	 * Gets the current value of a checkbox widget.
	 *
	 * @param id					The id of the widget.
	 * @param win					The window to find the widget in.
	 *
	 * @return						True if the box is checked, false if not.
	 */
	bool get_checkbox_value(const std::string& id, gui2::twindow* win) const;

	/**
	 * Gets the current value of a text_box widget.
	 *
	 * @param id					The id of the widget.
	 * @param win					The window to find the widget in.
	 *
	 * @return						The text written in the widget.
	 */
	std::string get_entry_value(const std::string& id,
								gui2::twindow* win) const;

	/**
	 * Sets the value of a checkbox widget.
	 *
	 * @param val					The new value.
	 * @param id					The id of the widget.
	 * @param win					The window which the widget is a child of.
	 */
	void set_slider_value(int val, const std::string& id,
						  gui2::twindow* win) const;

	/**
	 * Sets the value of a slider widget.
	 *
	 * @param val					The new value.
	 * @param id					The id of the widget.
	 * @param win					The window which the widget is a child of.
	 */
	void set_checkbox_value(bool val, const std::string& id,
							gui2::twindow* win) const;

	/**
	 * Sets the value of a text_box widget.
	 *
	 * @param val					The new value.
	 * @param id					The id of the widget.
	 * @param win					The window which the widget is a child of.
	 */
	void set_entry_value(const std::string& val, const std::string& id,
						 gui2::twindow* win) const;

	/**
	 * Writes all the values for the options of a certain component from a
	 * specified window into values_.
	 *
	 * @param key					The component's type.
	 * @param id					The component's id.
	 * @param window				The window.
	 */
	void extract_values(const std::string& key, const std::string& id,
					  gui2::twindow* window);

	/**
	 * Decides whether a config is a sane option node or not.
	 * A valid option node:
	 * 		- Must have an id field.
	 * 		- Its key must be "slider", "entry" or "checkbox"
	 *
	 * @param key					The option's key.
	 * @param option				The option's data.
	 *
	 * @return						True if the option is valid, false if not.
	 */
	static bool is_valid_option(const std::string& key, const config& option);

	/**
	 * Restores every widget's value to its default for a window.
	 *
	 * @param m						A pointer to the manager which generated
	 * 								the window.
	 * @param w						A pointer to the window itself.
	 */
	static void restore_defaults(manager* m, gui2::twindow* w);

	/**
	 * Finds the widgets representing the options of a certain component in a
	 * window (era, scenario or modification) and sets their value to their
	 * defaults.
	 *
	 * @param comp					The config of the component.
	 * @param m						A pointer to the manager which generated
	 * 								the window.
	 * @param w						A pointer to the window.
	 */
	static void restore_defaults_for_component(const config& comp, manager* m,
											   gui2::twindow* w);

	/**
	 * @todo 		Implement a way to initialize the checkbox via WML and then
	 * 				remove this function altogether.
	 *
	 * Sets the default states for all checkbox widgets inside a window. All
	 * required data is fetched from values_ and options_info_.
	 *
	 * @param window				The window.
	 */
	void __tmp_set_checkbox_defaults(gui2::twindow* window) const;
};

} // namespace options

} // namespace mp
#endif
