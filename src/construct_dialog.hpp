/* $Id$ */
/*
   Copyright (C) 2006 by Patrick Parker <patrick_x99@hotmail.com>
   wesnoth widget Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef CONSTRUCT_DIALOG_H_INCLUDED
#define CONSTRUCT_DIALOG_H_INCLUDED

#include "basic_dialog.hpp"

namespace gui {

class dialog : public gui::basic_dialog
{
public:
	dialog(display &disp, const std::string& title="", 
	       const std::string& message="",
	       const DIALOG_TYPE type=MESSAGE, 
	       const struct style *dialog_style=&default_style,
	       const std::string& help_topic="");

	class help_button : public dialog_button {
	public:
		help_button(display& disp, const std::string &help_topic);
		int action(dialog_process_info &info);
		const std::string topic() const { return topic_; }
	private:
		display &disp_;
		const std::string topic_;
	};

	int process(dialog_process_info &info);
	void update_widget_positions();
	dialog_frame& get_frame();
	int show();
	int show(int xloc, int yloc);

	help_button help_button_;
};

} //end namespace gui

#endif
