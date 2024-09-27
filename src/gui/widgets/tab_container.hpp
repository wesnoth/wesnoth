/*
	Copyright (C) 2024
	by Subhraman Sarkar (babaissarkar) <suvrax@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "gui/widgets/container_base.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/core/window_builder.hpp"

namespace gui2
{

namespace implementation
{
struct builder_tab_container;
}

/**
 * A container widget that shows one of its pages of widgets
 * depending on which tab the user clicked.
 */
class tab_container : public container_base
{
	friend struct implementation::builder_tab_container;

public:
	explicit tab_container(const implementation::builder_tab_container& builder);

	virtual void set_self_active(const bool active) override;

	/* **** ***** ***** setters / getters for members ***** ****** **** */

	virtual bool get_active() const override;

	virtual unsigned get_state() const override;

	bool can_wrap() const override;

	void select_tab(unsigned index);

	unsigned get_active_tab_index() {
		return get_internal_list().get_selected_row();
	}

	unsigned get_tab_count() const {
		return builders_.size();
	}

	grid* get_tab_grid(unsigned i)
	{
		assert(generator_);
		return &generator_->item(i);
	}

	const grid* get_tab_grid(unsigned i) const
	{
		assert(generator_);
		return &generator_->item(i);
	}
private:
	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 */
	enum state_t {
		ENABLED,
		DISABLED,
	};

	/**
	 * Current state of the widget.
	 *
	 * The state of the widget determines what to render and how the widget
	 * reacts to certain 'events'.
	 */
	state_t state_;

	std::vector<std::shared_ptr<builder_grid>> builders_;
	std::vector<widget_data> list_items_;

	/**
	 * Finishes the building initialization of the widget.
	 *
	 * @param generator           Generator for the list
	 */
	void finalize(std::unique_ptr<generator_base> generator);

	/**
	 * Contains a pointer to the generator.
	 */
	generator_base* generator_;

	/** Get the listbox inside which the tabs are shown */
	listbox& get_internal_list();

	void add_tab_entry(const widget_data& row);

	void change_selection();

	void finalize_listbox();

public:
	/** Static type getter that does not rely on the widget being constructed. */
	static const std::string& type();

private:
	/* **** ***** ***** inherited ****** **** */

	/** Inherited from styled_widget, implemented by REGISTER_WIDGET. */
	virtual const std::string& get_control_type() const override;

};

// }---------- DEFINITION ---------{

struct tab_container_definition : public styled_widget_definition
{
	explicit tab_container_definition(const config& cfg);

	struct resolution : public resolution_definition
	{
		explicit resolution(const config& cfg);

		builder_grid_ptr grid;
	};
};

// }---------- BUILDER -----------{

namespace implementation
{

struct builder_tab_container : public builder_styled_widget
{
	explicit builder_tab_container(const config& cfg);

	using builder_styled_widget::build;

	virtual std::unique_ptr<widget> build() const override;

	std::vector<std::shared_ptr<builder_grid>> builders;

	std::vector<widget_data> list_items;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2
