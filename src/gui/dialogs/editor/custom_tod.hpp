/*
	Copyright (C) 2008 - 2023
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
class slider;

namespace dialogs
{

/**
 * @ingroup GUIWindowDefinitionWML
 *
 * This shows the dialog to modify tod schedules.
 * Key               |Type          |Mandatory|Description
 * ------------------|--------------|---------|-----------
 * current_tod_name  | text_box     |yes      |The name of the time of day(ToD).
 * current_tod_id    | text_box     |yes      |The id of the time of day(ToD).
 * current_tod_image | @ref image   |yes      |The image for the time of day(ToD).
 * current_tod_mask  | @ref image   |yes      |The image mask for the time of day(ToD).
 * current_tod_sound | @ref label   |yes      |The sound for the time of day(ToD).
 * next_tod          | @ref button  |yes      |Selects the next ToD.
 * prev_tod          | @ref button  |yes      |Selects the previous ToD.
 * lawful_bonus      | @ref slider  |yes      |Sets the Lawful Bonus for the current ToD.
 * tod_red           | @ref slider  |yes      |Sets the red component of the current ToD.
 * tod_green         | @ref slider  |yes      |Sets the green component of the current ToD.
 * tod_blue          | @ref slider  |yes      |Sets the blue component of the current ToD.
 */
class custom_tod : public modal_dialog
{
public:
	custom_tod(const std::vector<time_of_day>& times, int current_time);

	/** The execute function. See @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(custom_tod)

	using string_pair = std::pair<std::string, std::string>;
	using tod_attribute_getter = std::function<string_pair(const time_of_day&)>;

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	virtual void post_show(window& window) override;

	/** Callback for the next tod button */
	void do_next_tod();
	void do_prev_tod();

	void do_new_tod();
	void do_delete_tod();

	template<custom_tod::string_pair(*fptr)(const time_of_day&)>
	void select_file(const std::string& default_dir);

	void color_slider_callback();

	void update_tod_display();

	void update_lawful_bonus();

	void set_selected_tod(time_of_day tod);
	const time_of_day& get_selected_tod() const;

	void update_selected_tod_info();

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
