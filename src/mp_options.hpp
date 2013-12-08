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
#include "widgets/scrollpane.hpp"
#include "widgets/label.hpp"
#include "widgets/button.hpp"
#include "widgets/textbox.hpp"
#include "widgets/slider.hpp"

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

class option_display
{
public:
	virtual ~option_display() {}
	virtual void layout(int& xpos, int& ypos, int border_size, gui::scrollpane* pane) = 0;
	virtual void set_value(const config::attribute_value& val) = 0;
	virtual config::attribute_value get_value() const = 0;
	virtual void process_event() = 0;
};

class entry_display : public option_display
{
public:
	entry_display(CVideo& video, const std::string& label, const std::string &value);
	~entry_display();

	void layout(int &xpos, int &ypos, int border_size, gui::scrollpane *pane);
	void set_value(const config::attribute_value &val);
	config::attribute_value get_value() const;
	void process_event() {}

private:
	gui::textbox* entry_;
	gui::label* label_;
};

class slider_display : public option_display
{
public:
	slider_display(CVideo& video, const std::string& label, int value, int min, int max, int step);
	~slider_display();

	void layout(int &xpos, int &ypos, int border_size, gui::scrollpane *pane);
	void set_value(const config::attribute_value &val);
	config::attribute_value get_value() const;
	void process_event();

private:
	void update_label();

	gui::slider* slider_;
	gui::label* label_;
	int last_value_;
	const std::string label_text_;
};

class checkbox_display : public option_display
{
public:
	checkbox_display(CVideo& video, const std::string& label, bool value);
	~checkbox_display();

	void layout(int &xpos, int &ypos, int border_size, gui::scrollpane *pane);
	void set_value(const config::attribute_value &val);
	config::attribute_value get_value() const;
	void process_event() {}
private:
	gui::button* checkbox_;
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
	//manager(const config& gamecfg, CVideo& video, const config& initial_values);

	manager(const config& gamecfg, CVideo& video, gui::scrollpane* pane, const config& initial_value);

	~manager();

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

	void layout_widgets(int startx, int starty);
	void process_event();

	/**
	 * Returns the the values for each option.
	 *
	 * @return						A config containing the values.
	 */
	const config& get_values();


private:

	/** Stores needed info about each element and their configuration options */
	config options_info_;

	/** Stores the selected values for each option */
	config values_;

	/** The screen to display the dialog on */
	CVideo &video_;

	/** The scrollarea to put the widgets on */
	gui::scrollpane* pane_;

	/** The id of the selected era */
	std::string era_;

	/** The id of the selected scenario */
	std::string scenario_;

	/** The ids of the selected modifications */
	std::vector<std::string> modifications_;

	std::map<std::string, option_display*> widgets_;

	/**
	 * Adds the necessary information about the specified component
	 * to options_info_.
	 *
	 * @param cfg 					The component's data.
	 * @param key					The component's type.
	 */
	void init_info(const config& cfg, const std::string& key);

	void init_widgets();

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
	 * Writes all the values for the options of a certain component from a
	 * specified window into values_.
	 *
	 * @param key					The component's type.
	 * @param id					The component's id.
	 * @param window				The window.
	 */
	void extract_values(const std::string& key, const std::string& id);

	void update_values();

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
	static void restore_defaults(manager* m);

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
	static void restore_defaults_for_component(const config& comp, manager* m);
};

} // namespace options

} // namespace mp
#endif
