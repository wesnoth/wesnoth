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

#ifndef GUI_DIALOGS_ADDON_CONNECT_HPP_INCLUDED
#define GUI_DIALOGS_ADDON_CONNECT_HPP_INCLUDED

#include <string>

class CVideo;

namespace gui2 {

class taddon_connect
{
public:
	taddon_connect() : 
		retval_(0),
		host_name_()
	{}

	void show(CVideo& video);

	int get_retval() const { return retval_; }

	const std::string& host_name() const { return host_name_; }

	void set_host_name(const std::string& host_name) 
		{ host_name_ = host_name; }

private:
	int retval_;
	std::string host_name_;

};

} // namespace gui2

#endif
