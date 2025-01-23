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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/editor/custom_tod.hpp"

#include "desktop/clipboard.hpp"
#include "display.hpp"
#include "filesystem.hpp"
#include "formatter.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/field.hpp"
#include "gui/dialogs/file_dialog.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/slider.hpp"
#include "gui/widgets/text_box.hpp"
#include "sound.hpp"

#include <boost/filesystem.hpp>
#include <functional>
#include <utility>

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

custom_tod::custom_tod(const std::vector<time_of_day>& times, int current_time, const std::string& addon_id)
	: modal_dialog(window_id())
	, addon_id_(addon_id)
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

void custom_tod::pre_show()
{
	static std::map<std::string, tod_attribute_getter> metadata_stuff {
		{"image", tod_getter_image},
		{"mask",  tod_getter_mask },
		{"sound", tod_getter_sound}
	};

	add_to_tab_order(find_widget<text_box>("tod_name", false, true));
	add_to_tab_order(find_widget<text_box>("tod_desc", false, true));
	add_to_tab_order(find_widget<text_box>("tod_id", false, true));

	for(const auto& data : metadata_stuff) {
		button& copy_w = find_widget<button>("copy_" + data.first);

		connect_signal_mouse_left_click(copy_w, std::bind(&custom_tod::copy_to_clipboard_callback, this, data));
	}

	connect_signal_mouse_left_click(
			find_widget<button>("browse_image"),
			std::bind(&custom_tod::select_file<tod_getter_image>, this, "data/core/images/misc"));

	connect_signal_mouse_left_click(
			find_widget<button>("browse_mask"),
			std::bind(&custom_tod::select_file<tod_getter_mask>,  this, "data/core/images"));

	connect_signal_mouse_left_click(
			find_widget<button>("browse_sound"),
			std::bind(&custom_tod::select_file<tod_getter_sound>, this, "data/core/sounds/ambient"));

	connect_signal_mouse_left_click(
			find_widget<button>("preview_image"),
			std::bind(&custom_tod::update_image, this, "image"));

	connect_signal_mouse_left_click(
			find_widget<button>("preview_mask"),
			std::bind(&custom_tod::update_image, this, "mask"));

	connect_signal_mouse_left_click(
			find_widget<button>("preview_sound"),
			std::bind(&custom_tod::play_sound, this));

	connect_signal_mouse_left_click(
			find_widget<button>("next_tod"),
			std::bind(&custom_tod::do_next_tod, this));

	connect_signal_mouse_left_click(
			find_widget<button>("previous_tod"),
			std::bind(&custom_tod::do_prev_tod, this));

	connect_signal_mouse_left_click(
			find_widget<button>("new"),
			std::bind(&custom_tod::do_new_tod, this));

	connect_signal_mouse_left_click(
			find_widget<button>("delete"),
			std::bind(&custom_tod::do_delete_tod, this));

	connect_signal_mouse_left_click(
			find_widget<button>("preview_color"),
			std::bind(&custom_tod::preview_schedule, this));

	connect_signal_notify_modified(
			find_widget<slider>("lawful_bonus"),
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
		const std::string& message
						= _("This file is outside Wesnoth’s data dirs. Do you wish to copy it into your add-on?");

		if(data.first == "image") {
			if (!filesystem::to_asset_path(dn, addon_id_, "images")) {
				if(gui2::show_message(_("Confirm"), message, message::yes_no_buttons) == gui2::retval::OK) {
					filesystem::copy_file(dlg.path(), dn);
				}
			}
			times_[current_tod_].image = dn;
		} else if(data.first == "mask") {
			if (!filesystem::to_asset_path(dn, addon_id_, "images")) {
				if(gui2::show_message(_("Confirm"), message, message::yes_no_buttons) == gui2::retval::OK) {
					filesystem::copy_file(dlg.path(), dn);
				}
			}
			times_[current_tod_].image_mask = dn;
		} else if(data.first == "sound") {
			if (!filesystem::to_asset_path(dn, addon_id_, "sounds")) {
				if(gui2::show_message(_("Confirm"), message, message::yes_no_buttons) == gui2::retval::OK) {
					filesystem::copy_file(dlg.path(), dn);
				}
			}
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
		throw std::string("Attempted to fetch a non-existent ToD!");
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

void custom_tod::play_sound() {
	std::string sound_path = find_widget<text_box>("path_sound").get_value();
	sound::play_sound(sound_path, sound::SOUND_SOURCES);
}

void custom_tod::update_image(const std::string& id_stem) {
	std::string img_path = find_widget<text_box>("path_"+id_stem).get_value();
	find_widget<image>("current_tod_" + id_stem).set_label(img_path);

	invalidate_layout();
}

void custom_tod::update_tod_display()
{
	display* disp = display::get_singleton();
	assert(disp && "Display pointer is null!");

	// The display handles invaliding whatever tiles need invalidating.
	disp->update_tod(&get_selected_tod());

	invalidate_layout();
}

void custom_tod::update_lawful_bonus()
{
	times_[current_tod_].lawful_bonus = find_widget<slider>("lawful_bonus").get_value();
}

void custom_tod::update_selected_tod_info()
{
	const time_of_day& current_tod = get_selected_tod();

	find_widget<text_box>("tod_name").set_value(current_tod.name);
	find_widget<text_box>("tod_desc").set_value(current_tod.description);
	find_widget<text_box>("tod_id").set_value(current_tod.id);

	find_widget<text_box>("path_image").set_value(current_tod.image);
	find_widget<text_box>("path_mask").set_value(current_tod.image_mask);
	find_widget<text_box>("path_sound").set_value(current_tod.sounds);

	find_widget<image>("current_tod_image").set_image(current_tod.image);
	find_widget<image>("current_tod_mask").set_image(current_tod.image_mask);

	find_widget<slider>("lawful_bonus").set_value(current_tod.lawful_bonus);

	color_field_r_->set_widget_value(current_tod.color.r);
	color_field_g_->set_widget_value(current_tod.color.g);
	color_field_b_->set_widget_value(current_tod.color.b);

	const std::string new_index_str = formatter() << (current_tod_ + 1) << "/" << times_.size();
	find_widget<label>("tod_number").set_label(new_index_str);

	update_tod_display();
}

void custom_tod::copy_to_clipboard_callback(const std::pair<std::string, tod_attribute_getter>& data)
{
	auto& [type, getter] = data;
	button& copy_w = find_widget<button>("copy_" + type);
	desktop::clipboard::copy_to_clipboard(getter(get_selected_tod()).second);
	copy_w.set_success(true);
}

/** Quickly preview the schedule changes and color */
void custom_tod::preview_schedule()
{
	update_map_and_schedule_(times_);
}

void custom_tod::update_schedule()
{
	/* Update times_ with values from the dialog */
	times_[current_tod_].name = find_widget<text_box>("tod_name").get_value();
	times_[current_tod_].description = find_widget<text_box>("tod_desc").get_value();
	times_[current_tod_].id = find_widget<text_box>("tod_id").get_value();

	times_[current_tod_].image = find_widget<text_box>("path_image").get_value();
	times_[current_tod_].image_mask = find_widget<text_box>("path_mask").get_value();
	times_[current_tod_].sounds = find_widget<text_box>("path_sound").get_value();

	times_[current_tod_].lawful_bonus = find_widget<slider>("lawful_bonus").get_value();

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
	update_map_and_schedule_ = std::move(update_func);
}

void custom_tod::post_show()
{
	update_tod_display();
}

} // namespace dialogs
