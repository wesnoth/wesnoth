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

#ifndef __GUI_DIALOGS_MP_METHOD_SELECTION_HPP_INCLUDED__
#define __GUI_DIALOGS_MP_METHOD_SELECTION_HPP_INCLUDED__


#include <string>

class CVideo;

namespace gui2 {

class tmp_method_selection
{
public:
	tmp_method_selection() : 
		retval_(0),
		user_name_(),
		choice_(-1)
	{}

	void show(CVideo& video);

	int get_retval() const { return retval_; }

	const std::string& user_name() const { return user_name_; }

	int get_choice() const { return choice_; }

private:
	int retval_;
	std::string user_name_;
	int choice_;
};

} // namespace gui2

#endif
