/*
	Copyright (C) 2008 - 2021
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
	: times_(times)
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

	connect_signal_notify_modified(
			find_widget<slider>(&window, "lawful_bonus", false),
			std::bind(&custom_tod::update_lawful_bonus, this));

	connect_signal_notify_modified(
			*(color_field_r_->get_widget()),
			std::bind(&custom_tod::color_slider_callback, this));

	connect_signal_notify_modified(
			*(color_field_g_->get_widget()),
			std::bind(&custom_tod::color_slider_callback, this));

	connect_signal_notify_modified(
			*(color_field_b_->get_widget()),
			std::bind(&custom_tod::color_slider_callback, this));

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

void custom_tod::color_slider_callback()
{
	time_of_day& current_tod = times_[current_tod_];

	current_tod.color.r = color_field_r_->get_widget_value(*get_window());
	current_tod.color.g = color_field_g_->get_widget_value(*get_window());
	current_tod.color.b = color_field_b_->get_widget_value(*get_window());

	update_tod_display();
}

void custom_tod::update_tod_display()
{
	display* disp = display::get_singleton();
	assert(disp && "Display pointer is null!");

	// Prevent a floating slice of window appearing alone over the
	// theme UI sidebar after redrawing tiles and before we have a
	// chance to redraw the rest of this window.
	get_window()->undraw();

	// NOTE: We only really want to re-render the gamemap tiles here.
	// Redrawing everything is a significantly more expensive task.
	// At this time, tiles are the only elements on which ToD tint is
	// meant to have a visible effect. This is very strongly tied to
	// the image caching mechanism.
	//
	// If this ceases to be the case in the future, you'll need to call
	// redraw_everything() instead.

	disp->update_tod(&get_selected_tod());

	// invalidate all tiles so they are redrawn with the new ToD tint next
	disp->invalidate_all();

	// redraw tiles
	disp->draw(false);

	// NOTE: revert to invalidate_layout if necessary to display the ToD mask image.
	get_window()->set_is_dirty(true);
}

void custom_tod::update_lawful_bonus()
{
	times_[current_tod_].lawful_bonus = find_widget<slider>(get_window(), "lawful_bonus", false).get_value();
}

void custom_tod::update_selected_tod_info()
{
	const time_of_day& current_tod = get_selected_tod();

	find_widget<text_box>(get_window(), "tod_name", false).set_value(current_tod.name);
	find_widget<text_box>(get_window(), "tod_id", false).set_value(current_tod.id);

	find_widget<text_box>(get_window(), "path_image", false).set_value(current_tod.image);
	find_widget<text_box>(get_window(), "path_mask", false).set_value(current_tod.image_mask);
	find_widget<text_box>(get_window(), "path_sound", false).set_value(current_tod.sounds);

	find_widget<image>(get_window(), "current_tod_image", false).set_image(current_tod.image);
	find_widget<image>(get_window(), "current_tod_mask", false).set_image(current_tod.image_mask);

	find_widget<slider>(get_window(), "lawful_bonus", false).set_value(current_tod.lawful_bonus);

	color_field_r_->set_widget_value(*get_window(), current_tod.color.r);
	color_field_g_->set_widget_value(*get_window(), current_tod.color.g);
	color_field_b_->set_widget_value(*get_window(), current_tod.color.b);

	const std::string new_index_str = formatter() << (current_tod_ + 1) << "/" << times_.size();
	find_widget<label>(get_window(), "tod_number", false).set_label(new_index_str);

	update_tod_display();
}

void custom_tod::copy_to_clipboard_callback(tod_attribute_getter getter)
{
	desktop::clipboard::copy_to_clipboard(getter(get_selected_tod()).second, false);
}

void custom_tod::post_show(window& /*window*/)
{
	update_tod_display();

	if(get_retval() == retval::OK) {
		// TODO: save ToD
	}
}

} // namespace dialogs
