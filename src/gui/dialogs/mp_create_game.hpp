/* $Id$ */
/*
   copyright (c) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#ifndef GUI_DIALOGS_MP_CREATE_GAME_HPP_INCLUDED
#define GUI_DIALOGS_MP_CREATE_GAME_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

class config;

namespace gui2 {

class twidget;
class ttext_box;

class tmp_create_game : public tdialog
{
public:
	tmp_create_game(const config& cfg);

private:
	
	/** Inherited from tdialog. */
	twindow* build_window(CVideo& video);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	const config& cfg_;

	const config* scenario_;

	/**
	 * All fields are also in the normal field vector, but they need to be
	 * manually controled as well so add the pointers here as well.
	 */
	
	tfield_bool 
		*use_map_settings_,
		*fog_,
		*shroud_,
		*start_time_;
	
	tfield_integer
		*turns_,
		*gold_,
		*experience_;

public:

	// another map selected
	void update_map(twindow& window);

	// use_map_settings toggled (also called in other cases.)
	void update_map_settings(twindow& window);
};

} // namespace gui2

#endif

