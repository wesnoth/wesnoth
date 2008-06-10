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

#ifndef GUI_DIALOGS_DIALOG_HPP_INCLUDED
#define GUI_DIALOGS_DIALOG_HPP_INCLUDED

#include <string>

class CVideo;

namespace gui2 {

class twindow;

/**
 * Abstract base class for all dialogs.
 *
 * A dialog shows a certain window instance to the user. The subclasses of this
 * class will hold the parameters used for a certain window, eg a server
 * connection dialog will hold the name of the selected server as parameter that
 * way the caller doesn't need to know about the 'contents' of the window.
 */
class tdialog
{
public:
	tdialog() : 
		retval_(0)
	{}

	/** Shows the window */
	void show(CVideo& video);

	int get_retval() const { return retval_; }

private:
	/** Returns the window exit status, 0 means not shown. */
	int retval_;

	/**
	 * Builds the window.
	 *
	 * Every dialog shows it's own kind of window, this function should return
	 * the window to show.
	 *
	 * @returns                   The window to show.
	 */
	virtual twindow build_window(CVideo& video) = 0;

	/**
	 * Actions to be taken before showing the window.
	 *
	 * @param video               The video which contains the surface to draw
	 *                            upon.
	 * @param window              The window to be shown.
	 */
	virtual void pre_show(CVideo& video, twindow& window) {}

	/**
	 * Actions to be taken after the window has been shown.
	 *
	 * @param window              The window which has been shown.
	 */
	virtual void post_show(twindow& window) {}
};

} // namespace gui2

#endif

