/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/label.hpp"

#include "gui/core/log.hpp"

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

#include "desktop/clipboard.hpp"
#include "desktop/open.hpp"
#include "gettext.hpp"

#include <boost/bind.hpp>
#include <string>
#include <sstream>

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(label)

tlabel::tlabel()
		: tcontrol(COUNT)
		, state_(ENABLED)
		, can_wrap_(false)
		, characters_per_line_(0)
		, link_aware_(false)
		, link_color_("#ffff00")
{
	connect_signal<event::LEFT_BUTTON_CLICK>(boost::bind(&tlabel::signal_handler_left_button_click, this, _2, _3));
	connect_signal<event::RIGHT_BUTTON_CLICK>(boost::bind(&tlabel::signal_handler_right_button_click, this, _2, _3));
}

bool tlabel::can_wrap() const
{
	return can_wrap_ || characters_per_line_ != 0;
}

unsigned tlabel::get_characters_per_line() const
{
	return characters_per_line_;
}

bool tlabel::get_link_aware() const
{
	return link_aware_;
}

std::string tlabel::get_link_color() const
{
	return link_color_;
}

void tlabel::set_active(const bool active)
{
	if(get_active() != active) {
		set_state(active ? ENABLED : DISABLED);
	}
}

bool tlabel::get_active() const
{
	return state_ != DISABLED;
}

unsigned tlabel::get_state() const
{
	return state_;
}

bool tlabel::disable_click_dismiss() const
{
	return false;
}

void tlabel::set_characters_per_line(const unsigned characters_per_line)
{
	characters_per_line_ = characters_per_line;
}

void tlabel::set_link_aware(bool link_aware)
{
	if(link_aware == link_aware_) {
		return;
	}

	link_aware_ = link_aware;
	update_canvas();
	set_is_dirty(true);
}

void tlabel::set_link_color(const std::string & color)
{
	if(color == link_color_) {
		return;
	}
	link_color_ = color;
	update_canvas();
	set_is_dirty(true);
}

void tlabel::set_state(const tstate state)
{
	if(state != state_) {
		state_ = state;
		set_is_dirty(true);
	}
}

const std::string& tlabel::get_control_type() const
{
	static const std::string type = "label";
	return type;
}

void tlabel::load_config_extra()
{
	assert(config());

	boost::intrusive_ptr<const tlabel_definition::tresolution>
	conf = boost::dynamic_pointer_cast<const tlabel_definition::tresolution>(
			config());

	assert(conf);

	set_link_aware(conf->link_aware);
	set_link_color(conf->link_color);
}


void tlabel::signal_handler_left_button_click(const event::tevent /* event */, bool & handled)
{
	DBG_GUI_E << "label click" << std::endl;

	if (!get_link_aware()) {
		return; // without marking event as "handled".
	}

	if (!desktop::open_object_is_supported()) {
		gui2::show_message(get_window()->video(), "", _("Opening links is not supported, contact your packager"), gui2::tmessage::auto_close);
		handled = true;
		return;
	}


	tpoint mouse = get_mouse_position();

	mouse.x -= get_x();
	mouse.y -= get_y();

	std::string link = get_label_link(mouse);

	if (link.length() == 0) {
		return ; // without marking event as "handled"
	}

	DBG_GUI_E << "Clicked Link:\"" << link << "\"\n";

	const int res = gui2::show_message(get_window()->video(), _("Confirm"), _("Do you want to open this link?") + std::string("\n\n") + link, gui2::tmessage::yes_no_buttons);
	if(res == gui2::twindow::OK) {
		desktop::open_object(link);
	}

	handled = true;
}

void tlabel::signal_handler_right_button_click(const event::tevent /* event */, bool & handled)
{
	DBG_GUI_E << "label right click" << std::endl;

	if (!get_link_aware()) {
		return ; // without marking event as "handled".
	}

	tpoint mouse = get_mouse_position();

	mouse.x -= get_x();
	mouse.y -= get_y();

	std::string link = get_label_link(mouse);

	if (link.length() == 0) {
		return ; // without marking event as "handled"
	}

	DBG_GUI_E << "Right Clicked Link:\"" << link << "\"\n";

	desktop::clipboard::copy_to_clipboard(link, false);

	gui2::show_message(get_window()->video(), "", _("Copied link!"), gui2::tmessage::auto_close);

	handled = true;
}

// }---------- DEFINITION ---------{

tlabel_definition::tlabel_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing label " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_label
 *
 * == Label ==
 *
 * @macro = label_description
 *
 * Although the label itself has no event interaction it still has two states.
 * The reason is that labels are often used as visual indication of the state
 * of the widget it labels.
 *
 * Note: The above is outdated, if "link_aware" is enabled then there is interaction.
 *
 *
 * The following states exist:
 * * state_enabled, the label is enabled.
 * * state_disabled, the label is disabled.
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="label_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super="generic/widget_definition/resolution"}
 * @begin{table}{config}
 *     link_aware & f_bool & false & Whether the label is link aware. This means
 *                                     it is rendered with links highlighted,
 *                                     and responds to click events on those
 *                                     links. $
 *     link_color & string & #ffff00 & The color to render links with. This
 *                                     string will be used verbatim in pango
 *                                     markup for each link. $
 * @end{table}
 * @begin{tag}{name="state_enabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_enabled"}
 * @begin{tag}{name="state_disabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_disabled"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="label_definition"}
 * @end{parent}{name="gui/"}
 */
tlabel_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg)
	, link_aware(cfg["link_aware"].to_bool(false))
	, link_color(cfg["link_color"].str().size() > 0 ? cfg["link_color"].str() : "#ffff00")
{
	// Note the order should be the same as the enum tstate is label.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));
}

// }---------- BUILDER -----------{

/*WIKI_MACRO
 * @begin{macro}{label_description}
 *
 *        A label displays a text, the text can be wrapped but no scrollbars
 *        are provided.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_label
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="label"}{min=0}{max=-1}{super="generic/widget_instance"}
 * == Label ==
 *
 * @macro = label_description
 *
 * List with the label specific variables:
 * @begin{table}{config}
 *     wrap & bool & false &      Is wrapping enabled for the label. $
 *     characters_per_line & unsigned & 0 &
 *                                Sets the maximum number of characters per
 *                                line. The amount is an approximate since the
 *                                width of a character differs. E.g. iii is
 *                                smaller than MMM. When the value is non-zero
 *                                it also implies can_wrap is true.
 *                                When having long strings wrapping them can
 *                                increase readability, often 66 characters per
 *                                line is considered the optimum for a one
 *                                column text.
 *     text_alignment & h_align & "left" &
 *                                How is the text aligned in the label. $
 * @end{table}
 * @end{tag}{name="label"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

namespace implementation
{

tbuilder_label::tbuilder_label(const config& cfg)
	: tbuilder_control(cfg)
	, wrap(cfg["wrap"].to_bool())
	, characters_per_line(cfg["characters_per_line"])
	, text_alignment(decode_text_alignment(cfg["text_alignment"]))
{
}

twidget* tbuilder_label::build() const
{
	tlabel* label = new tlabel();

	init_control(label);

	label->set_can_wrap(wrap);
	label->set_characters_per_line(characters_per_line);
	label->set_text_alignment(text_alignment);

	DBG_GUI_G << "Window builder: placed label '" << id << "' with definition '"
			  << definition << "'.\n";

	return label;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
