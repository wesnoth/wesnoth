/*
   Copyright (C) 2010 - 2018 by Fabian Müller <fabianmueller5@gmx.de>
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

#include "editor/map/map_context.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/group.hpp"
#include "team.hpp"

namespace gui2
{

namespace dialogs
{

class editor_edit_side : public modal_dialog
{
public:
	explicit editor_edit_side(editor::editor_team_info& info);

	/** The execute function. See @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(editor_edit_side)

private:
	virtual void pre_show(window& window) override;
	virtual void post_show(window& window) override;

	team::CONTROLLER& controller_;
	group<team::CONTROLLER> controller_group;

	team::SHARE_VISION& share_vision_;
	group<team::SHARE_VISION> vision_group;

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;
};
} // namespace dialogs
} // namespace gui2
