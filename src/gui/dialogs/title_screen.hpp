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

#ifndef GUI_DIALOGS_TITLE_SCREEN_HPP_INCLUDED
#define GUI_DIALOGS_TITLE_SCREEN_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

namespace gui2 {

class ttitle_screen : public tdialog
{
public:
	ttitle_screen() :
		video_(NULL)
	{
	}

	CVideo* video() { return video_; }

private:
	/** Used in show in order to show child windows. */
	CVideo* video_;

	/** Inherited from tdialog. */
	twindow* build_window(CVideo& video);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
};

} // namespace gui2

#endif
