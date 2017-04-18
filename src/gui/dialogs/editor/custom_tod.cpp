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

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "gui/dialogs/editor/custom_tod.hpp"

#include "filesystem.hpp"
#include "editor/editor_preferences.hpp"
#include "editor/editor_display.hpp"
#include "gui/auxiliary/field.hpp"
#include "gui/dialogs/file_dialog.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/slider.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/text_box.hpp"
#include "gettext.hpp"
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

REGISTER_DIALOG(custom_tod)

custom_tod::custom_tod(display& display,
						 const std::vector<time_of_day>& tods)
	: tods_(tods)
	, current_tod_(0)
	, current_tod_name_(nullptr)
	, current_tod_id_(nullptr)
	, current_tod_image_(nullptr)
	, current_tod_mask_(nullptr)
	, current_tod_sound_(nullptr)
	, current_tod_number_(nullptr)
	, lawful_bonus_field_(register_integer("lawful_bonus", true))
	, tod_red_field_(nullptr)
	, tod_green_field_(nullptr)
	, tod_blue_field_(nullptr)
	, display_(display)
{
}

void custom_tod::select_file(const std::string& filename,
							  const std::string& default_dir,
							  const std::string& attribute,
							  window& window)
{
	std::string fn = filesystem::base_name(filename);
	std::string dn = filesystem::directory_name(fn);
	if(dn.empty()) {
		dn = default_dir;
	}

	gui2::dialogs::file_dialog dlg;

	dlg.set_title(_("Choose File"))
	   .set_ok_label(_("Select"))
	   .set_path(dn)
	   .set_read_only(true);

	if(dlg.show(display_.video())) {
		dn = dlg.path();

		if(attribute == "image") {
			tods_[current_tod_].image = dn;
		} else if(attribute == "mask") {
			tods_[current_tod_].image_mask = dn;
		} else if(attribute == "sound") {
			tods_[current_tod_].sounds = dn;
		}
	}
	update_selected_tod_info(window);
}

void custom_tod::do_next_tod(window& window)
{
	current_tod_ = (current_tod_ + 1) % tods_.size();
	update_selected_tod_info(window);
}

void custom_tod::do_prev_tod(window& window)
{
	current_tod_ = (current_tod_ ? current_tod_ : tods_.size()) - 1;
	update_selected_tod_info(window);
}

void custom_tod::do_new_tod(window& window)
{
	tods_.insert(tods_.begin() + current_tod_, time_of_day());
	update_selected_tod_info(window);
}

void custom_tod::do_delete_tod(window& window)
{
	assert(tods_.begin() + current_tod_ < tods_.end());
	if(tods_.size() == 1) {
		tods_.at(0) = time_of_day();
		update_selected_tod_info(window);
		return;
	}
	tods_.erase(tods_.begin() + current_tod_);
	if(tods_.begin() + current_tod_ >= tods_.end()) {
		current_tod_ = tods_.size() - 1;
	}
	update_selected_tod_info(window);
}

const time_of_day& custom_tod::get_selected_tod() const
{
	assert(static_cast<size_t>(current_tod_) < tods_.size());
	return tods_[current_tod_];
}

void custom_tod::update_tod_display(window& window)
{
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

	display_.adjust_color_overlay(
		tod_red_field_->get_value(),
		tod_green_field_->get_value(),
		tod_blue_field_->get_value()
	);

	// invalidate all tiles so they are redrawn with the new ToD tint next
	display_.invalidate_all();

	// redraw tiles
	display_.draw(false);

	window.invalidate_layout();
}

void custom_tod::update_lawful_bonus(window& window)
{
	tods_[current_tod_].lawful_bonus
			= lawful_bonus_field_->get_widget_value(window);
}

void custom_tod::update_selected_tod_info(window& window)
{
	current_tod_name_->set_value(get_selected_tod().name);
	current_tod_id_->set_value(get_selected_tod().id);
	current_tod_image_->set_image(get_selected_tod().image);
	current_tod_mask_->set_image(get_selected_tod().image_mask);
	current_tod_sound_->set_label(get_selected_tod().sounds);

	std::stringstream ss;
	ss << (current_tod_ + 1) << "/" << tods_.size();
	current_tod_number_->set_label(ss.str());

	lawful_bonus_field_->set_widget_value(window,
										  get_selected_tod().lawful_bonus);
	tod_red_field_->set_value(get_selected_tod().color.r);
	tod_green_field_->set_value(get_selected_tod().color.g);
	tod_blue_field_->set_value(get_selected_tod().color.b);

	update_tod_display(window);
}

void custom_tod::pre_show(window& window)
{
	assert(!tods_.empty());

	tod_red_field_
			= find_widget<slider>(&window, "tod_red", false, true);

	tod_green_field_
			= find_widget<slider>(&window, "tod_green", false, true);

	tod_blue_field_
			= find_widget<slider>(&window, "tod_blue", false, true);

	current_tod_name_
			= find_widget<text_box>(&window, "tod_name", false, true);

	current_tod_id_ = find_widget<text_box>(&window, "tod_id", false, true);

	current_tod_image_
			= find_widget<image>(&window, "current_tod_image", false, true);

	current_tod_mask_
			= find_widget<image>(&window, "current_tod_mask", false, true);

	current_tod_sound_
			= find_widget<label>(&window, "current_sound", false, true);

	current_tod_number_
			= find_widget<label>(&window, "tod_number", false, true);

	connect_signal_mouse_left_click(find_widget<button>(&window, "image_button", false),
			std::bind(&custom_tod::select_file,
					this,
					get_selected_tod().image,
					"data/core/images/misc",
					"image",
					std::ref(window)));

	connect_signal_mouse_left_click(find_widget<button>(&window, "mask_button", false),
			std::bind(&custom_tod::select_file,
					this,
					get_selected_tod().image_mask,
					"data/core/images",
					"mask",
					std::ref(window)));

	connect_signal_mouse_left_click(find_widget<button>(&window, "sound_button", false),
			std::bind(&custom_tod::select_file,
					this,
					get_selected_tod().sounds,
					"data/core/sounds/ambient",
					"sound",
					std::ref(window)));

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
			*(lawful_bonus_field_->get_widget()),
			std::bind(&custom_tod::update_lawful_bonus,
					this,
					std::ref(window)));

	connect_signal_notify_modified(
			*tod_red_field_,
			std::bind(&custom_tod::update_tod_display,
					this,
					std::ref(window)));

	connect_signal_notify_modified(
			*tod_green_field_,
			std::bind(&custom_tod::update_tod_display,
					this,
					std::ref(window)));

	connect_signal_notify_modified(
			*tod_blue_field_,
			std::bind(&custom_tod::update_tod_display,
					this,
					std::ref(window)));

	for(size_t i = 0; i < tods_.size(); ++i) {
		time_of_day& tod = tods_[i];
		const int r = tod_red_field_->get_value();
		const int g = tod_green_field_->get_value();
		const int b = tod_blue_field_->get_value();
		if(tod.color.r == r && tod.color.g == g && tod.color.b == b) {
			current_tod_ = i;
			update_selected_tod_info(window);
			return;
		}
	}

	update_selected_tod_info(window);
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
