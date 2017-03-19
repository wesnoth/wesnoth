/*
   Copyright (C) 2016 - 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gui/dialogs/modal_dialog.hpp"
#include "tstring.hpp"

#include <map>
#include <vector>
#include <atomic>

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

class label;
class window;

namespace dialogs
{

class loading_screen : public modal_dialog
{
public:

	loading_screen(std::function<void()> f);

	~loading_screen();

	static void display(CVideo& video, std::function<void()> f);
	static bool displaying() { return current_load != nullptr; }

	static void progress(const char* stage_name = nullptr);

	/**
	 * Hides the window.
	 *
	 * The hiding also destroys the window. It is safe to call the function
	 * when the window is not shown.
	 */
	void close();
private:
	window* window_;
	size_t timer_id_;
	int animation_counter_;
	std::function<void()> work_;
	std::unique_ptr<boost::thread> worker_;
	std::unique_ptr<cursor::setter> cursor_setter_;
	std::exception_ptr exception_;
	void clear_timer();

	window* build_window(CVideo& video) const;

	virtual const std::string& window_id() const;

	void timer_callback(window& window);

	/** Inherited from modal_dialog. */
	void pre_show(window& window);

	/** Inherited from modal_dialog. */
	void post_show(window& window);

	label* progress_stage_label_;
	label* animation_label_;
	static loading_screen* current_load;

#if defined(_MSC_VER) && _MSC_VER < 1900
	// std::atomic is buggy in MSVC 2013 - doesn't work for cv types
	const char* current_stage_;
#else
	std::atomic<const char*> current_stage_;
#endif
	std::map<std::string, t_string> visible_stages_;
	std::vector<t_string> animation_stages_;
	std::map<std::string, t_string>::const_iterator current_visible_stage_;

	bool is_worker_running_;
};

} // namespace dialogs
} // namespace gui2
