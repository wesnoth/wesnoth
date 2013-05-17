/*
   Copyright (C) 2008 - 2013 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/editor_settings.hpp"

#include "editor/editor_preferences.hpp"
#include "editor/editor_display.hpp"
#include "gui/dialogs/field.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/slider.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gettext.hpp"
#include "image.hpp"

#include <boost/bind.hpp>

namespace gui2 {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_editor_settings
 *
 * == Editor settings ==
 *
 * This shows the dialog to set the editor settings.
 *
 * @begin{table}{dialog_widgets}
 *
 * current_tod & & label & m &
 *         Displays the currently selected time of day(ToD) mask. $
 *
 * current_tod_image & & label & m &
 *         The image for the ToD mask. $
 *
 * next_tod & & button & m &
 *         Selects the next ToD mask. $
 *
 * custom_tod_toggle & & toggle_button & m &
 *         Allow to set the ToD mask by selecting the color components
 *         manually. $
 *
 * custom_tod_red & & slider & m &
 *         Sets the red component of the custom ToD mask. $
 *
 * custom_tod_green & & slider & m &
 *         Sets the green component of the custom ToD mask. $
 *
 * custom_tod_blue & & slider & m &
 *         Sets the blue component of the custom ToD mask. $
 *
 * custom_tod_auto_refresh & & toggle_button & m &
 *         Directly update the ToD mask when a component slider is moved. $
 *
 * use_mdi & & boolean & m &
 *         Sets whether the user interface should be an MDI interface or not. $
 *
 * @end{table}
 */

REGISTER_DIALOG(editor_settings)

teditor_settings::teditor_settings(editor::editor_display* display
		, const std::vector<time_of_day>& tods)
	: tods_(tods)
	, current_tod_(0)
	, current_tod_label_(NULL)
	, current_tod_image_(NULL)
	, custom_tod_toggle_(NULL)
	, custom_tod_auto_refresh_(NULL)
	, custom_tod_toggle_field_(register_bool("custom_tod_toggle", true))
	, custom_tod_red_field_(register_integer("custom_tod_red"
				, true
				, &preferences::editor::tod_r
				, &preferences::editor::set_tod_r))
	, custom_tod_green_field_(register_integer("custom_tod_green"
				, true
				, &preferences::editor::tod_g
				, &preferences::editor::set_tod_g))
	, custom_tod_blue_field_(register_integer("custom_tod_blue"
				, true
				, &preferences::editor::tod_b
				, &preferences::editor::set_tod_b))
	, display_(display)
	, can_update_display_(false)
{
	register_bool("use_mdi"
			, true
			, &preferences::editor::use_mdi
			, &preferences::editor::set_use_mdi);
}

void teditor_settings::do_next_tod(twindow& window)
{
	current_tod_++;
	current_tod_ %= tods_.size();
	custom_tod_toggle_->set_value(false);
	update_selected_tod_info(window);
}

const time_of_day& teditor_settings::get_selected_tod() const
{
	assert(static_cast<size_t>(current_tod_) < tods_.size());
	return tods_[current_tod_];
}

void teditor_settings::update_tod_display(twindow& window)
{
	image::set_color_adjustment(
			  custom_tod_red_field_->get_widget_value(window)
			, custom_tod_green_field_->get_widget_value(window)
			, custom_tod_blue_field_->get_widget_value(window));

	// Prevent a floating slice of window appearing alone over the
	// theme UI sidebar after redrawing tiles and before we have a
	// chance to redraw the rest of this window.
	window.undraw();

	if(display_) {
		// NOTE: We only really want to re-render the gamemap tiles here.
		// Redrawing everything is a significantly more expensive task.
		// At this time, tiles are the only elements on which ToD tint is
		// meant to have a visible effect. This is very strongly tied to
		// the image caching mechanism.
		//
		// If this ceases to be the case in the future, you'll need to call
		// redraw_everything() instead.

		// invalidate all tiles so they are redrawn with the new ToD tint next
		display_->invalidate_all();
		// redraw tiles
		display_->draw();
	}

	// Redraw this window again.
	window.set_dirty(true);
}

