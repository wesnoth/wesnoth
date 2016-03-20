/*
   Copyright (C) 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_UNIT_RECRUIT_HPP_INCLUDED
#define GUI_DIALOGS_UNIT_RECRUIT_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "team.hpp"

class unit_type;

namespace gui2 {

class tunit_recruit : public tdialog
{
public:
	tunit_recruit(std::vector<const unit_type*>& recruit_list, team& team);

	int get_selected_index() const
	{
		return selected_index_;
	}

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(twindow& window);
	void post_show(twindow& window);

	void list_item_clicked(twindow& window);

	void show_help(twindow& window);

	std::vector<const unit_type*>& recruit_list_;

	team& team_;

	int selected_index_;
};

} // namespace gui2

#endif /* ! GUI_DIALOGS_UNIT_RECRUIT_HPP_INCLUDED */
