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

#include "gui/widgets/toggle_panel.hpp"

#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "gui/core/log.hpp"
#include "gui/core/window_builder/helper.hpp"
#include "gettext.hpp"
#include "sound.hpp"
#include "utils/foreach.hpp"
#include "wml_exception.hpp"

#include <boost/bind.hpp>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(toggle_panel)

ttoggle_panel::ttoggle_panel()
	: tpanel(COUNT)
	, state_(ENABLED)
	, state_num_(0)
	, retval_(0)
	, callback_state_change_(0)
	, callback_mouse_left_double_click_()
{
	set_wants_mouse_left_double_click();

	connect_signal<event::MOUSE_ENTER>(boost::bind(
			&ttoggle_panel::signal_handler_mouse_enter, this, _2, _3));
	connect_signal<event::MOUSE_LEAVE>(boost::bind(
			&ttoggle_panel::signal_handler_mouse_leave, this, _2, _3));
#if 0
	connect_signal<event::LEFT_BUTTON_CLICK>(
			boost::bind(&ttoggle_panel::signal_handler_pre_left_button_click,
						this,
						_2),
			event::tdispatcher::back_pre_child);
#endif
	connect_signal<event::LEFT_BUTTON_CLICK>(boost::bind(
			&ttoggle_panel::signal_handler_left_button_click, this, _2, _3));
	connect_signal<event::LEFT_BUTTON_CLICK>(
			boost::bind(&ttoggle_panel::signal_handler_left_button_click,
						this,
						_2,
						_3),
			event::tdispatcher::back_post_child);
	connect_signal<event::LEFT_BUTTON_DOUBLE_CLICK>(
			boost::bind(&ttoggle_panel::signal_handler_left_button_double_click,
						this,
						_2,
						_3));
	connect_signal<event::LEFT_BUTTON_DOUBLE_CLICK>(
			boost::bind(&ttoggle_panel::signal_handler_left_button_double_click,
						this,
						_2,
						_3),
			event::tdispatcher::back_post_child);
}

unsigned ttoggle_panel::num_states() const
{
	std::div_t res = std::div(this->config()->state.size(), COUNT);
	assert(res.rem == 0);
	assert(res.quot > 0);
	return res.quot;
}

void ttoggle_panel::set_child_members(
		const std::map<std::string /* widget id */, string_map>& data)
{
	FOREACH(const AUTO & item, data)
	{
		tcontrol* control = dynamic_cast<tcontrol*>(find(item.first, false));
		if(control) {
			control->set_members(item.second);
		}
	}
}
twidget* ttoggle_panel::find_at(const tpoint& coordinate,
								const bool must_be_active)
{
	/**
	 * @todo since there is no mouse event nesting (or event nesting at all)
	 * we need to capture all events. This means items on the panel will
	 * never receive an event, which gives problems with for example the
	 * intended button on the addon panel. So we need to chain mouse events
	 * as well and also add a handled flag for them.
	 */

	twidget* result = tcontainer_::find_at(coordinate, must_be_active);
	return result ? result : tcontrol::find_at(coordinate, must_be_active);
}

const twidget* ttoggle_panel::find_at(const tpoint& coordinate,
									  const bool must_be_active) const
{
	const twidget* result = tcontainer_::find_at(coordinate, must_be_active);
	return result ? result : tcontrol::find_at(coordinate, must_be_active);
}

void ttoggle_panel::set_active(const bool active)
{
	if(active) {
		set_state(ENABLED);
	} else {
		set_state(DISABLED);
	}
}

bool ttoggle_panel::get_active() const
{
	return state_ != DISABLED;
}

unsigned ttoggle_panel::get_state() const
{
	return state_ + COUNT * state_num_;
}

SDL_Rect ttoggle_panel::get_client_rect() const
{
	boost::intrusive_ptr<const ttoggle_panel_definition::tresolution> conf
			= boost::dynamic_pointer_cast<const ttoggle_panel_definition::
												  tresolution>(config());
	assert(conf);

	SDL_Rect result = get_rectangle();
	result.x += conf->left_border;
	result.y += conf->top_border;
	result.w -= conf->left_border + conf->right_border;
	result.h -= conf->top_border + conf->bottom_border;

	return result;
}

