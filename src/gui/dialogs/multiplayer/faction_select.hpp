/*
   Copyright (C) 2009 - 2018 by the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include <vector>

namespace gui2
{
namespace dialogs
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

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Inherited from modal_dialog. */
	virtual void post_show(window& window) override;

	/** Callbacks */
	void on_faction_select(window& window);

	void on_leader_select(window& window);

	void profile_button_callback(window& window);

	void on_gender_select(window& window);

	void update_leader_image(window& window);
};

} // namespace dialogs
} // namespace gui2
