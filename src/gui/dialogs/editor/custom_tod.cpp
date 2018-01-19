/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "gui/dialogs/helper.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/slider.hpp"
#include "gui/widgets/text_box.hpp"
#include "image.hpp"

#include "utils/functional.hpp"

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_custom_tod
 *
 * == Custom Schedules ==
 *
 * This shows the dialog to modify tod schedules.
 *
 * @begin{table}{dialog_widgets}
 *
 * current_tod_name & & text_box & m &
 *         The name of the time of day(ToD). $
 *
 * current_tod_id & & text_box & m &
 *         The id of the time of day(ToD). $
 *
 * current_tod_image & & image & m &
 *         The image for the time of day(ToD). $
 *
 * current_tod_mask & & image & m &
 *         The image mask for the time of day(ToD). $
 *
 * current_tod_sonund & & label & m &
 *         The sound for the time of day(ToD). $
 *
 * next_tod & & button & m &
 *         Selects the next ToD. $
 *
 * prev_tod & & button & m &
 *         Selects the previous ToD. $
 *
 * lawful_bonus & & slider & m &
 *         Sets the Lawful Bonus for the current ToD. $
 *
 * tod_red & & slider & m &
 *         Sets the red component of the current ToD. $
 *
 * tod_green & & slider & m &
 *         Sets the green component of the current ToD. $
 *
 * tod_blue & & slider & m &
 *         Sets the blue component of the current ToD. $
 *
 * @end{table}
 */

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
	assert(!times_.empty());
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
			std::bind(&custom_tod::select_file<tod_getter_image>, this, std::ref(window), "data/core/images/misc"));

	connect_signal_mouse_left_click(
			find_widget<button>(&window, "browse_mask", false),
			std::bind(&custom_tod::select_file<tod_getter_mask>,  this, std::ref(window), "data/core/images"));

	connect_signal_mouse_left_click(
			find_widget<button>(&window, "browse_sound", false),
			std::bind(&custom_tod::select_file<tod_getter_sound>, this, std::ref(window), "data/core/sounds/ambient"));

	connect_signal_mouse_left_click(
			find_widget<button>(&window, "next_tod", false),
			std::bind(&custom_tod::do_next_tod, this, std::ref(window)));

	connect_signal_mouse_left_click(
			find_widget<button>(&window, "previous_tod", false),
			std::bind(&custom_tod::do_prev_tod, this, std::ref(window)));

	connect_signal_mouse_left_click(
			find_widget<button>(&window, "new", false),
			std::bind(&custom_tod::do_new_tod, this, std::ref(window)));

	connect_signal_mouse_left_click(
			find_widget<button>(&window, "delete", false),
			std::bind(&custom_tod::do_delete_tod, this, std::ref(window)));

	connect_signal_notify_modified(
			find_widget<slider>(&window, "lawful_bonus", false),
			std::bind(&custom_tod::update_lawful_bonus, this, std::ref(window)));

	connect_signal_notify_modified(
			*(color_field_r_->get_widget()),
			std::bind(&custom_tod::color_slider_callback, this, std::ref(window)));

	connect_signal_notify_modified(
			*(color_field_g_->get_widget()),
			std::bind(&custom_tod::color_slider_callback, this, std::ref(window)));

	connect_signal_notify_modified(
			*(color_field_b_->get_widget()),
			std::bind(&custom_tod::color_slider_callback, this, std::ref(window)));

	update_selected_tod_info(window);
}

template<custom_tod::string_pair(*fptr)(const time_of_day&)>
void custom_tod::select_file(window& window, const std::string& default_dir)
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

	update_selected_tod_info(window);
}

void custom_tod::do_next_tod(window& window)
{
	current_tod_ = (current_tod_ + 1) % times_.size();
	update_selected_tod_info(window);
}

void custom_tod::do_prev_tod(window& window)
{
	current_tod_ = (current_tod_ ? current_tod_ : times_.size()) - 1;
	update_selected_tod_info(window);
}

void custom_tod::do_new_tod(window& window)
{
	times_.insert(times_.begin() + current_tod_, time_of_day());
	update_selected_tod_info(window);
}

void custom_tod::do_delete_tod(window& window)
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

	update_selected_tod_info(window);
}

const time_of_day& custom_tod::get_selected_tod() const
{
	try {
		return times_.at(current_tod_);
	} catch(std::out_of_range&) {
		throw std::string("Attempted to fetch a non-existant ToD!");
	}
}

void custom_tod::color_slider_callback(window& window)
{
	time_of_day& current_tod = times_[current_tod_];

	current_tod.color.r = color_field_r_->get_widget_value(window);
	current_tod.color.g = color_field_g_->get_widget_value(window);
	current_tod.color.b = color_field_b_->get_widget_value(window);

	update_tod_display(window);
}

void custom_tod::update_tod_display(window& window)
{
	display* disp = display::get_singleton();
	assert(disp && "Display pointer is null!");

	// Prevent a floating slice of window appearing alone over the
	// theme UI sidebar after redrawing tiles and before we have a
	// chance to redraw the rest of this window.
	window.undraw();

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
	window.set_is_dirty(true);
}

void custom_tod::update_lawful_bonus(window& window)
{
	times_[current_tod_].lawful_bonus = find_widget<slider>(&window, "lawful_bonus", false).get_value();
}

void custom_tod::update_selected_tod_info(window& window)
{
	const time_of_day& current_tod = get_selected_tod();

	find_widget<text_box>(&window, "tod_name", false).set_value(current_tod.name);
	find_widget<text_box>(&window, "tod_id", false).set_value(current_tod.id);

	find_widget<text_box>(&window, "path_image", false).set_value(current_tod.image);
	find_widget<text_box>(&window, "path_mask", false).set_value(current_tod.image_mask);
	find_widget<text_box>(&window, "path_sound", false).set_value(current_tod.sounds);

	find_widget<image>(&window, "current_tod_image", false).set_image(current_tod.image);
	find_widget<image>(&window, "current_tod_mask", false).set_image(current_tod.image_mask);

	find_widget<slider>(&window, "lawful_bonus", false).set_value(current_tod.lawful_bonus);

	color_field_r_->set_widget_value(window, current_tod.color.r);
	color_field_g_->set_widget_value(window, current_tod.color.g);
	color_field_b_->set_widget_value(window, current_tod.color.b);

	const std::string new_index_str = formatter() << (current_tod_ + 1) << "/" << times_.size();
	find_widget<label>(&window, "tod_number", false).set_label(new_index_str);

	update_tod_display(window);
}

void custom_tod::copy_to_clipboard_callback(tod_attribute_getter getter)
{
	desktop::clipboard::copy_to_clipboard(getter(get_selected_tod()).second, false);
}

void custom_tod::post_show(window& window)
{
	update_tod_display(window);

	if(get_retval() == window::OK) {
		// TODO: save ToD
	}
}

} // namespace dialogs
} // namespace gui2
