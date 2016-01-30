/*
   Copyright (C) 2012 - 2016 by Boldizsár Lipka <lipkab@zoho.com>
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
#include "gui/widgets/widget.hpp"
#include "gui/widgets/window.hpp"
#include "widgets/scrollpane.hpp"
#include "widgets/label.hpp"
#include "widgets/button.hpp"
#include "widgets/textbox.hpp"
#include "widgets/slider.hpp"
#include "widgets/combo.hpp"

namespace mp
{

namespace options
{

class option_display
{
public:
	virtual ~option_display() {}
	virtual void layout(int& xpos, int& ypos, int w, int border_size, gui::scrollpane* pane) = 0;
	virtual void set_value(const config::attribute_value& val) = 0;
	virtual config::attribute_value get_value() const = 0;
	virtual void process_event() {}
	virtual void hide_children(bool hide) = 0;
};

class entry_display : public option_display
{
public:
	entry_display(CVideo& video, const config& cfg);
	~entry_display();

	void layout(int &xpos, int &ypos, int w, int border_size, gui::scrollpane *pane);
	void set_value(const config::attribute_value &val);
	config::attribute_value get_value() const;
	virtual void hide_children(bool hide);

private:
	gui::textbox* entry_;
	gui::label* label_;
};

class slider_display : public option_display
{
public:
	slider_display(CVideo& video, const config& cfg);
	~slider_display();

	void layout(int &xpos, int &ypos, int w, int border_size, gui::scrollpane *pane);
	void set_value(const config::attribute_value &val);
	config::attribute_value get_value() const;
	void process_event();
	virtual void hide_children(bool hide);

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
	checkbox_display(CVideo& video, const config& cfg);
	~checkbox_display();

	void layout(int &xpos, int &ypos, int w, int border_size, gui::scrollpane *pane);
	void set_value(const config::attribute_value &val);
	config::attribute_value get_value() const;
	virtual void hide_children(bool hide);
private:
	gui::button* checkbox_;
};

class combo_display : public option_display
{
public:
	combo_display(CVideo& video, const config& cfg);
	~combo_display();

	void layout(int &xpos, int &ypos, int w, int border_size, gui::scrollpane *pane);
	void set_value(const config::attribute_value &val);
	config::attribute_value get_value() const;
	void hide_children(bool hide);

private:
	gui::label* label_;
	gui::combo* combo_;

	std::vector<std::string> values_;
};

class title_display : public option_display
{
public:
	title_display(CVideo& video, const std::string& label);
	~title_display();

	void layout(int &xpos, int &ypos, int w, int border_size, gui::scrollpane *pane);
	void set_value(const config::attribute_value &/*val*/) {}
	config::attribute_value get_value() const { return config::attribute_value(); }
	virtual void hide_children(bool hide);

private:
	gui::label* title_;
};

class manager;

class reset_display : public option_display
{
public:
	reset_display(CVideo& video, const std::string& comp, manager &m);
	~reset_display();

	void layout(int &xpos, int &ypos, int w, int border_size, gui::scrollpane *pane);
	void set_value(const config::attribute_value &/*val*/) {}
	config::attribute_value get_value() const { return config::attribute_value(); }
	void hide_children(bool hide);
	void process_event();

private:
	manager &manager_;
	std::string component_;
	gui::button* button_;
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
	 * @param display			The screen to display the dialog on.
	 *
	 * @param initial_value 	The initial values for each option.
	 */
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
	 * Sets the selected scenario. Whenever show_dialog is called, only
	 * options for the selected scenario will be displayed.
	 *
	 * @param id 				The scenario's id.
	 */
	void set_scenario(const std::string& id);

	/**
	 * Sets the selected campaign. Whenever show_dialog is called, only
	 * options for the campaign scenario will be displayed.
	 *
	 * @param id 				The campaign's id.
	 */
	void set_campaign(const std::string& id);

	/**
	 * Sets the activated modifications. Whenever show_dialog is called, only
	 * options for the activated modifications will be displayed.
	 *
	 * @param ids					The ids of the modifications
	 */
	void set_modifications(const std::vector<std::string>& ids);

	void layout_widgets(int startx, int starty, int w);
	void process_event();
	void hide_children(bool hide=true);

	/**
	 * Returns the the values for each option.
	 *
	 * @return						A config containing the values.
	 */
	const config& get_values();

	void init_widgets();

	void restore_defaults(const std::string &component);

	bool has_options() const;
private:

	/** Stores needed info about each element and their configuration options */
	config options_info_;

	/** Stores the selected values for each option */
	config values_;

	/** The screen to display the dialog on */
	CVideo& video_;

	/** The scrollarea to put the widgets on */
	gui::scrollpane* pane_;

	/** The id of the selected era */
	std::string era_;

	/** The id of the selected [multiplayer] or [campaign]*/
	std::string scenario_;
	bool is_campaign_;
	/** The ids of the selected modifications */
	std::vector<std::string> modifications_;

	std::map<std::string, option_display*> widgets_;
	std::vector<option_display*> widgets_ordered_;

	/**
	 * Adds the necessary information about the specified component
	 * to options_info_.
	 *
	 * @param cfg 					The component's data.
	 * @param key					The component's type.
	 */
	void init_info(const config& cfg, const std::string& key);

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

	const config& get_component_cfg(const std::string& id) const;

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

	bool is_active(const std::string& id) const;
};

} // namespace options

} // namespace mp
#endif