tpoint ttoggle_panel::border_space() const
{
	boost::intrusive_ptr<const ttoggle_panel_definition::tresolution> conf
			= boost::dynamic_pointer_cast<const ttoggle_panel_definition::
												  tresolution>(config());
	assert(conf);

	return tpoint(conf->left_border + conf->right_border,
				  conf->top_border + conf->bottom_border);
}

void ttoggle_panel::set_value(const unsigned selected)
{
	if(selected == get_value()) {
		return;
	}
	state_num_ = selected % num_states();
	set_is_dirty(true);
}

void ttoggle_panel::set_retval(const int retval)
{
	retval_ = retval;
}

void ttoggle_panel::set_state(const tstate state)
{
	if(state == state_) {
		return;
	}

	state_ = state;
	set_is_dirty(true);

	boost::intrusive_ptr<const ttoggle_panel_definition::tresolution> conf
			= boost::dynamic_pointer_cast<const ttoggle_panel_definition::
												  tresolution>(config());
	assert(conf);
}

void ttoggle_panel::impl_draw_background(surface& frame_buffer,
										 int x_offset,
										 int y_offset)
{
	// We don't have a fore and background and need to draw depending on
	// our state, like a control. So we use the controls drawing method.
	tcontrol::impl_draw_background(frame_buffer, x_offset, y_offset);
}

void ttoggle_panel::impl_draw_foreground(surface& frame_buffer,
										 int x_offset,
										 int y_offset)
{
	// We don't have a fore and background and need to draw depending on
	// our state, like a control. So we use the controls drawing method.
	tcontrol::impl_draw_foreground(frame_buffer, x_offset, y_offset);
}

const std::string& ttoggle_panel::get_control_type() const
{
	static const std::string type = "toggle_panel";
	return type;
}

void ttoggle_panel::signal_handler_mouse_enter(const event::tevent event,
											   bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	set_state(FOCUSED);
	handled = true;
}

void ttoggle_panel::signal_handler_mouse_leave(const event::tevent event,
											   bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	set_state(ENABLED);
	handled = true;
}

void
ttoggle_panel::signal_handler_pre_left_button_click(const event::tevent event)
{
	DBG_GUI_E << get_control_type() << "[" << id() << "]: " << event << ".\n";

	set_value(1);

#if 0
	/*
	 * Disabled since it causes issues with gamestate inspector (bug #22095).
	 * It was added in b84f2ebff0b53c7e4194da315c43f62a08494c52 for the lobby,
	 * since that code is still experimental, prefer to fix a real issue caused
	 * by it.
	 *
	 * The issue is that the gui2::tlistbox::add_row code was changed to
	 * increase the content size. Before the list was shown the list was
	 * cleared. The clear operation did not reduce the size (since the widgets
	 * were not shown yet). The add operation afterwards again reserved the
	 * space causing the size of the listbox to be twice the required space.
	 *
	 * 2014.06.09 -- Mordante
	 */
	if(callback_state_change_) {
		callback_state_change_(*this);
	}
#endif
}

void ttoggle_panel::signal_handler_left_button_click(const event::tevent event,
													 bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	sound::play_UI_sound(settings::sound_toggle_panel_click);

	set_value(get_value() + 1);

	if(callback_state_change_) {
		callback_state_change_(*this);
	}
	handled = true;
}

void ttoggle_panel::signal_handler_left_button_double_click(
		const event::tevent event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	if(retval_) {
		twindow* window = get_window();
		assert(window);

		window->set_retval(retval_);
	}

	if(callback_mouse_left_double_click_) {
		callback_mouse_left_double_click_(*this);
	}
	handled = true;
}

// }---------- DEFINITION ---------{

