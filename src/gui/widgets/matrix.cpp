/*
   Copyright (C) 2012 - 2016 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/matrix.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/log.hpp"
#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"
#include "gui/core/window_builder/helper.hpp"
#include "gui/widgets/detail/register.hpp"
#include "gui/widgets/settings.hpp"

#include <boost/bind.hpp>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(matrix)


tstate_default::tstate_default() : state_(ENABLED)
{
}
void tstate_default::set_active(const bool active)
{
	if(get_active() != active) {
		state_ = active ? ENABLED : DISABLED;
	}
}

bool tstate_default::get_active() const
{
	return state_ != DISABLED;
}

unsigned tstate_default::get_state() const
{
	return state_;
}

tmatrix::tmatrix(const implementation::tbuilder_matrix& builder)
	: tbase(builder, get_control_type()), content_(), pane_(NULL)
{
	boost::intrusive_ptr<const tmatrix_definition::tresolution>
	cfg = boost::dynamic_pointer_cast<const tmatrix_definition::tresolution>(
			config());

	tbuilder_widget::treplacements replacements;
	replacements.insert(std::make_pair("_main", builder.builder_main));

	if(builder.builder_top) {
		replacements.insert(std::make_pair("_top", builder.builder_top));
	}

	if(builder.builder_left) {
		replacements.insert(std::make_pair("_left", builder.builder_left));
	}

	if(builder.builder_right) {
		replacements.insert(std::make_pair("_right", builder.builder_right));
	}

	if(builder.builder_bottom) {
		replacements.insert(std::make_pair("_bottom", builder.builder_bottom));
	}

	cfg->content->build(content_, replacements);
	content_.set_parent(this);

	pane_ = find_widget<tpane>(&content_, "pane", false, true);
}

tmatrix* tmatrix::build(const implementation::tbuilder_matrix& builder)
{
	return new tmatrix(builder);
}

unsigned
tmatrix::create_item(const std::map<std::string, string_map>& item_data,
					 const std::map<std::string, std::string>& tags)
{
	return pane_->create_item(item_data, tags);
}

void tmatrix::place(const tpoint& origin, const tpoint& size)
{
	twidget::place(origin, size);

	content_.place(origin, size);
}

void tmatrix::layout_initialise(const bool full_initialisation)
{
	content_.layout_initialise(full_initialisation);
}

void
tmatrix::impl_draw_children(surface& frame_buffer, int x_offset, int y_offset)
{
	content_.draw_children(frame_buffer, x_offset, y_offset);
}

void tmatrix::layout_children()
{
	content_.layout_children();
}

void tmatrix::child_populate_dirty_list(twindow& caller,
										const std::vector<twidget*>& call_stack)
{
	std::vector<twidget*> child_call_stack = call_stack;
	content_.populate_dirty_list(caller, child_call_stack);
}

void tmatrix::request_reduce_width(const unsigned /*maximum_width*/)
{
}

twidget* tmatrix::find_at(const tpoint& coordinate, const bool must_be_active)
{
	return content_.find_at(coordinate, must_be_active);
}

const twidget* tmatrix::find_at(const tpoint& coordinate,
								const bool must_be_active) const
{
	return content_.find_at(coordinate, must_be_active);
}

twidget* tmatrix::find(const std::string& id, const bool must_be_active)
{
	if(twidget* result = twidget::find(id, must_be_active)) {
		return result;
	} else {
		return content_.find(id, must_be_active);
	}
}

const twidget* tmatrix::find(const std::string& id, const bool must_be_active)
		const
{
	if(const twidget* result = twidget::find(id, must_be_active)) {
		return result;
	} else {
		return content_.find(id, must_be_active);
	}
}

tpoint tmatrix::calculate_best_size() const
{
	tpoint size = content_.get_best_size();

	return size;
}

bool tmatrix::disable_click_dismiss() const
{
	return false;
}

iterator::twalker_* tmatrix::create_walker()
{
	/**
	 * @todo Implement properly.
	 */
	return NULL;
}

const std::string& tmatrix::get_control_type() const
{
	static const std::string type = "matrix";
	return type;
}

// }---------- DEFINITION ---------{

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_matrix
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="matrix_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * == Listbox ==
 *
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super=generic/widget_definition/resolution}
 *
 *
 * @begin{tag}{name="state_enabled"}{min=1}{max=1}{super="generic/state"}
 * @end{tag}{name="state_enabled"}
 * @begin{tag}{name="state_disabled"}{min=1}{max=1}{super="generic/state"}
 * @end{tag}{name="state_disabled"}
 * @begin{tag}{name="content"}{min=1}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="content"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="matrix_definition"}
 * @end{parent}{name="gui/"}
 */

tmatrix_definition::tmatrix_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing matrix " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

tmatrix_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg)
	, content(new tbuilder_grid(cfg.child("content", "[matrix_definition]")))
{
	// Note the order should be the same as the enum tstate in matrix.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));
}

// }---------- BUILDER -----------{

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_matrix
 *
 * == Listbox ==
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="matrix"}{min=0}{max=-1}{super="generic/widget_instance"}
 *
 *
 * List with the matrix specific variables:
 * @begin{table}{config}
 *     vertical_scrollbar_mode & scrollbar_mode & initial_auto &
 *                                     Determines whether or not to show the
 *                                     scrollbar. $
 *     horizontal_scrollbar_mode & scrollbar_mode & initial_auto &
 *                                     Determines whether or not to show the
 *                                     scrollbar. $
 * @end{table}
 *
 *
 * @begin{tag}{name="top"}{min=0}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="top"}
 * @begin{tag}{name="bottom"}{min=0}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="bottom"}
 *
 * @begin{tag}{name="left"}{min=0}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="left"}
 * @begin{tag}{name="right"}{min=0}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="right"}
 *
 * @begin{tag}{name="main"}{min="1"}{max="1"}{super="gui/window/resolution/grid/row/column"}
 * @end{tag}{name="main"}
 * @end{tag}{name="matrix"}
 *
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

namespace implementation
{

tbuilder_matrix::tbuilder_matrix(const config& cfg)
	: tbuilder_control(cfg)
	, vertical_scrollbar_mode(
			  get_scrollbar_mode(cfg["vertical_scrollbar_mode"]))
	, horizontal_scrollbar_mode(
			  get_scrollbar_mode(cfg["horizontal_scrollbar_mode"]))
	, builder_top(NULL)
	, builder_bottom(NULL)
	, builder_left(NULL)
	, builder_right(NULL)
	, builder_main(create_builder_widget(cfg.child("main", "[matrix]")))
{
	if(const config& top = cfg.child("top")) {
		builder_top = new tbuilder_grid(top);
	}

	if(const config& bottom = cfg.child("bottom")) {
		builder_bottom = new tbuilder_grid(bottom);
	}

	if(const config& left = cfg.child("left")) {
		builder_left = new tbuilder_grid(left);
	}

	if(const config& right = cfg.child("right")) {
		builder_right = new tbuilder_grid(right);
	}
}

twidget* tbuilder_matrix::build() const
{
	return tmatrix::build(*this);
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
