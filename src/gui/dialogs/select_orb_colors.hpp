/*
   Copyright (C) 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

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

#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/group.hpp"
#include <map>

namespace gui2
{

class toggle_button;

namespace dialogs
{

class select_orb_colors : public modal_dialog {
public:
	select_orb_colors();

	/**
	 * The display function.
	 *
	 * See @ref modal_dialog for more information.
	 */
	static void display(CVideo& video) {
		select_orb_colors().show(video);
	}
private:
	void setup_orb_group(const std::string& base_id, bool& shown, const std::string& initial, window& window, bool connect = true);
	void handle_toggle_click(bool& storage);
	void handle_reset_click(window& window);

	bool show_unmoved_, show_partial_, show_moved_, show_ally_, show_enemy_;
	std::map<std::string, group<std::string> > groups_;

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from modal_dialog. */
	void pre_show(window& window);
	/** Inherited from modal_dialog. */
	void post_show(window& window);
};

} // namespace dialogs
} // namespace gui2

#endif
