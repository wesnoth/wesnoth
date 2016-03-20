/*
   Copyright (C) 2009 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_STACKED_WIDGET_HPP_INCLUDED
#define GUI_WIDGETS_STACKED_WIDGET_HPP_INCLUDED

#include "gui/widgets/container.hpp"

#include "gui/auxiliary/widget_definition.hpp"
#include "gui/auxiliary/window_builder.hpp"

namespace gui2
{

// ------------ WIDGET -----------{

namespace implementation
{
struct tbuilder_stacked_widget;
}

class tgenerator_;

class tstacked_widget : public tcontainer_
{
	friend struct implementation::tbuilder_stacked_widget;
	friend class tdebug_layout_graph;

public:
	tstacked_widget();

	/***** ***** ***** inherited ***** ****** *****/

	/** See @ref tcontrol::get_active. */
	virtual bool get_active() const OVERRIDE;

	/** See @ref tcontrol::get_state. */
	virtual unsigned get_state() const OVERRIDE;

	/** See @ref twidget::layout_children. */
	virtual void layout_children() OVERRIDE;

	/**
	 * Gets the current visible layer number.
	 *
	 * The current layer number will be -1 if all layers are currently visible.
	 * In this case, only the topmost (highest-numbered) layer will receive
	 * events.
	 */
	int current_layer() const { return selected_layer_; }

	/**
	 * Selects and displays a particular layer.
	 *
	 * If layer -1 is selected, all layers will be displayed but only the
	 * topmost (highest-numbered) layer will receive events.
	 */
	void select_layer(const int layer);

	/**
	 * Gets the total number of layers.
	 */
	unsigned int get_layer_count() const;

	tgrid* get_layer_grid(unsigned int i);

private:
	/**
	 * Finishes the building initialization of the widget.
	 *
	 * @param widget_builder      The builder to build the contents of the
	 *                            widget.
	 */
	void finalize(std::vector<tbuilder_grid_const_ptr> widget_builder);

	/**
	 * Contains a pointer to the generator.
	 *
	 * The pointer is not owned by this class, it's stored in the content_grid_
	 * of the tscrollbar_container super class and freed when it's grid is
	 * freed.
	 */
	tgenerator_* generator_;

	/**
	 * The number of the current selected layer.
	 */
	int selected_layer_;

	/**
	 * Helper to ensure the correct state is set when selecting a layer.
	 */
	void select_layer_internal(const unsigned int layer, const bool select) const;

	/** See @ref tcontrol::get_control_type. */
	virtual const std::string& get_control_type() const OVERRIDE;

	/** See @ref tcontainer_::set_self_active. */
	virtual void set_self_active(const bool active) OVERRIDE;
};

// }---------- DEFINITION ---------{

struct tstacked_widget_definition : public tcontrol_definition
{
	explicit tstacked_widget_definition(const config& cfg);

	struct tresolution : public tresolution_definition_
	{
		explicit tresolution(const config& cfg);

		tbuilder_grid_ptr grid;
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct tbuilder_stacked_widget : public tbuilder_control
{
	explicit tbuilder_stacked_widget(const config& cfg);

	using tbuilder_control::build;

	twidget* build() const;

	/** The builders for all layers of the stack .*/
	std::vector<tbuilder_grid_const_ptr> stack;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2

#endif
