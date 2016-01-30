/*
   Copyright (C) 2009 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_CAMPAIGN_SELECTION_HPP_INCLUDED
#define GUI_DIALOGS_CAMPAIGN_SELECTION_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

#include "config.hpp"
#include "game_initialization/create_engine.hpp"

namespace gui2
{

class tcampaign_selection : public tdialog
{
public:
	explicit tcampaign_selection(ng::create_engine& eng) :
		engine_(eng),
		choice_(-1),
		deterministic_(false)
	{
	}

	/***** ***** ***** setters / getters for members ***** ****** *****/

	int get_choice() const
	{
		return choice_;
	}

	bool get_deterministic() const
	{
		return deterministic_;
	}

private:
	/** Called when another campaign is selected. */
	void campaign_selected(twindow& window);

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	void show_settings(CVideo& video);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);


	void mod_toggled(int id, twidget &);

	ng::create_engine& engine_;

	/** The chosen campaign. */
	int choice_;

	/** whether the player checked the "Deterministic" checkbox. */
	bool deterministic_;
};

} // namespace gui2

#endif
