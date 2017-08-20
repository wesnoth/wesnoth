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

#include "utils/functional.hpp"
#include <string>
#include <sstream>

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(label)

label::label(const implementation::builder_label& builder)
	: styled_widget(builder, get_control_type())
	, state_(ENABLED)
	, can_wrap_(false)
	, characters_per_line_(0)
	, link_aware_(false)
	, link_color_(color_t::from_hex_string("ffff00"))
	, can_shrink_(false)
	, text_alpha_(255)
{
	connect_signal<event::LEFT_BUTTON_CLICK>(std::bind(&label::signal_handler_left_button_click, this, _2, _3));
	connect_signal<event::RIGHT_BUTTON_CLICK>(std::bind(&label::signal_handler_right_button_click, this, _2, _3));
}

bool label::can_wrap() const
{
	return can_wrap_ || characters_per_line_ != 0;
}

unsigned label::get_characters_per_line() const
{
	return characters_per_line_;
}

bool label::get_link_aware() const
{
	return link_aware_;
}

color_t label::get_link_color() const
{
	return link_color_;
}

void label::set_text_alpha(unsigned short alpha)
{
	text_alpha_ = alpha;

	for(auto& tmp : get_canvases()) {
		tmp.set_variable("text_alpha", wfl::variant(text_alpha_));
	}

	set_is_dirty(true);
}

void label::set_active(const bool active)
{
	if(get_active() != active) {
		set_state(active ? ENABLED : DISABLED);
	}
}

bool label::get_active() const
{
	return state_ != DISABLED;
}

unsigned label::get_state() const
{
	return state_;
}

bool label::disable_click_dismiss() const
{
	return false;
}

void label::set_characters_per_line(const unsigned characters_per_line)
{
	characters_per_line_ = characters_per_line;
}

void label::set_link_aware(bool link_aware)
{
	if(link_aware == link_aware_) {
		return;
	}

	link_aware_ = link_aware;
	update_canvas();
	set_is_dirty(true);
}

void label::set_link_color(const color_t& color)
{
	if(color == link_color_) {
		return;
	}
	link_color_ = color;
	update_canvas();
	set_is_dirty(true);
}

void label::set_state(const state_t state)
{
	if(state != state_) {
		state_ = state;
		set_is_dirty(true);
	}
}

const std::string& label::get_control_type() const
{
	static const std::string type = "label";
	return type;
}

void label::signal_handler_left_button_click(const event::ui_event /* event */, bool & handled)
{
	DBG_GUI_E << "label click" << std::endl;

	if (!get_link_aware()) {
		return; // without marking event as "handled".
	}

	if (!desktop::open_object_is_supported()) {
		show_message(get_window()->video(), "", _("Opening links is not supported, contact your packager"), dialogs::message::auto_close);
		handled = true;
		return;
	}


	point mouse = get_mouse_position();

	mouse.x -= get_x();
	mouse.y -= get_y();

	std::string link = get_label_link(mouse);

	if (link.length() == 0) {
		return ; // without marking event as "handled"
	}

	DBG_GUI_E << "Clicked Link:\"" << link << "\"\n";

	const int res = show_message(get_window()->video(), _("Confirm"), _("Do you want to open this link?") + std::string("\n\n") + link, dialogs::message::yes_no_buttons);
	if(res == gui2::window::OK) {
		desktop::open_object(link);
	}

	handled = true;
}

void label::signal_handler_right_button_click(const event::ui_event /* event */, bool & handled)
{
	DBG_GUI_E << "label right click" << std::endl;

	if (!get_link_aware()) {
		return ; // without marking event as "handled".
	}

	point mouse = get_mouse_position();

	mouse.x -= get_x();
	mouse.y -= get_y();

	std::string link = get_label_link(mouse);

	if (link.length() == 0) {
		return ; // without marking event as "handled"
	}

	DBG_GUI_E << "Right Clicked Link:\"" << link << "\"\n";

	desktop::clipboard::copy_to_clipboard(link, false);

	show_message(get_window()->video(), "", _("Copied link!"), dialogs::message::auto_close);

	handled = true;
}

// }---------- DEFINITION ---------{

label_definition::label_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing label " << id << '\n';

	load_resolutions<resolution>(cfg);
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
label_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
	, link_aware(cfg["link_aware"].to_bool(false))
	, link_color(cfg["link_color"].empty() ? color_t::from_hex_string("ffff00") : color_t::from_rgba_string(cfg["link_color"].str()))
{
	// Note the order should be the same as the enum state_t is label.hpp.
	state.emplace_back(cfg.child("state_enabled"));
	state.emplace_back(cfg.child("state_disabled"));
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

builder_label::builder_label(const config& cfg)
	: builder_styled_widget(cfg)
	, wrap(cfg["wrap"].to_bool())
	, characters_per_line(cfg["characters_per_line"])
	, text_alignment(decode_text_alignment(cfg["text_alignment"]))
	, can_shrink(cfg["can_shrink"].to_bool(false))
{
}

widget* builder_label::build() const
{
	label* lbl = new label(*this);

	const auto conf = lbl->cast_config_to<label_definition>();
	assert(conf);

	lbl->set_can_wrap(wrap);
	lbl->set_characters_per_line(characters_per_line);
	lbl->set_text_alignment(text_alignment);
	lbl->set_text_alpha(ALPHA_OPAQUE);
	lbl->set_can_shrink(can_shrink);
	lbl->set_link_aware(conf->link_aware);
	lbl->set_link_color(conf->link_color);

	DBG_GUI_G << "Window builder: placed label '" << id << "' with definition '"
			  << definition << "'.\n";

	return lbl;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
