/*
   Copyright (C) 2008 - 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_MP_STAGING_HPP_INCLUDED
#define GUI_DIALOGS_MP_STAGING_HPP_INCLUDED

#include "ai/configuration.hpp"
#include "gui/dialogs/dialog.hpp"
#include "gui/dialogs/multiplayer/plugin_executor.hpp"

#include "game_initialization/connect_engine.hpp"
#include "game_initialization/multiplayer.hpp"
#include "mp_game_settings.hpp"

class config;

namespace gui2
{

class ttoggle_button;
class ttoggle_panel;
class tslider;
class tlabel;
class tmenu_button;
class twidget;

class tmp_staging : public tdialog, private plugin_executor
{
public:
	tmp_staging(const config& cfg, ng::connect_engine& connect_engine);

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	void sync_changes();

	void on_controller_select(ng::side_engine& side, tgrid& row_grid);
	void on_ai_select(ng::side_engine& side, tmenu_button& ai_menu);

	void select_leader_callback(twindow& window, ng::side_engine& side, tgrid& row_grid);

	void update_leader_display(ng::side_engine& side, tgrid& row_grid);

	ng::connect_engine& connect_engine_;

	std::vector<ai::description*> ai_algorithms_;

};

} // namespace gui2

#endif
