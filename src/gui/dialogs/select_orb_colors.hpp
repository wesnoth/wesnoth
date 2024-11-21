/*
	Copyright (C) 2017 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/group.hpp"

#include <map>

namespace gui2::dialogs
{
class select_orb_colors : public modal_dialog
{
public:
	select_orb_colors();

	/**
	 * The display function.
	 *
	 * See @ref modal_dialog for more information.
	 */
	DEFINE_SIMPLE_DISPLAY_WRAPPER(select_orb_colors)

private:
	/**
	 * Sets up the checkbox that's common to the no-color, one-color and two-color settings.
	 * Sets its ticked/unticked state and connects the callback for user interaction.
	 */
	void setup_orb_toggle(const std::string& base_id, bool& shown);
	/**
	 * Sets up the checkbox and row of color buttons for the one-color options, including
	 * connecting the callbacks for user interaction.
	 *
	 * @param base_id which group of checkboxes and buttons to affect
	 * @param shown the checkbox's ticked state (input and asynchronous output)
	 * @param initial which color to select (input only)
	 */
	void setup_orb_group(const std::string& base_id, bool& shown, const std::string& initial);
	/**
	 * Sets up two checkboxes and a row of color buttons.
	 */
	void setup_orb_group_two_color(const std::string& base_id, bool& shown, bool& two_color, const std::string& initial);

	/**
	 * Change the UI's ticked/unticked state. Neither sets up nor triggers callbacks.
	 */
	void reset_orb_toggle(const std::string& base_id, bool shown);
	void reset_orb_group(const std::string& base_id, bool shown, const std::string& initial);
	void reset_orb_group_two_color(const std::string& base_id, bool shown, bool two_color, const std::string& initial);

	void toggle_orb_callback(bool& storage);
	void reset_orb_callback();

	bool show_unmoved_;
	bool show_partial_;
	bool show_disengaged_;
	bool show_moved_;
	bool show_ally_;
	bool two_color_ally_;
	bool show_enemy_;

	std::map<std::string, group<std::string>> groups_;

	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	virtual void post_show() override;
};

} // namespace dialogs
