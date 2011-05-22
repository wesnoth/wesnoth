/*
   Copyright (C) 2011 by Lukasz Dobrogowski <lukasz.dobrogowski@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_MP_CHANGE_CONTROL_HPP_INCLUDED
#define GUI_DIALOGS_MP_CHANGE_CONTROL_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "menu_events.hpp"

#include <boost/shared_ptr.hpp>

namespace gui2 {

class tmp_change_control : public tdialog {
public:
	class model;
	class view;
	class controller;

	explicit tmp_change_control(events::menu_handler *mh);
	boost::shared_ptr<view> get_view();

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);
	void post_show(twindow& window);

	events::menu_handler *menu_handler_;
	boost::shared_ptr<view> view_;
};

}

#endif /* ! GUI_DIALOGS_MP_CHANGE_CONTROL_HPP_INCLUDED */
