/*
	Copyright (C) 2009 - 2024
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

#include "game_initialization/flg_manager.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/group.hpp"

#include <string>

namespace gui2::dialogs
{

class faction_select : public modal_dialog
{
public:
	faction_select(ng::flg_manager& flg_manager, const std::string& color, const int side);

	DEFINE_SIMPLE_EXECUTE_WRAPPER(faction_select)

	int get_side_num() const { return side_; }
private:
	ng::flg_manager& flg_manager_;

	const std::string tc_color_;

	const int side_;

	group<std::string> gender_toggle_;

	const int last_faction_, last_leader_, last_gender_;

	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	virtual void post_show() override;

	/** Callbacks */
	void on_faction_select();

	void on_leader_select();

	void profile_button_callback();

	void on_gender_select(const std::string& val);

	void update_leader_image();
};

} // namespace dialogs
