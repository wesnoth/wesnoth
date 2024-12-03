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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/tab_container.hpp"

#include "gui/core/log.hpp"
#include "gettext.hpp"
#include "gui/core/window_builder/helper.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "wml_exception.hpp"

#include <functional>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(tab_container)

tab_container::tab_container(const implementation::builder_tab_container& builder)
	: container_base(builder, type())
	, state_(ENABLED)
	, builders_(builder.builders)
	, list_items_(builder.list_items)
{
}

void tab_container::set_self_active(const bool active)
{
	state_ = active ? ENABLED : DISABLED;
}

bool tab_container::get_active() const
{
	return state_ != DISABLED;
}

unsigned tab_container::get_state() const
{
	return state_;
}

bool tab_container::can_wrap() const
{
	return true;
}

listbox& tab_container::get_internal_list()
{
	return get_grid().find_widget<listbox>("_tab_list", false);
}

void tab_container::finalize(std::unique_ptr<generator_base> generator)
{
	generator_ = generator.get();
	assert(generator_);

	widget_item empty_data;
	for(const auto& builder_entry : builders_) {
		generator->create_item(-1, *builder_entry, empty_data, nullptr);
	}

	grid* parent_grid = find_widget<grid>("_content_grid", false, true);
	if (parent_grid) {
		parent_grid->swap_child("_page", std::move(generator), false);
	}

	finalize_listbox();

	select_tab(0);
}

void tab_container::finalize_listbox() {
	for (const widget_data& row : list_items_) {
		add_tab_entry(row);
	}
	get_internal_list().connect_signal<event::NOTIFY_MODIFIED>(std::bind(&tab_container::change_selection, this));
};

void tab_container::add_tab_entry(const widget_data& row)
{
	listbox& list = get_internal_list();
	list.add_row(row);
}

void tab_container::select_tab(unsigned index)
{
	if (index < get_tab_count()) {
		get_internal_list().select_row(index);
		generator_->select_item(index, true);
	}
}

void tab_container::change_selection() {
	select_tab(get_active_tab_index());
	place(get_origin(), get_size());
	queue_redraw();

	fire(event::NOTIFY_MODIFIED, *this, nullptr);
}

// }---------- DEFINITION ---------{

tab_container_definition::tab_container_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing tab_container " << id;

	load_resolutions<resolution>(cfg);
}

tab_container_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg), grid(nullptr)
{
	// Note the order should be the same as the enum state_t is tab_container.hpp.
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", missing_mandatory_wml_tag("tab_container_definition][resolution", "state_enabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_disabled", missing_mandatory_wml_tag("tab_container_definition][resolution", "state_disabled")));

	auto child = VALIDATE_WML_CHILD(cfg, "grid", _("No grid defined for tab container control"));
	grid = std::make_shared<builder_grid>(child);
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_tab_container::builder_tab_container(const config& cfg)
	: implementation::builder_styled_widget(cfg)
{
	if (cfg.has_child("tab")) {
		for(const auto & tab : cfg.child_range("tab"))
		{
			widget_data list_row;
			widget_item item;

			item["label"] = tab["image"];
			list_row.emplace("image", item);
			item["label"] = tab["name"];
			list_row.emplace("name", item);

			list_items.emplace_back(list_row);

			if (tab.has_child("data")) {
				auto builder = std::make_shared<builder_grid>(tab.mandatory_child("data"));
				builders.push_back(builder);
			}
		}
	}
}

std::unique_ptr<widget> builder_tab_container::build() const
{
	auto widget = std::make_unique<tab_container>(*this);

	const auto conf = widget->cast_config_to<tab_container_definition>();
	assert(conf);

	widget->init_grid(*conf->grid);

	auto generator = generator_base::build(true, true, generator_base::independent, false);
	widget->finalize(std::move(generator));

	DBG_GUI_G << "Window builder: placed tab_container '" << id
			  << "' with definition '" << definition << "'.";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} //
