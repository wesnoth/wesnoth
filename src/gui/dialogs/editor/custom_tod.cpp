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

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "gui/dialogs/editor/custom_tod.hpp"

#include "desktop/clipboard.hpp"
#include "display.hpp"
#include "filesystem.hpp"
#include "formatter.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/field.hpp"
#include "gui/dialogs/file_dialog.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/slider.hpp"
#include "gui/widgets/text_box.hpp"

#include <functional>

namespace gui2::dialogs
{

static custom_tod::string_pair tod_getter_image(const time_of_day& tod)
{
	static std::string type = "image";
	return {type, tod.image};
}

static custom_tod::string_pair tod_getter_mask(const time_of_day& tod)
{
	static std::string type = "mask";
	return {type, tod.image_mask};
}

static custom_tod::string_pair tod_getter_sound(const time_of_day& tod)
{
	static std::string type = "sound";
	return {type, tod.sounds};
}

REGISTER_DIALOG(custom_tod)

custom_tod::custom_tod(const std::vector<time_of_day>& times, int current_time)
	: modal_dialog(window_id())
	, times_(times)
	, current_tod_(current_time)
	, color_field_r_(register_integer("tod_red",   true))
	, color_field_g_(register_integer("tod_green", true))
	, color_field_b_(register_integer("tod_blue",  true))
{
	if(times_.empty())
	{
		times_.push_back(time_of_day());
	}
}

void custom_tod::pre_show(window& window)
{
	static std::map<std::string, tod_attribute_getter> metadata_stuff {
		{"image", tod_getter_image},
		{"mask",  tod_getter_mask },
		{"sound", tod_getter_sound}
	};

	window.add_to_tab_order(find_widget<text_box>(&window, "tod_name", false, true));
	window.add_to_tab_order(find_widget<text_box>(&window, "tod_desc", false, true));
	window.add_to_tab_order(find_widget<text_box>(&window, "tod_id", false, true));

	for(const auto& data : metadata_stuff) {
		find_widget<text_box>(&window, "path_" + data.first, false).set_active(false);

		button& copy_w = find_widget<button>(&window, "copy_" + data.first, false);

		connect_signal_mouse_left_click(copy_w,
			std::bind(&custom_tod::copy_to_clipboard_callback, this, data.second));

		if(!desktop::clipboard::available()) {
			copy_w.set_active(false);
			copy_w.set_tooltip(_("Clipboard support not found, contact your packager"));
		}
	}

	connect_signal_mouse_left_click(
			find_widget<button>(&window, "browse_image", false),
			std::bind(&custom_tod::select_file<tod_getter_image>, this, "data/core/images/misc"));

	connect_signal_mouse_left_click(
			find_widget<button>(&window, "browse_mask", false),
			std::bind(&custom_tod::select_file<tod_getter_mask>,  this, "data/core/images"));

	connect_signal_mouse_left_click(
			find_widget<button>(&window, "browse_sound", false),
			std::bind(&custom_tod::select_file<tod_getter_sound>, this, "data/core/sounds/ambient"));

	connect_signal_mouse_left_click(
			find_widget<button>(&window, "next_tod", false),
			std::bind(&custom_tod::do_next_tod, this));

	connect_signal_mouse_left_click(
			find_widget<button>(&window, "previous_tod", false),
			std::bind(&custom_tod::do_prev_tod, this));

	connect_signal_mouse_left_click(
			find_widget<button>(&window, "new", false),
			std::bind(&custom_tod::do_new_tod, this));

	connect_signal_mouse_left_click(
			find_widget<button>(&window, "delete", false),
			std::bind(&custom_tod::do_delete_tod, this));

	connect_signal_mouse_left_click(
			find_widget<button>(&window, "preview", false),
			std::bind(&custom_tod::preview_schedule, this));

	connect_signal_notify_modified(
			find_widget<slider>(&window, "lawful_bonus", false),
			std::bind(&custom_tod::update_lawful_bonus, this));

	connect_signal_notify_modified(
			*(color_field_r_->get_widget()),
			std::bind(&custom_tod::color_slider_callback, this, COLOR_R));

	connect_signal_notify_modified(
			*(color_field_g_->get_widget()),
			std::bind(&custom_tod::color_slider_callback, this, COLOR_G));

	connect_signal_notify_modified(
			*(color_field_b_->get_widget()),
			std::bind(&custom_tod::color_slider_callback, this, COLOR_B));

	update_selected_tod_info();
}

template<custom_tod::string_pair(*fptr)(const time_of_day&)>
void custom_tod::select_file(const std::string& default_dir)
{
	const string_pair& data = (*fptr)(get_selected_tod());

	std::string fn = filesystem::base_name(data.second);
	std::string dn = filesystem::directory_name(fn);
	if(dn.empty()) {
		dn = default_dir;
	}

	gui2::dialogs::file_dialog dlg;

	dlg.set_title(_("Choose File"))
	   .set_ok_label(_("Select"))
	   .set_path(dn)
	   .set_read_only(true);

	if(dlg.show()) {
		dn = dlg.path();

		if(data.first == "image") {
			times_[current_tod_].image = dn;
		} else if(data.first == "mask") {
			times_[current_tod_].image_mask = dn;
		} else if(data.first == "sound") {
			times_[current_tod_].sounds = dn;
		}
	}

	update_selected_tod_info();
}

void custom_tod::do_next_tod()
{
	current_tod_ = (current_tod_ + 1) % times_.size();
	update_selected_tod_info();
}

void custom_tod::do_prev_tod()
{
	current_tod_ = (current_tod_ ? current_tod_ : times_.size()) - 1;
	update_selected_tod_info();
}

void custom_tod::do_new_tod()
{
	times_.insert(times_.begin() + current_tod_, time_of_day());
	update_selected_tod_info();
}

void custom_tod::do_delete_tod()
{
	assert(times_.begin() + current_tod_ < times_.end());

	if(times_.size() == 1) {
		times_.emplace_back();
	} else {
		times_.erase(times_.begin() + current_tod_);

		if(times_.begin() + current_tod_ >= times_.end()) {
			current_tod_ = times_.size() - 1;
		}
	}

	update_selected_tod_info();
}

const time_of_day& custom_tod::get_selected_tod() const
{
	try {
		return times_.at(current_tod_);
	} catch(const std::out_of_range&) {
		throw std::string("Attempted to fetch a non-existant ToD!");
	}
}

void custom_tod::color_slider_callback(COLOR_TYPE type)
{
	time_of_day& current_tod = times_[current_tod_];

	switch(type)
	{
	case COLOR_R:
		current_tod.color.r = color_field_r_->get_widget_value();
		break;
	case COLOR_G :
		current_tod.color.g = color_field_g_->get_widget_value();
		break;
	case COLOR_B :
		current_tod.color.b = color_field_b_->get_widget_value();
		break;
	}

	update_tod_display();
}

void custom_tod::update_tod_display()
{
	display* disp = display::get_singleton();
	assert(disp && "Display pointer is null!");

	// The display handles invaliding whatever tiles need invalidating.
	disp->update_tod(&get_selected_tod());

	// NOTE: revert to invalidate_layout if necessary to display the ToD mask image.
	get_window()->queue_redraw();
}

void custom_tod::update_lawful_bonus()
{
	times_[current_tod_].lawful_bonus = find_widget<slider>(get_window(), "lawful_bonus", false).get_value();
}

void custom_tod::update_selected_tod_info()
{
	const time_of_day& current_tod = get_selected_tod();

	find_widget<text_box>(get_window(), "tod_name", false).set_value(current_tod.name);
	find_widget<text_box>(get_window(), "tod_desc", false).set_value(current_tod.description);
	find_widget<text_box>(get_window(), "tod_id", false).set_value(current_tod.id);

	find_widget<text_box>(get_window(), "path_image", false).set_value(current_tod.image);
	find_widget<text_box>(get_window(), "path_mask", false).set_value(current_tod.image_mask);
	find_widget<text_box>(get_window(), "path_sound", false).set_value(current_tod.sounds);

	find_widget<image>(get_window(), "current_tod_image", false).set_image(current_tod.image);
	find_widget<image>(get_window(), "current_tod_mask", false).set_image(current_tod.image_mask);

	find_widget<slider>(get_window(), "lawful_bonus", false).set_value(current_tod.lawful_bonus);

	color_field_r_->set_widget_value(current_tod.color.r);
	color_field_g_->set_widget_value(current_tod.color.g);
	color_field_b_->set_widget_value(current_tod.color.b);

	const std::string new_index_str = formatter() << (current_tod_ + 1) << "/" << times_.size();
	find_widget<label>(get_window(), "tod_number", false).set_label(new_index_str);

	update_tod_display();
}

void custom_tod::copy_to_clipboard_callback(tod_attribute_getter getter)
{
	desktop::clipboard::copy_to_clipboard(getter(get_selected_tod()).second, false);
}

/** Quickly preview the schedule changes and color */
void custom_tod::preview_schedule()
{
	update_map_and_schedule_(times_);
}

void custom_tod::update_schedule()
{
	/* Update times_ with values from the dialog */
	times_[current_tod_].name = find_widget<text_box>(get_window(), "tod_name", false).get_value();
	times_[current_tod_].description = find_widget<text_box>(get_window(), "tod_desc", false).get_value();
	times_[current_tod_].id = find_widget<text_box>(get_window(), "tod_id", false).get_value();

	times_[current_tod_].image = find_widget<text_box>(get_window(), "path_image", false).get_value();
	times_[current_tod_].image_mask = find_widget<text_box>(get_window(), "path_mask", false).get_value();
	times_[current_tod_].sounds = find_widget<text_box>(get_window(), "path_sound", false).get_value();

	times_[current_tod_].lawful_bonus = find_widget<slider>(get_window(), "lawful_bonus", false).get_value();

	times_[current_tod_].color.r = color_field_r_->get_widget_value();
	times_[current_tod_].color.g = color_field_g_->get_widget_value();
	times_[current_tod_].color.b = color_field_b_->get_widget_value();
}

const std::vector<time_of_day> custom_tod::get_schedule()
{
	update_schedule();
	return times_;
}

void custom_tod::register_callback(std::function<void(std::vector<time_of_day>)> update_func)
{
	update_map_and_schedule_ = update_func;
}

void custom_tod::post_show(window& /*window*/)
{
	update_tod_display();
}

} // namespace dialogs
