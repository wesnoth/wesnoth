/* $Id$ */
/*
   Copyright (C) 2009 by Mark de Wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
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
	tcampaign_selection(
			const config::child_list::const_iterator& begin,
			const config::child_list::const_iterator& end)
		: begin_(begin)
		, end_(end)
		, choice_(-1)

	{
	}

	int get_choice() const { return choice_; }

	/** Called when another campaign is selected. */
	void campaign_selected(twindow& window);

private:

	/** Inherited from tdialog. */
	twindow* build_window(CVideo& video);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	/** Config which contains the list with the campaigns. */

	/** Iterator to the start of the campaign list. */
	const config::child_list::const_iterator& begin_;

	/** Iterator to the end of the campaign list. */
	const config::child_list::const_iterator& end_;

	/** The choosen campaign. */
	int choice_;
};

} // namespace gui2

#endif
