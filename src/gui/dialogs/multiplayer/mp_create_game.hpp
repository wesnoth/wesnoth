/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_MP_CREATE_GAME_HPP_INCLUDED
#define GUI_DIALOGS_MP_CREATE_GAME_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

#include "game_initialization/create_engine.hpp"
#include "game_initialization/mp_options.hpp"

class config;

namespace gui2
{

class tmp_create_game : public tdialog
{
	typedef std::pair<ng::level::TYPE, std::string> level_type_info;

public:
	explicit tmp_create_game(const config& cfg, ng::create_engine& eng);

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	const config& cfg_;

	const config* scenario_;

	ng::create_engine& engine_;

	//mp::options::manager options_manager_;

	std::vector<level_type_info> level_types_;
	
	void update_games_list(twindow& window);
	void display_games_of_type(twindow& window, ng::level::TYPE type);

	/**
	 * All fields are also in the normal field vector, but they need to be
	 * manually controlled as well so add the pointers here as well.
	 */

	tfield_bool* use_map_settings_, *fog_, *shroud_, *start_time_;

	tfield_integer* turns_, *gold_, *support_, *experience_;

	void on_tab_select(twindow& window);

	void update_options_list(twindow& window);

public:
	// another map selected
	void update_map(twindow& window);

	// use_map_settings toggled (also called in other cases.)
	void update_map_settings(twindow& window);
};

} // namespace gui2

#endif
