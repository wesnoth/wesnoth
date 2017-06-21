/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "time_of_day.hpp"

#include <vector>

namespace gui2
{

class label;
class image;
class slider;
class text_box;

namespace dialogs
{

class custom_tod : public modal_dialog
{
public:
	custom_tod(const std::vector<time_of_day>& times, int current_time);

	static bool execute(const std::vector<time_of_day>& times, int current_time, CVideo& video)
	{
		return custom_tod(times, current_time).show(video);
	}

	using string_pair = std::pair<std::string, std::string>;
	using tod_attribute_getter = std::function<string_pair(const time_of_day&)>;

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Inherited from modal_dialog. */
	virtual void post_show(window& window) override;

	/** Callback for the next tod button */
	void do_next_tod(window& window);
	void do_prev_tod(window& window);

	void do_new_tod(window& window);
	void do_delete_tod(window& window);

	template<custom_tod::string_pair(*fptr)(const time_of_day&)>
	void select_file(window& window, const std::string& default_dir);

	void color_slider_callback(window& window);

	void update_tod_display();

	void update_lawful_bonus(window& window);

	void set_selected_tod(time_of_day tod);
	const time_of_day& get_selected_tod() const;

	void update_selected_tod_info(window& window);

	void copy_to_clipboard_callback(tod_attribute_getter getter);

	/** Available time_of_days */
	std::vector<time_of_day> times_;

	/** Current ToD index */
	int current_tod_;

	field_integer* color_field_r_;
	field_integer* color_field_g_;
	field_integer* color_field_b_;
};

} // namespace dialogs
} // namespace gui2
