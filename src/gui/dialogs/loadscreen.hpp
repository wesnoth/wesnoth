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
#include "gui/widgets/label.hpp"

#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>

class CVideo;
namespace boost
{
	class thread;
}
namespace cursor
{
	struct setter;
}
namespace gui2
{

class twindow;

class tloadscreen : public tdialog
{
public:

	tloadscreen(boost::function<void()> f);

	~tloadscreen();

	static void display(CVideo& video, boost::function<void()> f);
	static bool displaying() { return current_load != NULL; }

	void show(CVideo& video);
	
	static void progress(const char* stage_name = NULL);

	/**
	 * Hides the window.
	 *
	 * The hiding also destroys the window. It is save to call the function
	 * when the window is not shown.
	 */
	void close();
private:
	twindow* window_;
	size_t timer_id_;
	int animation_counter_;
	boost::function<void()> work_;
	boost::scoped_ptr<boost::thread> worker_;
	boost::scoped_ptr<cursor::setter> cursor_setter_;

	twindow* build_window(CVideo& video) const;

	virtual const std::string& window_id() const;

	void timer_callback(twindow& window);

	/** Inherited from tdialog. */
	void pre_show(twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	tlabel* progress_stage_label_;
	tlabel* animation_label_;
	static tloadscreen* current_load;

	//Note we cannot use std::strings here unless we we explicitly use mutexes.
	const char* current_stage_;
	const char* current_visible_stage_;
};

} // namespace gui2
