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

#ifndef GUI_DIALOGS_LABEL_SETTINGS_HPP_INCLUDED
#define GUI_DIALOGS_LABEL_SETTINGS_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include <map>
#include "display_context.hpp"
#include "tstring.hpp"

namespace gui2 {

class tlabel_settings : public tdialog {
public:
	tlabel_settings(display_context& dc);
	
	/**
	 * The execute function.
	 *
	 * See @ref tdialog for more information.
	 */
	static bool execute(display_context& dc, CVideo& video);
private:
	std::map<std::string, bool> all_labels;
	std::map<std::string, t_string> labels_display;
	display_context& viewer;
	
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;
	
	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);
	
	/** Callback for toggling a checkbox state. */
	void toggle_category(twidget& box, std::string category);
};
}

#endif