ttoggle_panel_definition::ttoggle_panel_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing toggle panel " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_toggle_panel
 *
 * == Toggle panel ==
 *
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="toggle_panel_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * The definition of a toggle panel. A toggle panel is like a toggle button, but
 * instead of being a button it's a panel. This means it can hold multiple child
 * items.
 *
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super=generic/widget_definition/resolution}
 * The resolution for a toggle panel also contains the following keys:
 * @begin{table}{config}
 *     top_border & unsigned & 0 &     The size which isn't used for the client
 *                                   area. $
 *     bottom_border & unsigned & 0 &  The size which isn't used for the client
 *                                   area. $
 *     left_border & unsigned & 0 &    The size which isn't used for the client
 *                                   area. $
 *     right_border & unsigned & 0 &   The size which isn't used for the client
 *                                   area. $
 * @end{table}
 *
 * The following states exist:
 * * state_enabled, the panel is enabled and not selected.
 * * state_disabled, the panel is disabled and not selected.
 * * state_focused, the mouse is over the panel and not selected.
 *
 * * state_enabled_selected, the panel is enabled and selected.
 * * state_disabled_selected, the panel is disabled and selected.
 * * state_focused_selected, the mouse is over the panel and selected.
 * @begin{tag}{name="state_enabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_enabled"}
 * @begin{tag}{name="state_disabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_disabled"}
 * @begin{tag}{name="state_focused"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_focused"}
 * @begin{tag}{name="state_enabled_selected"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_enabled_selected"}
 * @begin{tag}{name="state_disabled_selected"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_disabled_selected"}
 * @begin{tag}{name="state_focused_selected"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_focused_selected"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="toggle_panel_definition"}
 * @end{parent}{name="gui/"}
 */
ttoggle_panel_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg)
	, top_border(cfg["top_border"])
	, bottom_border(cfg["bottom_border"])
	, left_border(cfg["left_border"])
	, right_border(cfg["right_border"])
{
	// Note the order should be the same as the enum tstate in toggle_panel.hpp.
	FOREACH(const AUTO& c, cfg.child_range("state"))
	{
		state.push_back(tstate_definition(c.child("enabled")));
		state.push_back(tstate_definition(c.child("disabled")));
		state.push_back(tstate_definition(c.child("focused")));
	}
}

// }---------- BUILDER -----------{

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_toggle_panel
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="toggle_panel"}{min=0}{max=-1}{super="generic/widget_instance"}
 * == Toggle panel ==
 *
 * A toggle panel is an item which can hold other items. The difference between
 * a grid and a panel is that it's possible to define how a panel looks. A grid
 * in an invisible container to just hold the items. The toggle panel is a
 * combination of the panel and a toggle button, it allows a toggle button with
 * its own grid.
 *
 * @begin{table}{config}
 *     grid & grid & &                 Defines the grid with the widgets to
 *                                     place on the panel. $
 *     return_value_id & string & "" & The return value id, see
 *                                     [[GUIToolkitWML#Button]] for more
 *                                     information. $
 *     return_value & int & 0 &        The return value, see
 *                                     [[GUIToolkitWML#Button]] for more
 *                                     information. $
 * @end{table}
 * @allow{link}{name="gui/window/resolution/grid"}
 * @end{tag}{name="toggle_panel"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

namespace implementation
{

tbuilder_toggle_panel::tbuilder_toggle_panel(const config& cfg)
	: tbuilder_control(cfg)
	, grid(NULL)
	, retval_id_(cfg["return_value_id"])
	, retval_(cfg["return_value"])
{
	const config& c = cfg.child("grid");

	VALIDATE(c, _("No grid defined."));

	grid = new tbuilder_grid(c);
}

twidget* tbuilder_toggle_panel::build() const
{
	ttoggle_panel* widget = new ttoggle_panel();

	init_control(widget);

	widget->set_retval(get_retval(retval_id_, retval_, id));

	DBG_GUI_G << "Window builder: placed toggle panel '" << id
			  << "' with definition '" << definition << "'.\n";

	widget->init_grid(grid);
	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
