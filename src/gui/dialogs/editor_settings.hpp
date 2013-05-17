/*
   Copyright (C) 2008 - 2013 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_EDITOR_SETTINGS_HPP_INCLUDED
#define GUI_DIALOGS_EDITOR_SETTINGS_HPP_INCLUDED

#include "time_of_day.hpp"
#include "gui/dialogs/dialog.hpp"

#include <vector>

namespace editor {

class editor_display;

} // namespace editor

namespace gui2 {

class tlabel;
class ttoggle_button;

class teditor_settings : public tdialog
{
public:
	teditor_settings(editor::editor_display* display
			, const std::vector<time_of_day>& tods);

	static bool execute(editor::editor_display* display
			, const std::vector<time_of_day>& tods
			, CVideo& video)
	{
		return teditor_settings(display, tods).show(video);
	}

private:

	/** Callback for the next tod button */
	void do_next_tod(twindow& window);

	void update_tod_display(twindow& window);
	void update_tod_display_fast(twindow& window);

	void slider_update_callback(twindow& window);

	void set_selected_tod(time_of_day tod);
	const time_of_day& get_selected_tod() const;

	void update_selected_tod_info(twindow& window);

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	/** Available time_of_days */
	std::vector<time_of_day> tods_;

	/** Current map generator index */
	int current_tod_;

	/** Label for the current tod */
	tlabel* current_tod_label_;

	/** Label for the current tod image*/
	tlabel* current_tod_image_;

	ttoggle_button* custom_tod_toggle_;
	ttoggle_button* custom_tod_auto_refresh_;

	tfield_bool* custom_tod_toggle_field_;

	tfield_integer* custom_tod_red_field_;
	tfield_integer* custom_tod_green_field_;
	tfield_integer* custom_tod_blue_field_;

	/**
	 * The display to update when the ToD changes.
	 *
	 * The pointer may be NULL, in the unit tests, but normally it should be a
	 * pointer to a valid object.
	 */
	editor::editor_display* display_;

	bool can_update_display_;
};

} // namespace gui2

#endif
