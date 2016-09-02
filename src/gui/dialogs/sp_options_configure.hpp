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

	/**
	 * Execute function. We only want to show the dialog if there are active mods and
	 * those active mods all have custom options.
	 */
	static bool execute(ng::create_engine& create_engine, CVideo& video)
	{
		using mod_type = ng::create_engine::extras_metadata_ptr;

		const std::vector<mod_type>& activemods = create_engine.active_mods_data();
		if(std::none_of(activemods.begin(), activemods.end(), [](mod_type mod) {
			return (*mod->cfg).has_child("options");
		})) {
			return true;
		}

		return tsp_options_configure(create_engine).show(video);
	}

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
