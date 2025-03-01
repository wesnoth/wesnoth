/*
	Copyright (C) 2023 - 2025
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
class reachmap_options : public modal_dialog
{
public:
	reachmap_options();

	DEFINE_SIMPLE_DISPLAY_WRAPPER(reachmap_options)

private:
	/**
	 * Sets up the checkbox and row of color buttons for the one-color options, including
	 * connecting the callbacks for user interaction.
	 *
	 * @param base_id which group of checkboxes and buttons to affect
	 * @param initial which color to select (input only)
	 */
	void setup_reachmap_group(const std::string& base_id, const std::string& initial);


	/**
	 * Change the UI's ticked/unticked state. Neither sets up nor triggers callbacks.
	 */

	void reset_reachmap_group(const std::string& base_id, const std::string& initial);

	void reset_reachmap_slider(const std::string& base_id, const int& initial);

	void reset_reachmap_callback();

	std::map<std::string, group<std::string>> groups_;

	virtual void pre_show() override;
	virtual void post_show() override;

	virtual const std::string& window_id() const override;
};

} // namespace gui2::dialogs
