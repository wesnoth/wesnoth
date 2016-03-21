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

#include "gui/widgets/text_box.hpp"

#include "font.hpp"
#include "gui/core/log.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "game_preferences.hpp"
#include "utils/foreach.hpp"
#include "serialization/unicode.hpp"
#include <boost/bind.hpp>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(text_box)

ttext_history ttext_history::get_history(const std::string& id,
										 const bool enabled)
{
	std::vector<std::string>* vec = preferences::get_history(id);
	return ttext_history(vec, enabled);
}

void ttext_history::push(const std::string& text)
{
	if(!enabled_) {
		return;
	} else {
		if(!text.empty() && (history_->empty() || text != history_->back())) {
			history_->push_back(text);
		}

		pos_ = history_->size();
	}
}

std::string ttext_history::up(const std::string& text)
{

	if(!enabled_) {
		return "";
	} else if(pos_ == history_->size()) {
		unsigned curr = pos_;
		push(text);
		pos_ = curr;
	}

	if(pos_ != 0) {
		--pos_;
	}

	return get_value();
}

std::string ttext_history::down(const std::string& text)
{
	if(!enabled_) {
		return "";
	} else if(pos_ == history_->size()) {
		push(text);
	} else {
		pos_++;
	}

	return get_value();
}

std::string ttext_history::get_value() const
{
	if(!enabled_ || pos_ == history_->size()) {
		return "";
	} else {
		return history_->at(pos_);
	}
}

ttext_box::ttext_box()
	: ttext_()
	, history_()
	, max_input_length_(0)
	, text_x_offset_(0)
	, text_y_offset_(0)
	, text_height_(0)
	, dragging_(false)
{
	set_wants_mouse_left_double_click();

	connect_signal<event::MOUSE_MOTION>(boost::bind(
			&ttext_box::signal_handler_mouse_motion, this, _2, _3, _5));
	connect_signal<event::LEFT_BUTTON_DOWN>(boost::bind(
			&ttext_box::signal_handler_left_button_down, this, _2, _3));
	connect_signal<event::LEFT_BUTTON_UP>(boost::bind(
			&ttext_box::signal_handler_left_button_up, this, _2, _3));
	connect_signal<event::LEFT_BUTTON_DOUBLE_CLICK>(boost::bind(
			&ttext_box::signal_handler_left_button_double_click, this, _2, _3));
}

void ttext_box::place(const tpoint& origin, const tpoint& size)
{
	// Inherited.
	tcontrol::place(origin, size);

	set_maximum_width(get_text_maximum_width());
	set_maximum_height(get_text_maximum_height(), false);

	set_maximum_length(max_input_length_);

	update_offsets();
}

void ttext_box::update_canvas()
{
	/***** Gather the info *****/

	// Set the cursor info.
	const unsigned start = get_selection_start();
	const int length = get_selection_length();

	set_maximum_length(max_input_length_);

	PangoEllipsizeMode ellipse_mode = PANGO_ELLIPSIZE_NONE;
	if(!can_wrap()) {
		if((start + length) > (get_length() / 2)) {
			ellipse_mode = PANGO_ELLIPSIZE_START;
		} else {
			ellipse_mode = PANGO_ELLIPSIZE_END;
		}
	}
	set_ellipse_mode(ellipse_mode);

	// Set the selection info
	unsigned start_offset = 0;
	unsigned end_offset = 0;
	if(length == 0) {
		// No nothing.
	} else if(length > 0) {
		start_offset = get_cursor_position(start).x;
		end_offset = get_cursor_position(start + length).x;
	} else {
		start_offset = get_cursor_position(start + length).x;
		end_offset = get_cursor_position(start).x;
	}

	/***** Set in all canvases *****/

	const int max_width = get_text_maximum_width();
	const int max_height = get_text_maximum_height();

	FOREACH(AUTO & tmp, canvas())
	{

		tmp.set_variable("text", variant(get_value()));
		tmp.set_variable("text_x_offset", variant(text_x_offset_));
		tmp.set_variable("text_y_offset", variant(text_y_offset_));
		tmp.set_variable("text_maximum_width", variant(max_width));
		tmp.set_variable("text_maximum_height", variant(max_height));

		tmp.set_variable("cursor_offset",
						 variant(get_cursor_position(start + length).x));

		tmp.set_variable("selection_offset", variant(start_offset));
		tmp.set_variable("selection_width", variant(end_offset - start_offset));
		tmp.set_variable("text_wrap_mode", variant(ellipse_mode));
	}
}

