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
#include "gui/dialogs/dialog.hpp"
#include "gui/widgets/group.hpp"
#include "team.hpp"

namespace gui2
{

class ttoggle_button;

class teditor_edit_side : public tdialog
{
public:
	explicit teditor_edit_side(editor::editor_team_info& info);

	/** The execute function see @ref tdialog for more information. */
	static bool execute(editor::editor_team_info& info, CVideo& video)
	{
		return teditor_edit_side(info).show(video);
	}

private:
	void pre_show(twindow& window);
	void post_show(twindow& window);

	team::CONTROLLER& controller_;
	tgroup<team::CONTROLLER> controller_group;

	team::SHARE_VISION& share_vision_;
	tgroup<team::SHARE_VISION> vision_group;

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;
};
}

#endif /* ! GUI_DIALOGS_EDIT_LABEL_INCLUDED */
