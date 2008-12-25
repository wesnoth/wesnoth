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

#ifndef GUI_DIALOGS_MP_METHOD_SELECTION_HPP_INCLUDED
#define GUI_DIALOGS_MP_METHOD_SELECTION_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

namespace gui2 {

class tmp_method_selection : public tdialog
{
public:
	tmp_method_selection() :
		user_name_(),
		choice_(-1)
	{}

	const std::string& user_name() const { return user_name_; }

	int get_choice() const { return choice_; }

private:
	std::string user_name_;
	int choice_;


	/** Inherited from tdialog. */
	twindow* build_window(CVideo& video);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
};

} // namespace gui2

#endif
