/*
   Copyright (C) 2010 - 2015 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "config.hpp"
#include "gui/dialogs/dialog.hpp"

#include <vector>

namespace gui2
{

class tcampaign_difficulty : public tdialog
{
public:
	/**
	 * @param config of the campaign difficulty is being chosen for
	 */
	tcampaign_difficulty(const config& campaign);

	/**
	 * Returns the selected difficulty define after displaying.
	 * @return 'CANCEL' if the dialog was canceled.
	 */
	std::string selected_difficulty() const
	{
		return selected_difficulty_;
	}

private:
	config difficulties_;
	std::string campaign_id_;
	std::string selected_difficulty_;

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
};
}

#endif /* ! GUI_DIALOGS_CAMPAIGN_DIFFICULTY_HPP_INCLUDED */
