/*
	Copyright (C) 2008 - 2024
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
#include <functional>

namespace gui2::dialogs
{
class custom_tod : public modal_dialog
{
public:
	custom_tod(const std::vector<time_of_day>& times, int current_time, const std::string& addon_id = "");

	/** The execute function. See @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(custom_tod)

	using string_pair = std::pair<std::string, std::string>;
	using tod_attribute_getter = std::function<string_pair(const time_of_day&)>;

	/** Return current schedule */
	const std::vector<time_of_day> get_schedule();

	/** Register callback for update */
	void register_callback(std::function<void(std::vector<time_of_day>)>);

	/** enum used in identifying sliders */
	enum COLOR_TYPE {COLOR_R, COLOR_G, COLOR_B};

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	virtual void post_show() override;

	/** Callback for the next tod button */
	void do_next_tod();
	void do_prev_tod();

	void do_new_tod();
	void do_delete_tod();

	/** Callback for preview button */
	void preview_schedule();

	template<custom_tod::string_pair(*fptr)(const time_of_day&)>
	void select_file(const std::string& default_dir);

	/* Callback for color sliders */
	void color_slider_callback(COLOR_TYPE type);

	/* Update map and schedule in realtime */
	std::function<void(std::vector<time_of_day>)> update_map_and_schedule_;

	void update_tod_display();

	void update_lawful_bonus();

	void set_selected_tod(time_of_day tod);
	const time_of_day& get_selected_tod() const;

	void update_selected_tod_info();

	void copy_to_clipboard_callback(const std::pair<std::string, tod_attribute_getter>& data);

	/** Update current TOD with values from the GUI */
	void update_schedule();

	/** Update image when preview is pressed */
	void update_image(const std::string& id_stem);

	/** Play sound when play is pressed */
	void play_sound();

	/** ID of the current addon. The schedule file will be saved here. */
	const std::string addon_id_;

	/** Available time of days */
	std::vector<time_of_day> times_;

	/** Current time of day (ToD) index */
	int current_tod_;

	field_integer* color_field_r_;
	field_integer* color_field_g_;
	field_integer* color_field_b_;
};

} // namespace gui2::dialogs