void ttext_box::delete_char(const bool before_cursor)
{
	if(before_cursor) {
		set_cursor(get_selection_start() - 1, false);
	}

	set_selection_length(1);

	delete_selection();
}

void ttext_box::delete_selection()
{
	if(get_selection_length() == 0) {
		return;
	}

	// If we have a negative range change it to a positive range.
	// This makes the rest of the algoritms easier.
	int len = get_selection_length();
	unsigned start = get_selection_start();
	if(len < 0) {
		len = -len;
		start -= len;
	}

	utf8::string tmp = get_value();
	set_value(utf8::erase(tmp, start, len));
	set_cursor(start, false);
}

void ttext_box::handle_mouse_selection(tpoint mouse, const bool start_selection)
{
	mouse.x -= get_x();
	mouse.y -= get_y();
	// FIXME we don't test for overflow in width
	if(mouse.x < static_cast<int>(text_x_offset_)
	   || mouse.y < static_cast<int>(text_y_offset_)
	   || mouse.y >= static_cast<int>(text_y_offset_ + text_height_)) {
		return;
	}

	int offset = get_column_line(tpoint(mouse.x - text_x_offset_,
										mouse.y - text_y_offset_)).x;

	if(offset < 0) {
		return;
	}


	set_cursor(offset, !start_selection);
	update_canvas();
	set_is_dirty(true);
	dragging_ |= start_selection;
}

void ttext_box::update_offsets()
{
	assert(config());

	boost::intrusive_ptr<const ttext_box_definition::tresolution>
	conf = boost::dynamic_pointer_cast<const ttext_box_definition::tresolution>(
			config());

	assert(conf);

	text_height_ = font::get_max_height(conf->text_font_size);

	game_logic::map_formula_callable variables;
	variables.add("height", variant(get_height()));
	variables.add("width", variant(get_width()));
	variables.add("text_font_height", variant(text_height_));

	text_x_offset_ = conf->text_x_offset(variables);
	text_y_offset_ = conf->text_y_offset(variables);

	// Since this variable doesn't change set it here instead of in
	// update_canvas().
	FOREACH(AUTO & tmp, canvas())
	{
		tmp.set_variable("text_font_height", variant(text_height_));
	}

	// Force an update of the canvas since now text_font_height is known.
	update_canvas();
}

bool ttext_box::history_up()
{
	if(!history_.get_enabled()) {
		return false;
	}

	const std::string str = history_.up(get_value());
	if(!str.empty()) {
		set_value(str);
	}
	return true;
}

bool ttext_box::history_down()
{
	if(!history_.get_enabled()) {
		return false;
	}

	const std::string str = history_.down(get_value());
	if(!str.empty()) {
		set_value(str);
	}
	return true;
}

void ttext_box::handle_key_default(bool& handled,
								   SDLKey key,
								   SDLMod modifier,
								   const utf8::string& unicode)
{
	if(key == SDLK_TAB && (modifier & KMOD_CTRL)) {
		if(!(modifier & KMOD_SHIFT)) {
			handled = history_up();
		} else {
			handled = history_down();
		}
	}

	if(!handled) {
		// Inherited.
		ttext_::handle_key_default(handled, key, modifier, unicode);
	}
}

void ttext_box::handle_key_clear_line(SDLMod /*modifier*/, bool& handled)
{
	handled = true;

	set_value("");
}

