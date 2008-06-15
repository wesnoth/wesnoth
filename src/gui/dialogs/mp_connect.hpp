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

#ifndef GUI_DIALOGS_MP_CONNECT_HPP_INCLUDED
#define GUI_DIALOGS_MP_CONNECT_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

namespace gui2 {

class twidget;
class ttext_box;

class tmp_connect : public tdialog
{
public:
	tmp_connect() : 
		host_name_(),
		video_(0),
		host_name_widget_(0)
	{}

	const std::string& host_name() const { return host_name_; }

private:
	std::string host_name_;

	/** Used in show in order to show list. */
	CVideo* video_;
	
	/** Widget for the host name. */
	ttext_box* host_name_widget_;

	/** Inherited from tdialog. */
	twindow build_window(CVideo& video);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
public:
	void show_server_list();
};

} // namespace gui2

#endif

