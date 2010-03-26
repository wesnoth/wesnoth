/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_CAMPAIGN_SELECTION_HPP_INCLUDED
#define GUI_DIALOGS_CAMPAIGN_SELECTION_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

#include "config.hpp"

namespace gui2 {

class tcampaign_selection
	: public tdialog
{
public:
	tcampaign_selection(const std::vector<config> &c)
		: campaigns_(c)
		, choice_(-1)

	{
	}

	int get_choice() const { return choice_; }

	/** Called when another campaign is selected. */
	void campaign_selected(twindow& window);

private:

	/** Inherited from tdialog, implemented by REGISTER_WINDOW. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	/** Config which contains the list with the campaigns. */

	/** Campaign list. */
	const std::vector<config> &campaigns_;

	/** The choosen campaign. */
	int choice_;
};

} // namespace gui2

#endif
