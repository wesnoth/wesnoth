/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_EDITOR_SETTINGS_HPP_INCLUDED
#define GUI_DIALOGS_EDITOR_SETTINGS_HPP_INCLUDED

#include "time_of_day.hpp"
#include "gui/auxiliary/notifiee.hpp"
#include "gui/dialogs/dialog.hpp"

#include <vector>
#include <boost/function.hpp>

namespace gui2 {

class tlabel;
class ttoggle_button;
class tslider;

class teditor_settings : public tdialog
{
public:
	teditor_settings();

	void set_redraw_callback(boost::function<void (int, int, int)> callback) { redraw_callback_ = callback; }

	/** Callback for the next tod button */
	void do_next_tod(twindow& window);

	void update_tod_display(twindow& window);

	void slider_update_callback(twindow& window);

	void set_tods(const std::vector<time_of_day>& tods) { tods_ = tods; }
	const std::vector<time_of_day>& get_tods() const { return tods_; }

	void set_current_adjustment(int r, int g, int b);

	void set_selected_tod(time_of_day tod);
	const time_of_day& get_selected_tod() const;

	int get_red() const;
	int get_green() const;
	int get_blue() const;

	void update_selected_tod_info(twindow& window);

	bool get_use_mdi() const;
	void set_use_mdi(bool value);

private:
	/** Inherited from tdialog, implemented by REGISTER_WINDOW. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	boost::function<void (int, int, int)> redraw_callback_;

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

	tslider* custom_tod_red_;
	tslider* custom_tod_green_;
	tslider* custom_tod_blue_;

	tfield_integer* custom_tod_red_field_;
	tfield_integer* custom_tod_green_field_;
	tfield_integer* custom_tod_blue_field_;

	tfield_bool* use_mdi_field_;

	/***** ***** ***** callback notifiees ***** ****** *****/

	/** Notifiee for the custom tod red modification callback. */
	tnotifiee<boost::function<void(void)> > custom_tod_red_moved_notifiee_;

	/** Notifiee for the custom tod green modification callback. */
	tnotifiee<boost::function<void(void)> > custom_tod_green_moved_notifiee_;

	/** Notifiee for the custom tod blue modification callback. */
	tnotifiee<boost::function<void(void)> > custom_tod_blue_moved_notifiee_;
};

} // namespace gui2

#endif
