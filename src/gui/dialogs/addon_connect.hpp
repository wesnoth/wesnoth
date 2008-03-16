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

#ifndef __GUI_DIALOGS_ADDON_CONNECT_HPP_INCLUDED__
#define __GUI_DIALOGS_ADDON_CONNECT_HPP_INCLUDED__

#include <string>

class CVideo;

namespace gui2 {

	void addon_connect(CVideo& video, const std::string& server);

} // namespace gui2

#endif
