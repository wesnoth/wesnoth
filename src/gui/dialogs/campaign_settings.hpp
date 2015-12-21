/*
   Copyright (C) 2014 - 2015 by Nathan Walker <nathan.b.walker@vanderbilt.edu>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_CAMPAIGN_SETTINGS_HPP_INCLUDED
#define GUI_DIALOGS_CAMPAIGN_SETTINGS_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

#include "addon/validation.hpp"
#include "addon/state.hpp"

#include "game_initialization/create_engine.hpp"

namespace gui2
{

class tcampaign_settings : public tdialog
{
public:
	tcampaign_settings(ng::create_engine& eng);

	// whether configure is enabled
	bool enable_configure();

	// whether connect is enabled
	bool enable_connect();

private:
	ng::create_engine& engine_;

	// called on era change
	// sets era via create_engine
	void change_era(twindow& window);

	// called on mod change
	// sets mod via create_engine
	void change_mod(size_t index, twindow& window);

	// populate era and mod lists
	void update_lists(twindow& window);

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

};

} // end namespace gui2
#endif
