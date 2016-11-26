/*
   Copyright (C) 2010 - 2016 by Fabian Müller <fabianmueller5@gmx.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_EDITOR_EDIT_SIDE_HPP_INCLUDED
#define GUI_DIALOGS_EDITOR_EDIT_SIDE_HPP_INCLUDED

#include "editor/map/map_context.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/group.hpp"
#include "team.hpp"

namespace gui2
{

class toggle_button;

namespace dialogs
{

class editor_edit_side : public modal_dialog
{
public:
	explicit editor_edit_side(editor::editor_team_info& info);

	/** The execute function see @ref modal_dialog for more information. */
	static bool execute(editor::editor_team_info& info, CVideo& video)
	{
		return editor_edit_side(info).show(video);
	}

private:
	void pre_show(window& window);
	void post_show(window& window);

	team::CONTROLLER& controller_;
	group<team::CONTROLLER> controller_group;

	team::SHARE_VISION& share_vision_;
	group<team::SHARE_VISION> vision_group;

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;
};
} // namespace dialogs
} // namespace gui2

#endif /* ! GUI_DIALOGS_EDIT_LABEL_INCLUDED */