void teditor_settings::slider_update_callback(twindow& window)
{
	if (custom_tod_auto_refresh_->get_value()) {
		update_tod_display(window);
	}
}

void teditor_settings::update_selected_tod_info(twindow& window)
{
	bool custom = custom_tod_toggle_->get_value();
	if (custom) {
		current_tod_label_->set_label(_("Custom setting"));
	} else {
		std::stringstream ss;
		ss << (current_tod_ + 1);
		ss << "/" << tods_.size();
		ss << ": " << get_selected_tod().name;
		current_tod_label_->set_label(ss.str());
		/**
		 * @todo Implement the showing of the ToD icon.
		 *
		 * Note at the moment the icon is a label widget, should become an
		 * image widget.
		 */
		//current_tod_image_->set_icon_name(get_selected_tod().image);
		custom_tod_red_field_->set_widget_value(
				  window
				, get_selected_tod().color.r);
		custom_tod_green_field_->set_widget_value(
				  window
				, get_selected_tod().color.g);
		custom_tod_blue_field_->set_widget_value(
				  window
				, get_selected_tod().color.b);
	}
	custom_tod_red_field_->widget()->set_active(custom);
	custom_tod_green_field_->widget()->set_active(custom);
	custom_tod_blue_field_->widget()->set_active(custom);
	current_tod_label_->set_active(!custom);

	if(can_update_display_) {
		update_tod_display(window);
		window.invalidate_layout();
	}
}

void teditor_settings::pre_show(CVideo& /*video*/, twindow& window)
{
	assert(!tods_.empty());
	current_tod_label_ = find_widget<tlabel>(
			&window, "current_tod", false, true);
	current_tod_image_ = find_widget<tlabel>(
			&window, "current_tod_image", false, true);
	custom_tod_toggle_ = find_widget<ttoggle_button>(
			&window, "custom_tod_toggle", false, true);
	custom_tod_auto_refresh_ = find_widget<ttoggle_button>(
			&window, "custom_tod_auto_refresh", false, true);

	tbutton& next_tod_button = find_widget<tbutton>(
			&window, "next_tod", false);
	connect_signal_mouse_left_click(next_tod_button, boost::bind(
			  &teditor_settings::do_next_tod
			, this
			, boost::ref(window)));


	tbutton& apply_button = find_widget<tbutton>(
			&window, "apply", false);
	connect_signal_mouse_left_click(apply_button, boost::bind(
			  &teditor_settings::update_tod_display
			, this
			, boost::ref(window)));

	custom_tod_toggle_->set_callback_state_change(
			dialog_callback<teditor_settings
				, &teditor_settings::update_selected_tod_info>);

	connect_signal_notify_modified(*(custom_tod_red_field_->widget())
			, boost::bind(
				  &teditor_settings::slider_update_callback
				, this
				, boost::ref(window)));

	connect_signal_notify_modified(*(custom_tod_green_field_->widget())
			, boost::bind(
				  &teditor_settings::slider_update_callback
				, this
				, boost::ref(window)));

	connect_signal_notify_modified(*(custom_tod_blue_field_->widget())
			, boost::bind(
				  &teditor_settings::slider_update_callback
				, this
				, boost::ref(window)));

	for (size_t i = 0; i < tods_.size(); ++i) {

		time_of_day& tod = tods_[i];
		const int r = custom_tod_red_field_->get_widget_value(window);
		const int g = custom_tod_green_field_->get_widget_value(window);
		const int b = custom_tod_blue_field_->get_widget_value(window);
		if (tod.color.r == r && tod.color.g == g && tod.color.b == b) {
			current_tod_ = i;
			custom_tod_toggle_->set_value(false);
			update_selected_tod_info(window);
			can_update_display_ = true;
			return;
		}
	}

	/* custom tod */
	custom_tod_toggle_->set_value(true);

	update_selected_tod_info(window);

	can_update_display_ = true;
}

void teditor_settings::post_show(twindow& window)
{
    update_tod_display(window);

	can_update_display_ = false;
}


} // namespace gui2

