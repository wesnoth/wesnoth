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

#ifndef GUI_DIALOGS_EDITOR_GENERATE_MAP_HPP_INCLUDED
#define GUI_DIALOGS_EDITOR_GENERATE_MAP_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

class map_generator;

class display;

namespace gui2 {

class teditor_generate_map : public tdialog
{
public:
	teditor_generate_map();
	
	void do_settings(twindow& window);
	
	void set_map_generator(map_generator* mg) { map_generator_ = mg; }
	map_generator* get_map_generator() { return map_generator_; }
	
	void set_gui(display* d) { gui_ = d; }
	display* get_gui() { return gui_; }
	
private:
	/** Inherited from tdialog. */
	twindow build_window(CVideo& video);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
	
	map_generator* map_generator_;
	
	display* gui_;
};

} // namespace gui2

#endif
