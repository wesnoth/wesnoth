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

#ifndef GUI_DIALOGS_SP_CONFIGURE_OPTIONS_HPP_INCLUDED
#define GUI_DIALOGS_SP_CONFIGURE_OPTIONS_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "gui/dialogs/multiplayer/plugin_executor.hpp"
#include "gui/dialogs/multiplayer/mp_options_helper.hpp"

#include "game_initialization/create_engine.hpp"
#include "game_initialization/configure_engine.hpp"

namespace gui2
{

class tsp_options_configure : public tdialog, private plugin_executor
{
public:
	explicit tsp_options_configure(ng::create_engine& create_engine);

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	ng::create_engine& create_engine_;
	std::unique_ptr<ng::configure_engine> config_engine_;
	std::unique_ptr<tmp_options_helper> options_manager_;
};

} // namespace gui2

#endif
