/*
	Copyright (C) 2024
	by babaissarkar(Subhraman Sarkar) <suvrax@gmail.com>
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
#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/window_builder/helper.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/stacked_widget.hpp"
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
	return find_widget<listbox>(&get_grid(), "_tab_list", false);
}

void tab_container::finalize_setup() {
	for (const widget_data& row : list_items_) {
		add_tab_entry(row);
	}
	get_internal_list().connect_signal<event::NOTIFY_MODIFIED>(std::bind(&tab_container::change_selection, this));
};

void tab_container::add_tab_entry(const widget_data row)
{
	listbox& list = get_internal_list();
	list.add_row(row);
}

void tab_container::select_tab(unsigned index)
{
	if (index < list_items_.size()) {
		get_internal_list().select_row(index);

		grid* parent_grid = find_widget<grid>(this, "_content_grid", false, true);

		if (parent_grid) {
			std::unique_ptr<widget> grid = std::move(builders_[list_items_.at(index)["name"]["label"]]->build());
			grid.get()->set_id("_page");
			parent_grid->swap_child("_page", std::move(grid), false);
		}
	}
}

void tab_container::change_selection() {
	select_tab(get_internal_list().get_selected_row());
	place(get_origin(), get_size());
	queue_redraw();
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
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", _("Missing required state for tab container control")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_disabled", _("Missing required state for tab container control")));

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
				builders[tab["name"]] = builder;
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

	widget->set_items(list_items);
	widget->set_builders(builders);

	widget->finalize_setup();

	widget->select_tab(list_items.size()-1);

	DBG_GUI_G << "Window builder: placed tab_container '" << id
			  << "' with definition '" << definition << "'.";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} //
