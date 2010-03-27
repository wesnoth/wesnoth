/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "gui/dialogs/editor_settings.hpp"

#include "gui/dialogs/field.hpp"
#include "gui/dialogs/helper.hpp"

#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/slider.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gettext.hpp"

#include <boost/bind.hpp>

#define ERR_ED LOG_STREAM_INDENT(err, editor)

namespace gui2 {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_editor_settings
 *
 * == Editor settings ==
 *
 * This shows the dialog to set the editor settings.
 *
 * @start_table = grid
 *     (current_tod) (label) ()   Displays the currently selected time of
 *                                day(ToD) mask.
 *     (current_tod_image) (label) ()
 *                                The image for the ToD mask.
 *     (next_tod) (button) ()     Selects the next ToD mask.
 *
 *     (custom_tod_toggle) (toggle_button) ()
 *                                Allow to set the ToD mask by selecting the
 *                                colour components manually.
 *     (custom_tod_red) (slider) ()
 *                                Sets the red component of the custom ToD
 *                                mask.
 *     (custom_tod_green) (slider) ()
 *                                Sets the green component of the custom ToD
 *                                mask.
 *     (custom_tod_blue) (slider) ()
 *                                Sets the blue component of the custom ToD
 *                                mask.
 *
 *     (custom_tod_auto_refresh) (toggle_button) ()
 *                                Directly update the ToD mask when a
 *                                component slider is moved.
 *
 *     (use_mdi) (boolean) ()     Sets whether the user interface should be
 *                                an MDI interface or not.
 * @end_table
 */

REGISTER_WINDOW(editor_settings)

teditor_settings::teditor_settings()
	: redraw_callback_()
	, tods_()
	, current_tod_(0)
	, current_tod_label_(NULL)
	, current_tod_image_(NULL)
	, custom_tod_toggle_(NULL)
	, custom_tod_auto_refresh_(NULL)
	, custom_tod_toggle_field_(register_bool("custom_tod_toggle", false))
	, custom_tod_red_(NULL)
	, custom_tod_green_(NULL)
	, custom_tod_blue_(NULL)
	, custom_tod_red_field_(register_integer("custom_tod_red", false))
	, custom_tod_green_field_(register_integer("custom_tod_green", false))
	, custom_tod_blue_field_(register_integer("custom_tod_blue", false))
	, use_mdi_field_(register_bool("use_mdi"))
{
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

int teditor_settings::get_red() const
{
	return custom_tod_red_field_->get_cache_value();
}
int teditor_settings::get_green() const
{
	return custom_tod_green_field_->get_cache_value();
}
int teditor_settings::get_blue() const
{
	return custom_tod_blue_field_->get_cache_value();
}

void teditor_settings::set_use_mdi(bool value)
{
	use_mdi_field_->set_cache_value(value);
}

bool teditor_settings::get_use_mdi() const
{
	return use_mdi_field_->get_cache_value();
}

void teditor_settings::update_tod_display(twindow& window)
{
	redraw_callback_(custom_tod_red_->get_value(),
		custom_tod_green_->get_value(),
		custom_tod_blue_->get_value());
	window.set_dirty(true);
}

void teditor_settings::slider_update_callback(twindow& window)
{
	if (custom_tod_auto_refresh_->get_value()) {
		update_tod_display(window);
	}
}

void teditor_settings::set_current_adjustment(int r, int g, int b)
{
	for (size_t i = 0; i < tods_.size(); ++i) {
		time_of_day& tod = tods_[i];
		if (tod.red == r && tod.green == g && tod.blue == b) {
			current_tod_ = i;
			custom_tod_toggle_field_->set_cache_value(false);
			return;
		}
	}
	/* custom tod */
	custom_tod_red_field_->set_cache_value(r);
	custom_tod_green_field_->set_cache_value(g);
	custom_tod_blue_field_->set_cache_value(b);
	custom_tod_toggle_field_->set_cache_value(true);
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
		custom_tod_red_->set_value(get_selected_tod().red);
		custom_tod_green_->set_value(get_selected_tod().green);
		custom_tod_blue_->set_value(get_selected_tod().blue);
		custom_tod_red_field_->set_cache_value(get_selected_tod().red);
		custom_tod_green_field_->set_cache_value(get_selected_tod().green);
		custom_tod_blue_field_->set_cache_value(get_selected_tod().blue);
	}
	custom_tod_red_->set_active(custom);
	custom_tod_green_->set_active(custom);
	custom_tod_blue_->set_active(custom);
	current_tod_label_->set_active(!custom);
	update_tod_display(window);
	window.invalidate_layout();
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
	custom_tod_red_ = find_widget<tslider>(
			&window, "custom_tod_red", false, true);
	custom_tod_green_ = find_widget<tslider>(
			&window, "custom_tod_green", false, true);
	custom_tod_blue_ = find_widget<tslider>(
			&window, "custom_tod_blue", false, true);

	tbutton& next_tod_button = find_widget<tbutton>(
			&window, "next_tod", false);
	next_tod_button.connect_signal_mouse_left_click(boost::bind(
			  &teditor_settings::do_next_tod
			, this
			, boost::ref(window)));


	tbutton& apply_button = find_widget<tbutton>(
			&window, "apply", false);
	apply_button.connect_signal_mouse_left_click(boost::bind(
			  &teditor_settings::update_tod_display
			, this
			, boost::ref(window)));

	custom_tod_toggle_->set_callback_state_change(
			dialog_callback<teditor_settings
				, &teditor_settings::update_selected_tod_info>);
	custom_tod_red_->set_callback_positioner_move(
			dialog_callback<teditor_settings
				, &teditor_settings::slider_update_callback>);
	custom_tod_green_->set_callback_positioner_move(
			dialog_callback<teditor_settings
				, &teditor_settings::slider_update_callback>);
	custom_tod_blue_->set_callback_positioner_move(
			dialog_callback<teditor_settings
				, &teditor_settings::slider_update_callback>);
	update_selected_tod_info(window);
}

} // namespace gui2

