/*
 Part of the Battle for Wesnoth Project http://www.wesnoth.org/
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.
 
 See the COPYING file for more details.
 */

#ifndef GUI_DIALOGS_SELECT_ORB_COLORS_HPP_INCLUDED
#define GUI_DIALOGS_SELECT_ORB_COLORS_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include <map>

namespace gui2 {

class ttoggle_button;

class tselect_orb_colors : public tdialog {
public:
	tselect_orb_colors();

	/**
	 * The display function.
	 *
	 * See @ref tdialog for more information.
	 */
	static void display(CVideo& video) {
		tselect_orb_colors().show(video);
	}
private:
	void setup_orb_group(const std::string& base_id, bool& shown, std::string& color, twindow& window, bool connect = true);
	void handle_orb_click(ttoggle_button* clicked, const std::vector<ttoggle_button*>& group, std::string& storage);
	void handle_toggle_click(bool& storage);
	void handle_reset_click(twindow& window);

	bool show_unmoved_, show_partial_, show_moved_, show_ally_, show_enemy_;
	std::string unmoved_, partial_, moved_, ally_, enemy_;
	std::map<std::string, std::vector<ttoggle_button*> > groups_;

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);
	/** Inherited from tdialog. */
	void post_show(twindow& window);
};

}

#endif
