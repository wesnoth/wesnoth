/*
   Copyright (C) 2010 - 2014 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_CAMPAIGN_DIFFICULTY_HPP_INCLUDED
#define GUI_DIALOGS_CAMPAIGN_DIFFICULTY_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

#include "gui/auxiliary/old_markup.hpp"
#include <vector>

namespace gui2
{

class tcampaign_difficulty : public tdialog
{
public:
	explicit tcampaign_difficulty(const std::vector<std::string>& items);

	/**
	 * Returns the selected item index after displaying.
	 * @return -1 if the dialog was canceled.
	 */
	int selected_index() const
	{
		return index_;
	}

private:
	int index_;
	std::vector<tlegacy_menu_item> items_;

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
};
}


#endif /* ! GUI_DIALOGS_CAMPAIGN_DIFFICULTY_HPP_INCLUDED */
