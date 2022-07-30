/*
	Copyright (C) 2017 - 2022
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
	void setup_orb_toggle(const std::string& base_id, bool& shown);
	void setup_orb_group(const std::string& base_id, bool& shown, const std::string& initial);

	void reset_orb_toggle(const std::string& base_id, bool& shown);
	void reset_orb_group(const std::string& base_id, bool& shown, const std::string& initial);

	void toggle_orb_callback(bool& storage);
	void reset_orb_callback();

	bool show_unmoved_;
	bool show_partial_;
	bool show_disengaged_;
	bool show_moved_;
	bool show_ally_;
	bool show_enemy_;

	std::map<std::string, group<std::string>> groups_;

	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	virtual void post_show(window& window) override;
};

} // namespace dialogs
