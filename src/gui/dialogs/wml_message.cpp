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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/wml_message.hpp"

#include "gui/widgets/window.hpp"

namespace gui2 {

void twml_message_::pre_show(CVideo& video, twindow& window)
{
	// Inherited.
	tmessage::pre_show(video, window);

	window.canvas(1).set_variable("portrait_image", variant(portrait_));
	window.canvas(1).set_variable("portrait_mirror", variant(mirror_));
}

twindow* twml_message_left::build_window(CVideo& video)
{
	return build(video, get_id(WML_MESSAGE_LEFT));
}

twindow* twml_message_right::build_window(CVideo& video)
{
	return build(video, get_id(WML_MESSAGE_RIGHT));
}

} // namespace gui2