void ttext_box::load_config_extra()
{
	assert(config());

	boost::intrusive_ptr<const ttext_box_definition::tresolution>
	conf = boost::dynamic_pointer_cast<const ttext_box_definition::tresolution>(
			config());

	assert(conf);

	set_font_size(conf->text_font_size);
	set_font_style(conf->text_font_style);

	update_offsets();
}

const std::string& ttext_box::get_control_type() const
{
	static const std::string type = "text_box";
	return type;
}

void ttext_box::signal_handler_mouse_motion(const event::tevent event,
											bool& handled,
											const tpoint& coordinate)
{
	DBG_GUI_E << get_control_type() << "[" << id() << "]: " << event << ".\n";

	if(dragging_) {
		handle_mouse_selection(coordinate, false);
	}

	handled = true;
}

void ttext_box::signal_handler_left_button_down(const event::tevent event,
												bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	/*
	 * Copied from the base class see how we can do inheritance with the new
	 * system...
	 */
	get_window()->keyboard_capture(this);
	get_window()->mouse_capture();

	handle_mouse_selection(get_mouse_position(), true);

	handled = true;
}

void ttext_box::signal_handler_left_button_up(const event::tevent event,
											  bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	dragging_ = false;
	handled = true;
}

void
ttext_box::signal_handler_left_button_double_click(const event::tevent event,
												   bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	select_all();
	handled = true;
}

// }---------- DEFINITION ---------{

ttext_box_definition::ttext_box_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing text_box " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_text_box
 *
 * == Text box ==
 *
 * The definition of a text box.
 *
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="text_box_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * The resolution for a text box also contains the following keys:
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super=generic/widget_definition/resolution}
 * @begin{table}{config}
 *     text_x_offset & f_unsigned & "" & The x offset of the text in the text
 *                                     box. This is needed for the code to
 *                                     determine where in the text the mouse
 *                                     clicks, so it can set the cursor
 *                                     properly. $
 *     text_y_offset & f_unsigned & "" & The y offset of the text in the text
 *                                     box. $
 * @end{table}
 *
 * The following states exist:
 * * state_enabled, the text box is enabled.
 * * state_disabled, the text box is disabled.
 * * state_focused, the text box has the focus of the keyboard.
 * @begin{tag}{name="state_enabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_enabled"}
 * @begin{tag}{name="state_disabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_disabled"}
 * @begin{tag}{name="state_focused"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_focused"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="text_box_definition"}
 * @end{parent}{name="gui/"}
 */
ttext_box_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg)
	, text_x_offset(cfg["text_x_offset"])
	, text_y_offset(cfg["text_y_offset"])
{
	// Note the order should be the same as the enum tstate in text_box.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));
	state.push_back(tstate_definition(cfg.child("state_focused")));
}

// }---------- BUILDER -----------{

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_text_box
 *
 * == Text box ==
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="text_box"}{min="0"}{max="-1"}{super="generic/widget_instance"}
 * @begin{table}{config}
 *     label & t_string & "" &          The initial text of the text box. $
 *     history & string & "" &         The name of the history for the text
 *                                     box.
 *                                     A history saves the data entered in a
 *                                     text box between the games. With the up
 *                                     and down arrow it can be accessed. To
 *                                     create a new history item just add a
 *                                     new unique name for this field and the
 *                                     engine will handle the rest. $
 * @end{table}
 * @end{tag}{name="text_box"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

namespace implementation
{

tbuilder_text_box::tbuilder_text_box(const config& cfg)
	: tbuilder_control(cfg)
	, history(cfg["history"])
	, max_input_length(cfg["max_input_length"])
{
}

twidget* tbuilder_text_box::build() const
{
	ttext_box* widget = new ttext_box();

	init_control(widget);

	// A textbox doesn't have a label but a text
	widget->set_value(label);

	if(!history.empty()) {
		widget->set_history(history);
	}

	widget->set_max_input_length(max_input_length);

	DBG_GUI_G << "Window builder: placed text box '" << id
			  << "' with definition '" << definition << "'.\n";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
