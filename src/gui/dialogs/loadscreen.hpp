/*
   Copyright (C) 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gui/dialogs/dialog.hpp"

class CVideo;

namespace gui2
{

class twindow;

class tloadscreen : public tdialog
{
public:
	tloadscreen()
		: window_(NULL)
	{
	}

	~tloadscreen()
	{
		close();
	}

	static void display(CVideo& video) {
		tloadscreen().show(video);
	}

	void show(CVideo& video);

	/**
	 * Hides the window.
	 *
	 * The hiding also destroys the window. It is save to call the function
	 * when the window is not shown.
	 */
	void close();

private:
	twindow* window_;

	twindow* build_window(CVideo& video) const;

	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
};

} // namespace gui2
