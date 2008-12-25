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

#include <vector>

class map_generator;

class display;

namespace gui2 {

class tlabel;

class teditor_generate_map : public tdialog
{
public:
	teditor_generate_map();

	/** Callback for the settings button */
	void do_settings(twindow& window);

	/** Callback for the next generator button */
	void do_next_generator(twindow& window);

	void set_map_generators(std::vector<map_generator*> mg) { map_generators_ = mg; }
	std::vector<map_generator*> get_map_generators() { return map_generators_; }

	map_generator* get_selected_map_generator();

	void update_current_generator_label(twindow& window);

	void set_gui(display* d) { gui_ = d; }
	display* get_gui() { return gui_; }

private:
	/** Inherited from tdialog. */
	twindow* build_window(CVideo& video);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	/** Available map generators */
	std::vector<map_generator*> map_generators_;

	/** Current map generator index */
	int current_map_generator_;

	/** Label for the current map generator */
	tlabel* current_generator_label_;


	/** Needed for the old-style map generator settings dialog */
	display* gui_;
};

} // namespace gui2

#endif
